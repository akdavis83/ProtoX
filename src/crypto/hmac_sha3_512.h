#ifndef QTC_CRYPTO_HMAC_SHA3_512_H
#define QTC_CRYPTO_HMAC_SHA3_512_H

#include <crypto/sha3.h>

#include <cstdint>
#include <cstdlib>

/** A hasher class for HMAC-SHA3-512. */
class CHMAC_SHA3_512
{
private:
    CSHA3_512 outer;
    CSHA3_512 inner;

public:
    static const size_t OUTPUT_SIZE = 64;

    CHMAC_SHA3_512(const unsigned char* key, size_t keylen);
    CHMAC_SHA3_512& Write(const unsigned char* data, size_t len)
    {
        inner.Write(data, len);
        return *this;
    }
    void Finalize(unsigned char hash[OUTPUT_SIZE]);
};

#endif // QTC_CRYPTO_HMAC_SHA3_512_H
