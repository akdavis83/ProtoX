// QTC BLAKE3 Mining Algorithm Tests
// Tests for quantum-safe BLAKE3 mining implementation

#include <test/util/setup_common.h>
#include <crypto/blake3/blake3.h>
#include <hash.h>
#include <uint256.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(blake3_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(blake3_basic_hash)
{
    // Test basic BLAKE3 hashing functionality
    const std::string test_input = "QTC quantum-safe mining test";
    uint8_t hash_output[BLAKE3_OUT_LEN];
    
    blake3_hash(test_input.c_str(), test_input.length(), hash_output);
    
    // Verify we got a 32-byte hash
    BOOST_CHECK_EQUAL(BLAKE3_OUT_LEN, 32);
    
    // Verify hash is not all zeros
    bool non_zero = false;
    for (int i = 0; i < BLAKE3_OUT_LEN; i++) {
        if (hash_output[i] != 0) {
            non_zero = true;
            break;
        }
    }
    BOOST_CHECK(non_zero);
}

BOOST_AUTO_TEST_CASE(blake3_deterministic)
{
    // Test that BLAKE3 produces deterministic results
    const std::string test_input = "QTC BLAKE3 deterministic test";
    uint8_t hash1[BLAKE3_OUT_LEN];
    uint8_t hash2[BLAKE3_OUT_LEN];
    
    blake3_hash(test_input.c_str(), test_input.length(), hash1);
    blake3_hash(test_input.c_str(), test_input.length(), hash2);
    
    // Hashes should be identical
    BOOST_CHECK(memcmp(hash1, hash2, BLAKE3_OUT_LEN) == 0);
}

BOOST_AUTO_TEST_CASE(blake3_different_inputs)
{
    // Test that different inputs produce different hashes
    const std::string input1 = "QTC input 1";
    const std::string input2 = "QTC input 2";
    uint8_t hash1[BLAKE3_OUT_LEN];
    uint8_t hash2[BLAKE3_OUT_LEN];
    
    blake3_hash(input1.c_str(), input1.length(), hash1);
    blake3_hash(input2.c_str(), input2.length(), hash2);
    
    // Hashes should be different
    BOOST_CHECK(memcmp(hash1, hash2, BLAKE3_OUT_LEN) != 0);
}

BOOST_AUTO_TEST_CASE(qblake3_class_test)
{
    // Test QTC QBLAKE3 hasher class
    const std::string test_input = "QTC QBLAKE3 class test";
    
    QBLAKE3 hasher;
    uint256 result;
    
    hasher.Write(std::span{reinterpret_cast<const unsigned char*>(test_input.c_str()), test_input.length()});
    hasher.Finalize(result);
    
    // Verify we got a valid hash
    BOOST_CHECK(!result.IsNull());
    
    // Test reset functionality
    hasher.Reset();
    uint256 result2;
    hasher.Write(std::span{reinterpret_cast<const unsigned char*>(test_input.c_str()), test_input.length()});
    hasher.Finalize(result2);
    
    // Results should be the same after reset
    BOOST_CHECK(result == result2);
}

BOOST_AUTO_TEST_CASE(blake3_vs_sha256_performance)
{
    // Performance comparison test (informational)
    const std::string large_input(10000, 'A'); // 10KB of data
    uint8_t blake3_hash[BLAKE3_OUT_LEN];
    
    // Time BLAKE3
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        blake3_hash(large_input.c_str(), large_input.length(), blake3_hash);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto blake3_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // BLAKE3 should complete without errors
    BOOST_CHECK(blake3_duration.count() > 0);
    
    // Log performance for analysis
    std::cout << "BLAKE3 1000x 10KB hashes: " << blake3_duration.count() << " microseconds" << std::endl;
}

BOOST_AUTO_TEST_CASE(qtc_mining_simulation)
{
    // Simulate QTC mining with BLAKE3
    struct MockBlockHeader {
        uint32_t version = 1;
        uint256 hashPrevBlock;
        uint256 hashMerkleRoot;
        uint32_t nTime = 1234567890;
        uint32_t nBits = 0x1d00ffff;
        uint32_t nNonce = 0;
    } header;
    
    // Try different nonces to simulate mining
    uint8_t hash[BLAKE3_OUT_LEN];
    bool found_valid_hash = false;
    
    for (uint32_t nonce = 0; nonce < 100000; nonce++) {
        header.nNonce = nonce;
        
        // Hash the block header with BLAKE3
        qtc_blake3_hash_block_header(&header, sizeof(header), hash);
        
        // Check if hash starts with zeros (simple difficulty check)
        if (hash[0] == 0x00 && hash[1] == 0x00) {
            found_valid_hash = true;
            break;
        }
    }
    
    // We should eventually find a hash with leading zeros
    BOOST_CHECK(found_valid_hash);
}

BOOST_AUTO_TEST_CASE(blake3_known_vectors)
{
    // Test with known BLAKE3 test vectors
    const uint8_t empty_input[] = "";
    uint8_t expected_empty[BLAKE3_OUT_LEN] = {
        0xaf, 0x13, 0x49, 0xb9, 0xf5, 0xf9, 0xa1, 0xa6,
        0xa0, 0x40, 0x4d, 0xea, 0x36, 0xdc, 0xc9, 0x49,
        0x9b, 0xcb, 0x25, 0xc9, 0xad, 0xc1, 0x12, 0xb7,
        0xcc, 0x9a, 0x93, 0xca, 0xe4, 0x1f, 0x32, 0x62
    };
    
    uint8_t result[BLAKE3_OUT_LEN];
    blake3_hash(empty_input, 0, result);
    
    // Verify against expected result
    BOOST_CHECK(memcmp(result, expected_empty, BLAKE3_OUT_LEN) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
