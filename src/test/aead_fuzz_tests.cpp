// Copyright (c) 2024 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <crypto/chacha20poly1305.h>
#include <util/strencodings.h>
#include <random.h>
#include <span>
#include <vector>

BOOST_FIXTURE_TEST_SUITE(aead_fuzz_tests, BasicTestingSetup)

// Helper: Generate random data
static std::vector<uint8_t> RandomBytes(size_t len)
{
    std::vector<uint8_t> data(len);
    GetRandBytes(data);
    return data;
}

// Helper: Corrupt a random byte
static void CorruptRandomByte(std::vector<uint8_t>& data)
{
    if (data.empty()) return;
    size_t idx = GetRand<size_t>(data.size());
    data[idx] ^= 0x01; // Flip one bit
}

BOOST_AUTO_TEST_CASE(aead_basic_seal_open)
{
    // Test basic AEAD functionality
    auto key = RandomBytes(AEADChaCha20Poly1305::KEY_LEN);
    AEADChaCha20Poly1305 aead(key);
    
    auto nonce = RandomBytes(AEADChaCha20Poly1305::NONCE_LEN);
    std::vector<uint8_t> ad = RandomBytes(16);
    std::vector<uint8_t> plaintext = RandomBytes(128);
    
    std::vector<uint8_t> ciphertext, decrypted;
    
    // Seal
    BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, plaintext, ciphertext));
    BOOST_CHECK(ciphertext.size() == plaintext.size() + AEADChaCha20Poly1305::TAG_LEN);
    
    // Open
    BOOST_REQUIRE(aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, ciphertext, decrypted));
    BOOST_CHECK(decrypted == plaintext);
}

BOOST_AUTO_TEST_CASE(aead_corrupted_tag)
{
    auto key = RandomBytes(AEADChaCha20Poly1305::KEY_LEN);
    AEADChaCha20Poly1305 aead(key);
    
    auto nonce = RandomBytes(AEADChaCha20Poly1305::NONCE_LEN);
    std::vector<uint8_t> ad = RandomBytes(8);
    std::vector<uint8_t> plaintext = RandomBytes(64);
    
    std::vector<uint8_t> ciphertext, decrypted;
    
    // Seal valid message
    BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, plaintext, ciphertext));
    
    // Corrupt the tag (last 16 bytes)
    BOOST_REQUIRE(ciphertext.size() >= AEADChaCha20Poly1305::TAG_LEN);
    size_t tag_start = ciphertext.size() - AEADChaCha20Poly1305::TAG_LEN;
    ciphertext[tag_start] ^= 0x01;
    
    // Open should fail
    BOOST_CHECK(!aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                          ad, ciphertext, decrypted));
}

BOOST_AUTO_TEST_CASE(aead_corrupted_ciphertext)
{
    auto key = RandomBytes(AEADChaCha20Poly1305::KEY_LEN);
    AEADChaCha20Poly1305 aead(key);
    
    auto nonce = RandomBytes(AEADChaCha20Poly1305::NONCE_LEN);
    std::vector<uint8_t> ad;
    std::vector<uint8_t> plaintext = RandomBytes(100);
    
    std::vector<uint8_t> ciphertext, decrypted;
    
    // Seal valid message
    BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, plaintext, ciphertext));
    
    // Corrupt ciphertext (not tag)
    if (ciphertext.size() > AEADChaCha20Poly1305::TAG_LEN) {
        size_t ct_len = ciphertext.size() - AEADChaCha20Poly1305::TAG_LEN;
        size_t corrupt_idx = GetRand<size_t>(ct_len);
        ciphertext[corrupt_idx] ^= 0xFF;
    }
    
    // Open should fail due to tag mismatch
    BOOST_CHECK(!aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                          ad, ciphertext, decrypted));
}

BOOST_AUTO_TEST_CASE(aead_wrong_ad)
{
    auto key = RandomBytes(AEADChaCha20Poly1305::KEY_LEN);
    AEADChaCha20Poly1305 aead(key);
    
    auto nonce = RandomBytes(AEADChaCha20Poly1305::NONCE_LEN);
    std::vector<uint8_t> ad1 = RandomBytes(32);
    std::vector<uint8_t> ad2 = RandomBytes(32);
    std::vector<uint8_t> plaintext = RandomBytes(80);
    
    std::vector<uint8_t> ciphertext, decrypted;
    
    // Seal with ad1
    BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad1, plaintext, ciphertext));
    
    // Try to open with ad2 (different AD)
    BOOST_CHECK(!aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                          ad2, ciphertext, decrypted));
    
    // Should work with correct AD
    BOOST_REQUIRE(aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad1, ciphertext, decrypted));
    BOOST_CHECK(decrypted == plaintext);
}

BOOST_AUTO_TEST_CASE(aead_nonce_reuse_detection)
{
    auto key = RandomBytes(AEADChaCha20Poly1305::KEY_LEN);
    AEADChaCha20Poly1305 aead(key);
    
    auto nonce = RandomBytes(AEADChaCha20Poly1305::NONCE_LEN);
    std::vector<uint8_t> ad;
    std::vector<uint8_t> msg1 = RandomBytes(50);
    std::vector<uint8_t> msg2 = RandomBytes(50);
    
    std::vector<uint8_t> ct1, ct2;
    
    // Encrypt two different messages with same nonce
    BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, msg1, ct1));
    BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, msg2, ct2));
    
    // With same nonce, ciphertexts should be different (due to different plaintexts)
    // but this is a dangerous pattern in real usage
    BOOST_CHECK(ct1 != ct2);
    
    // Both should decrypt correctly individually
    std::vector<uint8_t> pt1, pt2;
    BOOST_REQUIRE(aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, ct1, pt1));
    BOOST_REQUIRE(aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, ct2, pt2));
    
    BOOST_CHECK(pt1 == msg1);
    BOOST_CHECK(pt2 == msg2);
}

BOOST_AUTO_TEST_CASE(aead_truncated_ciphertext)
{
    auto key = RandomBytes(AEADChaCha20Poly1305::KEY_LEN);
    AEADChaCha20Poly1305 aead(key);
    
    auto nonce = RandomBytes(AEADChaCha20Poly1305::NONCE_LEN);
    std::vector<uint8_t> ad;
    std::vector<uint8_t> plaintext = RandomBytes(200);
    
    std::vector<uint8_t> ciphertext, decrypted;
    
    // Seal valid message
    BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                           ad, plaintext, ciphertext));
    
    // Test various truncations
    for (size_t truncate = 1; truncate <= std::min(ciphertext.size(), size_t(32)); ++truncate) {
        std::vector<uint8_t> truncated(ciphertext.begin(), ciphertext.end() - truncate);
        
        // Should fail to open truncated ciphertext
        BOOST_CHECK(!aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                              ad, truncated, decrypted));
    }
    
    // Empty ciphertext should fail
    std::vector<uint8_t> empty;
    BOOST_CHECK(!aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                          ad, empty, decrypted));
}

BOOST_AUTO_TEST_CASE(aead_various_sizes)
{
    auto key = RandomBytes(AEADChaCha20Poly1305::KEY_LEN);
    AEADChaCha20Poly1305 aead(key);
    
    // Test various message sizes
    std::vector<size_t> sizes = {0, 1, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 255, 256, 257, 1023, 1024, 1025};
    
    for (size_t size : sizes) {
        auto nonce = RandomBytes(AEADChaCha20Poly1305::NONCE_LEN);
        std::vector<uint8_t> ad = RandomBytes(GetRand<size_t>(64));
        std::vector<uint8_t> plaintext = RandomBytes(size);
        
        std::vector<uint8_t> ciphertext, decrypted;
        
        // Seal
        BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                               ad, plaintext, ciphertext));
        BOOST_CHECK(ciphertext.size() == plaintext.size() + AEADChaCha20Poly1305::TAG_LEN);
        
        // Open
        BOOST_REQUIRE(aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                               ad, ciphertext, decrypted));
        BOOST_CHECK(decrypted == plaintext);
    }
}

BOOST_AUTO_TEST_CASE(aead_stress_test)
{
    auto key = RandomBytes(AEADChaCha20Poly1305::KEY_LEN);
    AEADChaCha20Poly1305 aead(key);
    
    // Stress test with many operations
    for (int i = 0; i < 100; ++i) {
        auto nonce = RandomBytes(AEADChaCha20Poly1305::NONCE_LEN);
        std::vector<uint8_t> ad = RandomBytes(GetRand<size_t>(128));
        std::vector<uint8_t> plaintext = RandomBytes(GetRand<size_t>(1000));
        
        std::vector<uint8_t> ciphertext, decrypted;
        
        BOOST_REQUIRE(aead.Seal(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                               ad, plaintext, ciphertext));
        BOOST_REQUIRE(aead.Open(std::span<const uint8_t, AEADChaCha20Poly1305::NONCE_LEN>(nonce.data(), AEADChaCha20Poly1305::NONCE_LEN),
                               ad, ciphertext, decrypted));
        BOOST_CHECK(decrypted == plaintext);
    }
}

BOOST_AUTO_TEST_SUITE_END()
EOF}
