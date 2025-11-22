#include <qtc_wallet.h>
#include <crypto/sha3.h>
#include <random.h>
#include <bech32.h>
#include <crypto/qtc_hash.h> // QTC_Program20_From_PK_SHA3_256

namespace qtc {
    // Production quantum wallet generation (NIST FIPS-203/204 compliant C++ implementation)
    QuantumWallet GenerateQuantumWallet() {
        QuantumWallet wallet;
        
        // Step 1: Generate Kyber1024 keypair with cryptographically secure random seed
        std::array<uint8_t, 64> kyber_seed;
        GetStrongRandBytes(kyber_seed);
        
        auto [kyber_pk, kyber_sk] = qtc_kyber::KeyGen1024(kyber_seed);
        auto [ciphertext, shared_secret] = qtc_kyber::Encrypt1024(kyber_pk);
        
        wallet.kyber_public = kyber_pk;
        wallet.kyber_private = kyber_sk;
        wallet.shared_secret = shared_secret;
        
        // Step 2: Derive deterministic entropy using SHA3-512 (FIPS-202 compliant)
        wallet.entropy = DeriveEntropy(shared_secret);
        
        // Step 3: Generate Dilithium3 keypair from 32-byte seed (FIPS-204 compliant)
        qtc_dilithium::Seed dilithium_seed;
        std::copy(wallet.entropy.begin(), wallet.entropy.begin() + 32, dilithium_seed.begin());
        auto [dilithium_pk, dilithium_sk] = qtc_dilithium::GenerateKeys(dilithium_seed);
        
        wallet.dilithium_public = dilithium_pk;
        wallet.dilithium_private = dilithium_sk;
        
        // Step 4: Generate QTC address from Dilithium public key
        wallet.address = GenerateQTCAddress(dilithium_pk);
        
        return wallet;
    }

    // Generate QTC address (canonical: SHA3-256 -> 20-byte program, bech32m v1)
    std::string GenerateQTCAddress(const qtc_dilithium::PublicKey& dilithium_pk) {
        // Step 1: Derive 20-byte witness program from SHA3-256(pubkey)
        auto program20 = QTC_Program20_From_PK_SHA3_256(dilithium_pk.data(), dilithium_pk.size());
        
        // Step 2: bech32m encode for witness v1
        auto words = bech32::ConvertBits<8,5,true>(program20.begin(), program20.end());
        std::vector<unsigned char> data; data.reserve(1 + words.size());
        data.push_back(1); // v1
        data.insert(data.end(), words.begin(), words.end());
        return bech32::Encode(bech32::Encoding::BECH32M, "qtc", data);
    }

    // Derive entropy from shared secret using SHA3-512 (FIPS-202 standard)
    std::array<uint8_t, 64> DeriveEntropy(const qtc_kyber::SharedSecret& shared_secret) {
        std::array<uint8_t, 64> entropy;
        CSHA3_512().Write(shared_secret.data(), shared_secret.size()).Finalize(entropy.data());
        return entropy;
    }
}
