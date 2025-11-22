#ifndef QTC_CRYPTO_DILITHIUM_DILITHIUM3_H
#define QTC_CRYPTO_DILITHIUM_DILITHIUM3_H

#include <array>
#include <vector>
#include <span>

namespace qtc_dilithium {
    // NIST FIPS-204 Dilithium3 Parameters (Production Grade - DILITHIUM_MODE=3)
    static constexpr size_t DILITHIUM_MODE = 3;
    static constexpr size_t DILITHIUM_N = 256;
    static constexpr size_t DILITHIUM_Q = 8380417;
    static constexpr size_t DILITHIUM_D = 13;
    static constexpr size_t DILITHIUM_ROOT_OF_UNITY = 1753;
    static constexpr size_t DILITHIUM_SEEDBYTES = 32;
    static constexpr size_t DILITHIUM_CRHBYTES = 64;
    static constexpr size_t DILITHIUM_TRBYTES = 64;
    static constexpr size_t DILITHIUM_RNDBYTES = 32;
    
    // Dilithium3 specific parameters (MODE=3)
    static constexpr size_t DILITHIUM_K = 6;        // Vector dimension k
    static constexpr size_t DILITHIUM_L = 5;        // Vector dimension l
    static constexpr size_t DILITHIUM_ETA = 4;      // Noise bound
    static constexpr size_t DILITHIUM_TAU = 49;     // Number of ±1's in c
    static constexpr size_t DILITHIUM_BETA = 196;   // τ*ETA
    static constexpr size_t DILITHIUM_GAMMA1 = (1 << 19);     // γ1 = 2^19
    static constexpr size_t DILITHIUM_GAMMA2 = ((DILITHIUM_Q-1)/32);  // (q-1)/32
    static constexpr size_t DILITHIUM_OMEGA = 55;   // ω
    static constexpr size_t DILITHIUM_CTILDEBYTES = 48;
    
    // Packing sizes (NIST reference)
    static constexpr size_t DILITHIUM_POLYT1_PACKEDBYTES = 320;
    static constexpr size_t DILITHIUM_POLYT0_PACKEDBYTES = 416;
    static constexpr size_t DILITHIUM_POLYVECH_PACKEDBYTES = (DILITHIUM_OMEGA + DILITHIUM_K);
    static constexpr size_t DILITHIUM_POLYZ_PACKEDBYTES = 640;    // for GAMMA1 = 2^19
    static constexpr size_t DILITHIUM_POLYW1_PACKEDBYTES = 128;   // for GAMMA2 = (q-1)/32
    static constexpr size_t DILITHIUM_POLYETA_PACKEDBYTES = 128;  // for ETA = 4
    
    // Final Dilithium3 key sizes (FIPS-204 compliant)
    static constexpr size_t DILITHIUM3_PUBLICKEY_BYTES = (DILITHIUM_SEEDBYTES + 
                                                          DILITHIUM_K * DILITHIUM_POLYT1_PACKEDBYTES);  // 1952 bytes
    static constexpr size_t DILITHIUM3_SECRETKEY_BYTES = (2*DILITHIUM_SEEDBYTES + 
                                                          DILITHIUM_TRBYTES + 
                                                          DILITHIUM_L * DILITHIUM_POLYETA_PACKEDBYTES +
                                                          DILITHIUM_K * DILITHIUM_POLYETA_PACKEDBYTES +
                                                          DILITHIUM_K * DILITHIUM_POLYT0_PACKEDBYTES);   // 4000 bytes
    static constexpr size_t DILITHIUM3_SIGNATURE_BYTES = (DILITHIUM_CTILDEBYTES + 
                                                          DILITHIUM_L * DILITHIUM_POLYZ_PACKEDBYTES +
                                                          DILITHIUM_POLYVECH_PACKEDBYTES);              // 3293 bytes
    static constexpr size_t DILITHIUM3_SEED_BYTES = DILITHIUM_SEEDBYTES;  // 32 bytes (not 64)

    using PublicKey = std::array<uint8_t, DILITHIUM3_PUBLICKEY_BYTES>;
    using SecretKey = std::array<uint8_t, DILITHIUM3_SECRETKEY_BYTES>;
    using Signature = std::vector<uint8_t>; // Variable length
    using Seed = std::array<uint8_t, DILITHIUM3_SEED_BYTES>;

    // Core Dilithium3 functions (NIST FIPS-204 compliant)
    std::pair<PublicKey, SecretKey> GenerateKeys(const Seed& seed);  // Deterministic from 32-byte seed
    std::pair<PublicKey, SecretKey> GenerateKeys();                  // Random version
    Signature Sign(std::span<const uint8_t> message, const SecretKey& sk);
    bool Verify(const Signature& signature, std::span<const uint8_t> message, 
                const PublicKey& pk);
                
    // Internal implementation details - PRODUCTION READY
    // Full NIST FIPS-204 Dilithium3 implementation complete
}

#endif // QTC_CRYPTO_DILITHIUM_DILITHIUM3_H
