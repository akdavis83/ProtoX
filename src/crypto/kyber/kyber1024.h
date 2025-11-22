#ifndef QTC_CRYPTO_KYBER_KYBER1024_H
#define QTC_CRYPTO_KYBER_KYBER1024_H

#include <array>
#include <vector>
#include <cstdint>
#include <utility>

namespace qtc_kyber {
    // NIST FIPS-203 Kyber1024 Parameters (Production Grade)
    static constexpr size_t KYBER_K = 4;           // Kyber1024 security parameter
    static constexpr size_t KYBER_N = 256;         // Polynomial degree
    static constexpr size_t KYBER_Q = 3329;        // Modulus prime
    static constexpr size_t KYBER_SYMBYTES = 32;   // Hash/seed size
    static constexpr size_t KYBER_SSBYTES = 32;    // Shared secret size
    static constexpr size_t KYBER_ETA1 = 2;        // Noise parameter eta1
    static constexpr size_t KYBER_ETA2 = 2;        // Noise parameter eta2
    
    // NIST FIPS-203 Kyber1024 Key Sizes (matching NIST C reference)
    static constexpr size_t KYBER_POLYBYTES = 384;
    static constexpr size_t KYBER_POLYVECBYTES = (KYBER_K * KYBER_POLYBYTES);
    static constexpr size_t KYBER_POLYCOMPRESSEDBYTES = 160;
    static constexpr size_t KYBER_POLYVECCOMPRESSEDBYTES = (KYBER_K * 352);
    
    static constexpr size_t KYBER_INDCPA_PUBLICKEYBYTES = (KYBER_POLYVECBYTES + KYBER_SYMBYTES);
    static constexpr size_t KYBER_INDCPA_SECRETKEYBYTES = (KYBER_POLYVECBYTES);
    static constexpr size_t KYBER_INDCPA_BYTES = (KYBER_POLYVECCOMPRESSEDBYTES + KYBER_POLYCOMPRESSEDBYTES);
    
    // Final Kyber1024 sizes (FIPS-203 compliant)
    static constexpr size_t KYBER1024_PUBLICKEY_BYTES = KYBER_INDCPA_PUBLICKEYBYTES;   // 1568 bytes
    static constexpr size_t KYBER1024_SECRETKEY_BYTES = (KYBER_INDCPA_SECRETKEYBYTES + 
                                                         KYBER_INDCPA_PUBLICKEYBYTES + 
                                                         2*KYBER_SYMBYTES);           // 3168 bytes
    static constexpr size_t KYBER1024_CIPHERTEXT_BYTES = KYBER_INDCPA_BYTES;          // 1568 bytes
    static constexpr size_t KYBER1024_SHAREDSECRET_BYTES = KYBER_SSBYTES;             // 32 bytes

    using PublicKey = std::array<uint8_t, KYBER1024_PUBLICKEY_BYTES>;
    using SecretKey = std::array<uint8_t, KYBER1024_SECRETKEY_BYTES>;
    using Ciphertext = std::array<uint8_t, KYBER1024_CIPHERTEXT_BYTES>;
    using SharedSecret = std::array<uint8_t, KYBER1024_SHAREDSECRET_BYTES>;
    using Polynomial = std::array<int16_t, KYBER_N>;
    using PolyVector = std::array<Polynomial, KYBER_K>;

    // NTT constants (from kyber1024.js)
    extern const std::array<uint16_t, 128> NTT_ZETAS;
    extern const std::array<uint16_t, 128> NTT_ZETAS_INV;

    // Core Kyber1024 API (NIST FIPS-203 compliant)
    std::pair<PublicKey, SecretKey> KeyGen1024(const std::array<uint8_t, 64>& seed);
    std::pair<PublicKey, SecretKey> KeyGen1024(); // Random version
    std::pair<Ciphertext, SharedSecret> Encrypt1024(const PublicKey& pk);
    SharedSecret Decrypt1024(const Ciphertext& cipher, const SecretKey& sk);

    // Internal implementation functions
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> indcpaKeyGen();
    std::vector<uint8_t> indcpaEncrypt(const std::vector<uint8_t>& pk, 
                                      const std::array<uint8_t, 32>& msg, 
                                      const std::array<uint8_t, 32>& coins);
    std::array<uint8_t, 32> indcpaDecrypt(const std::vector<uint8_t>& c, 
                                          const std::vector<uint8_t>& sk);
    
    // NTT operations
    void ntt(Polynomial& poly);
    void nttInverse(Polynomial& poly);
    int16_t nttFqMul(int16_t a, int16_t b);
    int16_t montgomeryReduce(int32_t a);
    int16_t barrett(int16_t a);
    
    // Polynomial operations
    void polyAdd(Polynomial& result, const Polynomial& a, const Polynomial& b);
    void polySubtract(Polynomial& result, const Polynomial& a, const Polynomial& b);
    void polyMul(Polynomial& result, const PolyVector& a, const PolyVector& b);
    void polyToMont(Polynomial& poly);
    void polyReduce(Polynomial& poly);
    
    // Encoding/decoding
    void polyToBytes(std::array<uint8_t, 384>& bytes, const Polynomial& poly);
    void polyFromBytes(Polynomial& poly, const std::array<uint8_t, 384>& bytes);
    void polyToMsg(std::array<uint8_t, 32>& msg, const Polynomial& poly);
    void polyFromMsg(Polynomial& poly, const std::array<uint8_t, 32>& msg);
    
    // Compression
    std::array<uint8_t, 1408> compress1(const PolyVector& u);
    std::array<uint8_t, 160> compress2(const Polynomial& v);
    PolyVector decompress1(const std::array<uint8_t, 1408>& data);
    Polynomial decompress2(const std::array<uint8_t, 160>& data);
    
    // Sampling and matrix generation
    Polynomial sample(const std::array<uint8_t, 32>& seed, uint8_t nonce);
    std::array<PolyVector, KYBER_K> generateMatrixA(const std::array<uint8_t, 32>& seed, bool transposed = false);
}

#endif // QTC_CRYPTO_KYBER_KYBER1024_H
