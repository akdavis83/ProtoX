#include "net/pqnoise/pqnoise_state.h"
#include "crypto/sha3.h"
#include "crypto/hkdf_sha3_512.h"

#include <random>

namespace qtc_net {

// Helper function to generate random bytes
static std::vector<uint8_t> GenRandomBytes(size_t len) {
    std::vector<uint8_t> buf(len);
    std::random_device rd;
    std::generate(buf.begin(), buf.end(), [&]() { return rd(); });
    return buf;
}

PQNoiseHandshake::PQNoiseHandshake(bool is_server)
    : m_state(State::INIT), m_is_server(is_server) {}

PQNoiseHandshake::State PQNoiseHandshake::GetState() const {
    return m_state;
}

const std::vector<uint8_t>& PQNoiseHandshake::GetKeyIn() const {
    return m_key_in;
}

const std::vector<uint8_t>& PQNoiseHandshake::GetKeyOut() const {
    return m_key_out;
}

void PQNoiseHandshake::AppendToTranscript(const std::vector<uint8_t>& data) {
    m_transcript.insert(m_transcript.end(), data.begin(), data.end());
}

void PQNoiseHandshake::DeriveKeys(const qtc_kyber::SharedSecret& ss) {
    std::vector<uint8_t> ss_vec(ss.begin(), ss.end());
    auto derived_keys = HKDF_SHA3_512({}, ss_vec, m_transcript, 64);
    m_key_in.assign(derived_keys.begin(), derived_keys.begin() + 32);
    m_key_out.assign(derived_keys.begin() + 32, derived_keys.end());
}

std::vector<uint8_t> PQNoiseHandshake::BuildClientHello() {
    if (m_is_server || m_state != State::INIT) {
        m_state = State::ERROR;
        return {};
    }

    // client_random (32 bytes)
    auto client_random = GenRandomBytes(32);
    AppendToTranscript(client_random);

    m_state = State::SENT_CLIENT_HELLO;
    return client_random;
}

std::vector<uint8_t> PQNoiseHandshake::BuildServerHello(const std::vector<uint8_t>& client_hello, const qtc_dilithium::SecretKey& server_identity_sk) {
    if (!m_is_server || m_state != State::INIT) {
        m_state = State::ERROR;
        return {};
    }

    AppendToTranscript(client_hello);

    // server_random (32 bytes)
    auto server_random = GenRandomBytes(32);
    AppendToTranscript(server_random);

    // Generate ephemeral Kyber key pair
    auto ephemeral_keys = qtc_kyber::KeyGen1024();
    m_ephemeral_pk = ephemeral_keys.first;
    m_ephemeral_sk = ephemeral_keys.second;
    
    std::vector<uint8_t> pk_vec(m_ephemeral_pk.begin(), m_ephemeral_pk.end());
    AppendToTranscript(pk_vec);

    // Sign the transcript
    qtc_dilithium::Signature sig = qtc_dilithium::Sign(m_transcript, server_identity_sk);

    // Construct the server hello message
    std::vector<uint8_t> server_hello_msg;
    server_hello_msg.insert(server_hello_msg.end(), server_random.begin(), server_random.end());
    server_hello_msg.insert(server_hello_msg.end(), pk_vec.begin(), pk_vec.end());
    server_hello_msg.insert(server_hello_msg.end(), sig.begin(), sig.end());

    m_state = State::RECEIVED_SERVER_HELLO; // For server
    return server_hello_msg;
}

bool PQNoiseHandshake::ProcessServerHello(const std::vector<uint8_t>& server_hello, const qtc_dilithium::PublicKey& server_identity_pk) {
    if (m_is_server || m_state != State::SENT_CLIENT_HELLO) {
        m_state = State::ERROR;
        return false;
    }

    // Deconstruct server hello
    if (server_hello.size() < (32 + qtc_kyber::KYBER1024_PUBLICKEY_BYTES + qtc_dilithium::DILITHIUM3_SIGNATURE_BYTES)) {
        m_state = State::ERROR;
        return false;
    }

    std::vector<uint8_t> server_random(server_hello.begin(), server_hello.begin() + 32);
    std::vector<uint8_t> server_ephemeral_pk_vec(server_hello.begin() + 32, server_hello.begin() + 32 + qtc_kyber::KYBER1024_PUBLICKEY_BYTES);
    qtc_dilithium::Signature sig(server_hello.begin() + 32 + qtc_kyber::KYBER1024_PUBLICKEY_BYTES, server_hello.end());

    AppendToTranscript(server_random);
    AppendToTranscript(server_ephemeral_pk_vec);

    // Verify the signature
    if (!qtc_dilithium::Verify(sig, m_transcript, server_identity_pk)) {
        m_state = State::ERROR;
        return false;
    }

    std::copy(server_ephemeral_pk_vec.begin(), server_ephemeral_pk_vec.end(), m_ephemeral_pk.begin());

    m_state = State::RECEIVED_SERVER_HELLO;
    return true;
}

std::vector<uint8_t> PQNoiseHandshake::BuildClientKEM(const std::vector<uint8_t>& server_hello) {
    if (m_is_server || m_state != State::RECEIVED_SERVER_HELLO) {
        m_state = State::ERROR;
        return {};
    }

    // Encapsulate the shared secret
    auto kem_result = qtc_kyber::Encrypt1024(m_ephemeral_pk);
    auto ciphertext = kem_result.first;
    auto shared_secret = kem_result.second;

    std::vector<uint8_t> ct_vec(ciphertext.begin(), ciphertext.end());
    AppendToTranscript(ct_vec);

    // Derive keys
    DeriveKeys(shared_secret);

    m_state = State::SENT_KEM_CT;
    return ct_vec;
}

bool PQNoiseHandshake::ProcessClientKEM(const std::vector<uint8_t>& client_kem, const qtc_kyber::SecretKey& kem_sk) {
    if (!m_is_server || m_state != State::RECEIVED_SERVER_HELLO) {
        m_state = State::ERROR;
        return false;
    }

    if (client_kem.size() != qtc_kyber::KYBER1024_CIPHERTEXT_BYTES) {
        m_state = State::ERROR;
        return false;
    }

    qtc_kyber::Ciphertext ciphertext;
    std::copy(client_kem.begin(), client_kem.end(), ciphertext.begin());

    AppendToTranscript(client_kem);

    // Decapsulate the shared secret
    auto shared_secret = qtc_kyber::Decrypt1024(ciphertext, m_ephemeral_sk);

    // Derive keys
    DeriveKeys(shared_secret);

    m_state = State::ESTABLISHED;
    return true;
}

} // namespace qtc_net
