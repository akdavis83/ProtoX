#pragma once
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <crypto/chacha20poly1305.h>

enum class PQNoiseRole { Client, Server };
enum class PQNoiseState { Init, SentClientHello, RecvServerHello, Established, Error };

struct PQNoiseConfig {
    std::vector<uint8_t> server_kem_pk; // Published static KEM public key
    std::optional<std::vector<uint8_t>> server_kem_sk; // Server-side secret key for decapsulation (required on Server)
    std::optional<std::vector<uint8_t>> server_sig_pk; // Optional Dilithium public key for server auth (stubbed for Phase 3)
    std::optional<std::vector<uint8_t>> server_sig_sk; // Server-side secret key for signing (required on Server)
};

class PQNoiseSession {
public:
    PQNoiseSession(PQNoiseRole role, const PQNoiseConfig& cfg);

    bool StartHandshake(std::vector<uint8_t>& out_msg);
    bool OnHandshakeMessage(std::span<const uint8_t> in_msg, std::vector<uint8_t>& out_msg);

    bool Seal(std::span<const uint8_t> plain, std::vector<uint8_t>& out_cipher);
    bool Open(std::span<const uint8_t> cipher, std::vector<uint8_t>& out_plain);

    bool Established() const { return m_state == PQNoiseState::Established; }
    std::string LastError() const { return m_err; }

private:
    bool ClientStart(std::vector<uint8_t>& out_clienthello);
    bool ServerRespond(std::span<const uint8_t> clienthello, std::vector<uint8_t>& out_serverhello);
    bool ClientFinish(std::span<const uint8_t> serverhello);

    bool DeriveKeys(std::span<const uint8_t> shared_secret);
    void SetError(std::string msg);

private:
    PQNoiseRole m_role;
    PQNoiseState m_state{PQNoiseState::Init};
    std::string m_err;

    // KEM / key material
    std::vector<uint8_t> m_server_kem_pk;
    std::optional<std::vector<uint8_t>> m_server_kem_sk;
    std::optional<std::vector<uint8_t>> m_server_sig_pk;
    std::optional<std::vector<uint8_t>> m_server_sig_sk;

    // Ephemeral values and secrets
    std::vector<uint8_t> m_handshake_transcript; // Transcript of handshake for signing
    std::vector<uint8_t> m_client_eph;     // client ephemeral (public bytes)
    std::vector<uint8_t> m_server_eph;     // server ephemeral (public bytes)
    std::vector<uint8_t> m_kem_ct;         // encapsulated ciphertext (server->client)
    std::vector<uint8_t> m_kdf_key_in;     // derived inbound key
    std::vector<uint8_t> m_kdf_key_out;    // derived outbound key
    uint64_t m_send_nonce{0};
    uint64_t m_recv_nonce{0};

    // AEAD contexts (initialized after DeriveKeys)
    std::optional<AEADChaCha20Poly1305> m_aead_in;
    std::optional<AEADChaCha20Poly1305> m_aead_out;
};
