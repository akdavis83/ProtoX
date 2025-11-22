#include "net/pq_noise.h"
#include "net/pq_metrics.h"
#include <cstring>
#include <oqs/oqs.h>
#include "crypto/hkdf_sha3_512.h"
#include "random.h"
#include "util/strencodings.h"

// HKDF-SHA3-512 wrapper
static bool hkdf_sha3_512_extract_expand(std::span<const uint8_t> ikm,
                                         std::span<const uint8_t> salt,
                                         std::span<const uint8_t> info,
                                         std::vector<uint8_t>& okm,
                                         size_t out_len)
{
    std::vector<uint8_t> salt_v(salt.begin(), salt.end());
    std::vector<uint8_t> ikm_v(ikm.begin(), ikm.end());
    std::vector<uint8_t> info_v(info.begin(), info.end());
    okm = HKDF_SHA3_512(salt_v, ikm_v, info_v, out_len);
    return okm.size() == out_len;
}

PQNoiseSession::PQNoiseSession(PQNoiseRole role, const PQNoiseConfig& cfg)
    : m_role(role), m_server_kem_pk(cfg.server_kem_pk), m_server_sig_pk(cfg.server_sig_pk)
{
    if (cfg.server_kem_sk) {
        m_server_kem_sk = *cfg.server_kem_sk;
    }
    if (cfg.server_sig_sk) {
        m_server_sig_sk = *cfg.server_sig_sk;
    }
}

void PQNoiseSession::SetError(std::string msg)
{
    m_err = std::move(msg);
    m_state = PQNoiseState::Error;
    PQMetrics::RecordHandshakeFailure(msg);
    LogPQError(msg, "PQNoiseSession");
}

bool PQNoiseSession::StartHandshake(std::vector<uint8_t>& out_msg)
{
    PQMetrics::RecordHandshakeAttempt();
    LogPQHandshakeStage("Starting handshake", m_role == PQNoiseRole::Client ? "Client" : "Server");
    
    if (m_role == PQNoiseRole::Client) {
        return ClientStart(out_msg);
    }
    // Server waits for ClientHello
    m_state = PQNoiseState::Init;
    out_msg.clear();
    return true;
}

bool PQNoiseSession::OnHandshakeMessage(std::span<const uint8_t> in_msg, std::vector<uint8_t>& out_msg)
{
    if (m_role == PQNoiseRole::Server) {
        return ServerRespond(in_msg, out_msg);
    }
    out_msg.clear();
    return ClientFinish(in_msg);
}

bool PQNoiseSession::ClientStart(std::vector<uint8_t>& out_clienthello)
{
    // Client encapsulates to server static KEM public key and sends ciphertext in ClientHello
    if (m_server_kem_pk.empty()) {
        SetError(Missing server KEM public key");
        return false;
    }

    // Use liboqs to perform Kyber1024 encapsulation
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_kyber_1024);
    if (!kem) {
        SetError("Failed to initialize Kyber1024 KEM");
        return false;
    }

    // Prepare buffers
    std::vector<uint8_t> shared_secret(kem->length_shared_secret);
    std::vector<uint8_t> ciphertext(kem->length_ciphertext);

    // Perform encapsulation
    OQS_STATUS status = OQS_KEM_encaps(kem, ciphertext.data(), shared_secret.data(), m_server_kem_pk.data());
    OQS_KEM_free(kem);

    if (status != OQS_SUCCESS) {
        SetError("Kyber1024 encapsulation failed");
        return false;
    }

    // Store ciphertext and derive transport keys
    m_kem_ct = ciphertext;
    if (!DeriveKeys(shared_secret)) {
        SetError("Key derivation failed");
        return false;
    }

    // Build ClientHello message: magic(4) + version(2) + ciphertext_len(2) + ciphertext + random_padding
    out_clienthello.clear();
    out_clienthello.push_back(0xF9); // Magic bytes for PQ
    out_clienthello.push_back(0xBE);
    out_clienthello.push_back(0xB4);
    out_clienthello.push_back(0xD9);
    
    out_clienthello.push_back(0x00); // Version 1
    out_clienthello.push_back(0x01);
    
    uint16_t ct_len = static_cast<uint16_t>(ciphertext.size());
    out_clienthello.push_back((ct_len >> 8) & 0xFF);
    out_clienthello.push_back(ct_len & 0xFF);
    
    out_clienthello.insert(out_clienthello.end(), ciphertext.begin(), ciphertext.end());
    
    // Add 32 bytes random padding
    std::vector<uint8_t> padding(32);
    GetRandBytes(padding);
    out_clienthello.insert(out_clienthello.end(), padding.begin(), padding.end());

    m_handshake_transcript.insert(m_handshake_transcript.end(), out_clienthello.begin(), out_clienthello.end());

    m_state = PQNoiseState::SentClientHello;
    LogPQHandshakeStage("ClientHello sent", strprintf("ciphertext %d bytes", ciphertext.size()));
    return true;
}

bool PQNoiseSession::ServerRespond(std::span<const uint8_t> clienthello, std::vector<uint8_t>& out_serverhello)
{
    // Parse ClientHello and extract ciphertext
    if (clienthello.size() < 8) {
        SetError("ClientHello too short");
        return false;
    }

    // Verify magic bytes
    if (clienthello[0] != 0xF9 || clienthello[1] != 0xBE || 
        clienthello[2] != 0xB4 || clienthello[3] != 0xD9) {
        SetError("Invalid ClientHello magic");
        return false;
    }

    // Extract ciphertext length and ciphertext
    uint16_t ct_len = (static_cast<uint16_t>(clienthello[6]) << 8) | clienthello[7];
    if (clienthello.size() < 8 + ct_len) {
        SetError("ClientHello ciphertext truncated");
        return false;
    }

    std::vector<uint8_t> ciphertext(clienthello.begin() + 8, clienthello.begin() + 8 + ct_len);

    // Perform KEM decapsulation
    if (!m_server_kem_sk) {
        SetError("Missing server KEM secret key");
        return false;
    }

    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_kyber_1024);
    if (!kem) {
        SetError("Failed to initialize Kyber1024 KEM");
        return false;
    }

    std::vector<uint8_t> shared_secret(kem->length_shared_secret);
    OQS_STATUS status = OQS_KEM_decaps(kem, shared_secret.data(), ciphertext.data(), m_server_kem_sk->data());
    OQS_KEM_free(kem);

    if (status != OQS_SUCCESS) {
        SetError("Kyber1024 decapsulation failed");
        return false;
    }

    // Derive transport keys
    if (!DeriveKeys(shared_secret)) {
        SetError("Key derivation failed");
        return false;
    }

    // --- transcript and signing ---
    m_handshake_transcript.insert(m_handshake_transcript.end(), clienthello.begin(), clienthello.end());

    if (!m_server_sig_sk) {
        SetError("Missing server signature secret key for authentication");
        return false;
    }

    OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_dilithium_3);
    if (!sig) {
        SetError("Failed to initialize Dilithium3 SIG");
        return false;
    }

    std::vector<uint8_t> signature(sig->length_signature);
    size_t signature_len;

    OQS_STATUS sig_status = OQS_SIG_sign(sig, signature.data(), &signature_len, m_handshake_transcript.data(), m_handshake_transcript.size(), m_server_sig_sk->data());
    OQS_SIG_free(sig);

    if (sig_status != OQS_SUCCESS) {
        SetError("Failed to sign handshake transcript");
        return false;
    }
    signature.resize(signature_len);
    // --- end signing ---

    // Build ServerHello: magic + version + status + signature
    out_serverhello.clear();
    out_serverhello.push_back(0xF9); // Magic bytes
    out_serverhello.push_back(0xBE);
    out_serverhello.push_back(0xB4);
    out_serverhello.push_back(0xD9);
    
    out_serverhello.push_back(0x00); // Version 1
    out_serverhello.push_back(0x01);
    
    out_serverhello.push_back(0x00); // Status: success
    
    // Append signature
    out_serverhello.insert(out_serverhello.end(), signature.begin(), signature.end());

    m_state = PQNoiseState::Established;
    PQMetrics::RecordHandshakeSuccess();
    PQMetrics::RecordSessionStart();
    LogPQHandshakeStage("Server handshake complete", "Session established");
    LogPQSuite("NoisePQ_KYBER1024_DILITHIUM3_SHA3-512_CHACHA20-POLY1305");
    return true;
}

bool PQNoiseSession::ClientFinish(std::span<const uint8_t> serverhello)
{
    // Parse ServerHello and verify signature
    if (!m_server_sig_pk) {
        SetError("Missing server signature public key for authentication");
        return false;
    }

    OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_dilithium_3);
    if (!sig) {
        SetError("Failed to initialize Dilithium3 SIG");
        return false;
    }

    if (serverhello.size() < 7 + sig->length_signature) {
        SetError("ServerHello too short to contain a signature");
        OQS_SIG_free(sig);
        return false;
    }

    // Verify magic and status
    if (serverhello[0] != 0xF9 || serverhello[1] != 0xBE || 
        serverhello[2] != 0xB4 || serverhello[3] != 0xD9) {
        SetError("Invalid ServerHello magic");
        OQS_SIG_free(sig);
        return false;
    }

    if (serverhello[6] != 0x00) {
        SetError("Server rejected handshake");
        OQS_SIG_free(sig);
        return false;
    }

    // Extract signature from the end of the message
    std::vector<uint8_t> signature(serverhello.end() - sig->length_signature, serverhello.end());

    // Verify signature against the handshake transcript (which is the ClientHello)
    OQS_STATUS sig_status = OQS_SIG_verify(sig, m_handshake_transcript.data(), m_handshake_transcript.size(), signature.data(), signature.size(), m_server_sig_pk->data());
    OQS_SIG_free(sig);

    if (sig_status != OQS_SUCCESS) {
        SetError("Invalid server signature, possible MitM attack");
        return false;
    }

    m_state = PQNoiseState::Established;
    PQMetrics::RecordHandshakeSuccess();
    PQMetrics::RecordSessionStart();
    LogPQHandshakeStage("Client handshake complete", "Session established");
    return true;
}

bool PQNoiseSession::DeriveKeys(std::span<const uint8_t> shared_secret)
{
    // HKDF-SHA3-512 key derivation
    std::vector<uint8_t> salt = {0x50, 0x51, 0x4E, 0x6F, 0x69, 0x73, 0x65}; // "PQNoise"
    std::vector<uint8_t> info = {0x4B, 0x65, 0x79, 0x73}; // "Keys"
    
    std::vector<uint8_t> derived_keys;
    if (!hkdf_sha3_512_extract_expand(shared_secret, salt, info, derived_keys, 96)) {
        return false;
    }

    // Split derived keys: 32 bytes key_in, 32 bytes key_out, 32 bytes nonce_base
    m_kdf_key_in.assign(derived_keys.begin(), derived_keys.begin() + 32);
    m_kdf_key_out.assign(derived_keys.begin() + 32, derived_keys.begin() + 64);
    
    // Initialize AEAD contexts
    m_aead_in.emplace(m_kdf_key_in, derived_keys.data() + 64);
    m_aead_out.emplace(m_kdf_key_out, derived_keys.data() + 64);
    
    m_send_nonce = 0;
    m_recv_nonce = 0;
    
    return true;
}

bool PQNoiseSession::Seal(std::span<const uint8_t> plain, std::vector<uint8_t>& out_cipher)
{
    if (!m_aead_out || m_state != PQNoiseState::Established) {
        return false;
    }
    
    // Format: nonce(8) + ciphertext + tag
    out_cipher.resize(8 + plain.size() + 16);
    
    // Write nonce
    for (int i = 0; i < 8; i++) {
        out_cipher[i] = (m_send_nonce >> (8 * i)) & 0xFF;
    }
    
    // Encrypt
    std::vector<uint8_t> nonce_bytes(12, 0);
    std::copy_n(out_cipher.data(), 8, nonce_bytes.data());
    
    bool success = m_aead_out->Encrypt(nonce_bytes, {}, plain, 
                                       {out_cipher.data() + 8, plain.size()}, 
                                       {out_cipher.data() + 8 + plain.size(), 16});
    
    if (success) {
        m_send_nonce++;
        PQMetrics::RecordBytesEncrypted(plain.size());
    }
    
    return success;
}

bool PQNoiseSession::Open(std::span<const uint8_t> cipher, std::vector<uint8_t>& out_plain)
{
    if (!m_aead_in || m_state != PQNoiseState::Established || cipher.size() < 24) {
        return false;
    }
    
    // Extract nonce
    uint64_t received_nonce = 0;
    for (int i = 0; i < 8; i++) {
        received_nonce |= (static_cast<uint64_t>(cipher[i]) << (8 * i));
    }
    
    // Check nonce ordering
    if (received_nonce != m_recv_nonce) {
        return false;
    }
    
    // Decrypt
    std::vector<uint8_t> nonce_bytes(12, 0);
    std::copy_n(cipher.data(), 8, nonce_bytes.data());
    
    out_plain.resize(cipher.size() - 24);
    bool success = m_aead_in->Decrypt(nonce_bytes, {}, 
                                      {cipher.data() + 8, cipher.size() - 24},
                                      {cipher.data() + cipher.size() - 16, 16},
                                      out_plain);
    
    if (success) {
        m_recv_nonce++;
        PQMetrics::RecordBytesDecrypted(out_plain.size());
    }
    
    return success;
