#ifndef QTC_CRYPTO_QTC_HASH_H
#define QTC_CRYPTO_QTC_HASH_H

#include <array>
#include <cstdint>
#include <algorithm>
#include <crypto/sha3.h>

// Recommended: use SHA3-256(pubkey) and take first 20 bytes for the witness program.
inline std::array<uint8_t, 20> QTC_Program20_From_PK_SHA3_256(const uint8_t* pk, size_t pk_len) {
    std::array<uint8_t, 32> h256{};
    CSHA3_256().Write(pk, pk_len).Finalize(h256.data());
    std::array<uint8_t, 20> out{};
    std::copy(h256.begin(), h256.begin() + 20, out.begin());
    return out;
}

// Alternative: domain-separated SHA3-512 then truncate (if you must stick to SHA3-512 everywhere).
inline std::array<uint8_t, 20> QTC_Program20_From_PK_SHA3_512_DS(const uint8_t* pk, size_t pk_len) {
    static const char tag[] = "QTC-PKH";
    std::array<uint8_t, 64> h512{};
    CSHA3_512().Write(reinterpret_cast<const uint8_t*>(tag), sizeof(tag) - 1)
               .Write(pk, pk_len).Finalize(h512.data());
    std::array<uint8_t, 20> out{};
    std::copy(h512.begin(), h512.begin() + 20, out.begin());
    return out;
}

#endif // QTC_CRYPTO_QTC_HASH_H
