#ifndef QTC_WALLET_H
#define QTC_WALLET_H

#include <crypto/kyber/kyber1024.h>
#include <crypto/dilithium/dilithium3.h>
#include <string>

namespace qtc {
    struct QuantumWallet {
        std::string address;                            // "qtc1v5xpy4uu72sj3z..."
        qtc_kyber::PublicKey kyber_public;
        qtc_kyber::SecretKey kyber_private;
        qtc_dilithium::PublicKey dilithium_public;
        qtc_dilithium::SecretKey dilithium_private;
        qtc_kyber::SharedSecret shared_secret;
        std::array<uint8_t, 64> entropy;  // SHA3-512 output (64 bytes)
    };

    // Production quantum wallet generation (self-contained C++ implementation)
    QuantumWallet GenerateQuantumWallet();
    
    // Address generation (SHA3-256 + bech32 "qtc" prefix)
    std::string GenerateQTCAddress(const qtc_dilithium::PublicKey& dilithium_pk);
    
    // Entropy derivation (SHA3-512 from shared secret - FIPS-202 compliant)
    std::array<uint8_t, 64> DeriveEntropy(const qtc_kyber::SharedSecret& shared_secret);
}

#endif // QTC_WALLET_H
