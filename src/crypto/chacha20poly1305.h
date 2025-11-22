// Minimal ChaCha20-Poly1305 AEAD implementation (RFC 8439)
// Constant-time style operations; requires strict nonce discipline.
//
// API:
//  - AEADChaCha20Poly1305(key32)
//  - bool Seal(nonce12, ad, plaintext, out_ciphertext_with_tag)
//  - bool Open(nonce12, ad, ciphertext_with_tag, out_plaintext)

#pragma once
#include <cstdint>
#include <vector>
#include <span>

class AEADChaCha20Poly1305 {
public:
    static constexpr size_t KEY_LEN = 32;
    static constexpr size_t NONCE_LEN = 12; // RFC 8439 IETF
    static constexpr size_t TAG_LEN = 16;

    explicit AEADChaCha20Poly1305(std::span<const uint8_t> key);

    bool Seal(std::span<const uint8_t, NONCE_LEN> nonce,
              std::span<const uint8_t> ad,
              std::span<const uint8_t> plaintext,
              std::vector<uint8_t>& out_ciphertext_tag);

    bool Open(std::span<const uint8_t, NONCE_LEN> nonce,
              std::span<const uint8_t> ad,
              std::span<const uint8_t> ciphertext_tag,
              std::vector<uint8_t>& out_plaintext);

private:
    void chacha20_block(uint32_t out[16], const uint32_t key[8], uint32_t counter, const uint32_t nonce[3]) const;
    void chacha20_xor(std::span<uint8_t> data,
                      const uint32_t key[8], uint32_t counter, const uint32_t nonce[3]) const;

    static void clamp_r(uint8_t r[16]);
    static void poly1305_mac(const uint8_t* msg, size_t msg_len,
                             const uint8_t* ad, size_t ad_len,
                             const uint8_t* one_time_key,
                             uint8_t out_tag[16]);

    uint32_t m_key[8]{}; // 256-bit key as 8x32-bit little-endian words
};
EOF}
