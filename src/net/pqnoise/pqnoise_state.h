#ifndef QTC_NET_PQNOISE_PQNOISE_STATE_H
#define QTC_NET_PQNOISE_PQNOISE_STATE_H

#include <vector>
#include <string>
#include <array>
#include "crypto/kyber/kyber1024.h"
#include "crypto/dilithium/dilithium3.h"

namespace qtc_net {

class PQNoiseHandshake {
public:
    enum class State {
        INIT,
        SENT_CLIENT_HELLO,
        RECEIVED_SERVER_HELLO,
        SENT_KEM_CT,
        ESTABLISHED,
        ERROR
    };

    PQNoiseHandshake(bool is_server);

    // Getters
    State GetState() const;
    const std::vector<uint8_t>& GetKeyIn() const;
    const std::vector<uint8_t>& GetKeyOut() const;

    // Handshake message generation
    std::vector<uint8_t> BuildClientHello();
    std::vector<uint8_t> BuildServerHello(const std::vector<uint8_t>& client_hello, const qtc_dilithium::SecretKey& server_identity_sk);

    // Handshake message processing
    bool ProcessServerHello(const std::vector<uint8_t>& server_hello, const qtc_dilithium::PublicKey& server_identity_pk);
    std::vector<uint8_t> BuildClientKEM(const std::vector<uint8_t>& server_hello);
    bool ProcessClientKEM(const std::vector<uint8_t>& client_kem, const qtc_kyber::SecretKey& kem_sk);


private:
    void AppendToTranscript(const std::vector<uint8_t>& data);
    void DeriveKeys(const qtc_kyber::SharedSecret& ss);

    State m_state;
    bool m_is_server;
    std::vector<uint8_t> m_transcript;
    std::vector<uint8_t> m_key_in;
    std::vector<uint8_t> m_key_out;

    // Ephemeral keys for the handshake
    qtc_kyber::PublicKey m_ephemeral_pk;
    qtc_kyber::SecretKey m_ephemeral_sk;
};

} // namespace qtc_net

#endif // QTC_NET_PQNOISE_PQNOISE_STATE_H
