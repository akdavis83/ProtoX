#include <crypto/hmac_sha3_512.h>
#include <cstring>

CHMAC_SHA3_512::CHMAC_SHA3_512(const unsigned char* key, size_t keylen)
{
    unsigned char rkey[128];
    if (keylen <= 128) {
        memcpy(rkey, key, keylen);
        memset(rkey + keylen, 0, 128 - keylen);
    } else {
        CSHA3_512().Write(key, keylen).Finalize(rkey);
        memset(rkey + 64, 0, 128 - 64);
    }

    for (int i = 0; i < 128; i++)
        rkey[i] ^= 0x5c;
    outer.Write(rkey, 128);

    for (int i = 0; i < 128; i++)
        rkey[i] ^= 0x5c ^ 0x36;
    inner.Write(rkey, 128);
}

void CHMAC_SHA3_512::Finalize(unsigned char hash[OUTPUT_SIZE])
{
    unsigned char temp[64];
    inner.Finalize(temp);
    outer.Write(temp, 64).Finalize(hash);
}
