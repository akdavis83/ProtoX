#ifndef QTC_CRYPTO_HKDF_SHA3_512_H
#define QTC_CRYPTO_HKDF_SHA3_512_H

#include <vector>
#include <cstdint>

// HKDF-Extract-and-Expand using HMAC-SHA3-512
std::vector<uint8_t> HKDF_SHA3_512(const std::vector<uint8_t>& salt,
                                 const std::vector<uint8_t>& ikm,
                                 const std::vector<uint8_t>& info,
                                 size_t out_len);

#endif // QTC_CRYPTO_HKDF_SHA3_512_H
