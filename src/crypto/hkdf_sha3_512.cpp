#include "crypto/hkdf_sha3_512.h"
#include "crypto/sha3.h"
#include "crypto/hmac_sha3_512.h"

#include <cmath>

// HMAC-SHA3-512
static std::vector<uint8_t> hmac_sha3_512(const std::vector<uint8_t>& key, const std::vector<uint8_t>& data) {
    unsigned char hash[64];
    CHMAC_SHA3_512(key.data(), key.size()).Write(data.data(), data.size()).Finalize(hash);
    return std::vector<uint8_t>(hash, hash + 64);
}


std::vector<uint8_t> HKDF_SHA3_512(const std::vector<uint8_t>& salt,
                                 const std::vector<uint8_t>& ikm,
                                 const std::vector<uint8_t>& info,
                                 size_t out_len) {
    // 1. Extract
    std::vector<uint8_t> prk = hmac_sha3_512(salt, ikm);

    // 2. Expand
    std::vector<uint8_t> okm;
    okm.reserve(out_len);

    std::vector<uint8_t> t;
    for (size_t i = 0; i < std::ceil((double)out_len / 64); ++i) {
        std::vector<uint8_t> input = t;
        input.insert(input.end(), info.begin(), info.end());
        input.push_back(i + 1);
        t = hmac_sha3_512(prk, input);
        okm.insert(okm.end(), t.begin(), t.end());
    }

    okm.resize(out_len);
    return okm;
}
