#include crypto/chacha20poly1305.h
#include <cstring>

static inline uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }
static inline uint32_t rotl(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }
static inline uint32_t load32_le(const uint8_t* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
static inline void store32_le(uint8_t* p, uint32_t v) { p[0] = v & 0xFF; p[1] = (v >> 8) & 0xFF; p[2] = (v >> 16) & 0xFF; p[3] = (v >> 24) & 0xFF; }

static inline void quarterround(uint32_t s[16], int a, int b, int c, int d)
{
    s[a] += s[b]; s[d] ^= s[a]; s[d] = rotl(s[d], 16);
    s[c] += s[d]; s[b] ^= s[c]; s[b] = rotl(s[b], 12);
    s[a] += s[b]; s[d] ^= s[a]; s[d] = rotl(s[d], 8);
    s[c] += s[d]; s[b] ^= s[c]; s[b] = rotl(s[b], 7);
}

AEADChaCha20Poly1305::AEADChaCha20Poly1305(std::span<const uint8_t, KEY_LEN> key)
{
    for (int i = 0; i < 8; ++i) {
        m_key[i] = load32_le(key.data() + 4 * i);
    }
}

void AEADChaCha20Poly1305::chacha20_block(uint32_t out[16], const uint32_t key[8], uint32_t counter, const uint32_t nonce[3]) const
{
    uint32_t s[16];
    s[0] = 0x61707865; // expa
    s[1] = 0x3320646e; // nd
