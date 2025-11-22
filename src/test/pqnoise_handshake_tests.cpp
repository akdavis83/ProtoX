// Copyright (c) 2024 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <net/pq_noise.h>
#include <util/strencodings.h>
#include <random.h>
#include <oqs/oqs.h>
#include <vector>
#include <span>

BOOST_FIXTURE_TEST_SUITE(pqnoise_handshake_tests, BasicTestingSetup)

// Helper: Generate Kyber-1024 keypair
static bool GenerateKyberKeypair(std::vector<uint8_t>& pk, std::vector<uint8_t>& sk)
{
    OQS_KEM* kem = OQS_KEM_new(OQS_KEM_alg_kyber_1024);
    if (!kem) return false;
    
    pk.resize(kem->length_public_key);
    sk.resize(kem->length_secret_key);
    
    OQS_STATUS status = OQS_KEM_keypair(kem, pk.data(), sk.data());
    OQS_KEM_free(kem);
    return status == OQS_SUCCESS;
}

BOOST_AUTO_TEST_CASE(pqnoise_basic_handshake)
{
    // Generate server Kyber-1024 keypair
    std::vector<uint8_t> server_pk, server_sk;
    BOOST_REQUIRE(GenerateKyberKeypair(server_pk, server_sk));
    
    // Setup server and client sessions
    PQNoiseConfig server_cfg{server_pk, server_sk, std::nullopt};
    PQNoiseConfig client_cfg{server_pk, std::nullopt, std::nullopt};
    
    PQNoiseSession server(PQNoiseRole::Server, server_cfg);
    PQNoiseSession client(PQNoiseRole::Client, client_cfg);
    
    // Test handshake flow
    std::vector<uint8_t> client_hello, server_hello;
    
    // Step 1: Client starts handshake
    BOOST_REQUIRE(client.StartHandshake(client_hello));
    BOOST_CHECK(!client_hello.empty());
    BOOST_CHECK(client_hello.size() > 4); // At least magic + version + suite
    
    // Step 2: Server processes ClientHello and responds
    BOOST_REQUIRE(server.OnHandshakeMessage(client_hello, server_hello));
    BOOST_CHECK(!server_hello.empty());
    BOOST_CHECK(server_hello.size() >= 7); // magic + version + sig_len
    
    // Step 3: Client processes ServerHello
    std::vector<uint8_t> dummy_response;
    BOOST_REQUIRE(client.OnHandshakeMessage(server_hello, dummy_response));
    BOOST_CHECK(dummy_response.empty()); // Client finish doesn't send response
    
    // Both sessions should be established
    BOOST_CHECK(client.Established());
    BOOST_CHECK(server.Established());
    BOOST_CHECK(client.LastError().empty());
    BOOST_CHECK(server.LastError().empty());
}

BOOST_AUTO_TEST_CASE(pqnoise_aead_encrypt_decrypt)
{
    // Setup established session (reuse handshake code)
    std::vector<uint8_t> server_pk, server_sk;
    BOOST_REQUIRE(GenerateKyberKeypair(server_pk, server_sk));
    
    PQNoiseConfig server_cfg{server_pk, server_sk, std::nullopt};
    PQNoiseConfig client_cfg{server_pk, std::nullopt, std::nullopt};
    
    PQNoiseSession server(PQNoiseRole::Server, server_cfg);
    PQNoiseSession client(PQNoiseRole::Client, client_cfg);
    
    // Complete handshake
    std::vector<uint8_t> client_hello, server_hello, dummy;
    BOOST_REQUIRE(client.StartHandshake(client_hello));
    BOOST_REQUIRE(server.OnHandshakeMessage(client_hello, server_hello));
    BOOST_REQUIRE(client.OnHandshakeMessage(server_hello, dummy));
    
    BOOST_REQUIRE(client.Established());
    BOOST_REQUIRE(server.Established());
    
    // Test AEAD encryption/decryption
    std::vector<uint8_t> plaintext = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
    std::vector<uint8_t> ciphertext, decrypted;
    
    // Client encrypts message
    BOOST_REQUIRE(client.Seal(plaintext, ciphertext));
    BOOST_CHECK(!ciphertext.empty());
    BOOST_CHECK(ciphertext.size() == plaintext.size() + 24); // 8 bytes nonce + 16 bytes tag
    
    // Server decrypts message
    BOOST_REQUIRE(server.Open(ciphertext, decrypted));
    BOOST_CHECK(decrypted == plaintext);
    
    // Test server->client direction
    std::vector<uint8_t> response = {'A', 'C', 'K'};
    std::vector<uint8_t> response_encrypted, response_decrypted;
    
    BOOST_REQUIRE(server.Seal(response, response_encrypted));
    BOOST_REQUIRE(client.Open(response_encrypted, response_decrypted));
    BOOST_CHECK(response_decrypted == response);
}

BOOST_AUTO_TEST_CASE(pqnoise_rekey_test)
{
    // Setup established session
    std::vector<uint8_t> server_pk, server_sk;
    BOOST_REQUIRE(GenerateKyberKeypair(server_pk, server_sk));
    
    PQNoiseConfig server_cfg{server_pk, server_sk, std::nullopt};
    PQNoiseConfig client_cfg{server_pk, std::nullopt, std::nullopt};
    
    PQNoiseSession server(PQNoiseRole::Server, server_cfg);
    PQNoiseSession client(PQNoiseRole::Client, client_cfg);
    
    // Complete handshake
    std::vector<uint8_t> client_hello, server_hello, dummy;
    BOOST_REQUIRE(client.StartHandshake(client_hello));
    BOOST_REQUIRE(server.OnHandshakeMessage(client_hello, server_hello));
    BOOST_REQUIRE(client.OnHandshakeMessage(server_hello, dummy));
    
    // Test multiple encryptions (nonce increments)
    std::vector<uint8_t> plaintext = {'T', 'e', 's', 't'};
    
    for (int i = 0; i < 10; i++) {
        std::vector<uint8_t> ciphertext1, ciphertext2, decrypted1, decrypted2;
        
        // Encrypt same plaintext twice - should produce different ciphertexts due to nonce
        BOOST_REQUIRE(client.Seal(plaintext, ciphertext1));
        BOOST_REQUIRE(client.Seal(plaintext, ciphertext2));
        BOOST_CHECK(ciphertext1 != ciphertext2); // Different nonces
        
        // Both should decrypt correctly
        BOOST_REQUIRE(server.Open(ciphertext1, decrypted1));
        BOOST_REQUIRE(server.Open(ciphertext2, decrypted2));
        BOOST_CHECK(decrypted1 == plaintext);
        BOOST_CHECK(decrypted2 == plaintext);
    }
}

BOOST_AUTO_TEST_CASE(pqnoise_error_handling)
{
    std::vector<uint8_t> server_pk, server_sk;
    BOOST_REQUIRE(GenerateKyberKeypair(server_pk, server_sk));
    
    // Test invalid server config (no secret key)
    PQNoiseConfig invalid_server_cfg{server_pk, std::nullopt, std::nullopt};
    PQNoiseSession invalid_server(PQNoiseRole::Server, invalid_server_cfg);
    
    std::vector<uint8_t> client_hello = {'F', 'a', 'k', 'e'};
    std::vector<uint8_t> server_response;
    
    // Server should fail without secret key
    BOOST_CHECK(!invalid_server.OnHandshakeMessage(client_hello, server_response));
    BOOST_CHECK(!invalid_server.LastError().empty());
    
    // Test invalid client config (no server public key)
    PQNoiseConfig invalid_client_cfg{{}, std::nullopt, std::nullopt};
    PQNoiseSession invalid_client(PQNoiseRole::Client, invalid_client_cfg);
    
    std::vector<uint8_t> invalid_hello;
    BOOST_CHECK(!invalid_client.StartHandshake(invalid_hello));
    BOOST_CHECK(!invalid_client.LastError().empty());
}

BOOST_AUTO_TEST_CASE(pqnoise_replay_protection)
{
    // Setup established session
    std::vector<uint8_t> server_pk, server_sk;
    BOOST_REQUIRE(GenerateKyberKeypair(server_pk, server_sk));
    
    PQNoiseConfig server_cfg{server_pk, server_sk, std::nullopt};
    PQNoiseConfig client_cfg{server_pk, std::nullopt, std::nullopt};
    
    PQNoiseSession server(PQNoiseRole::Server, server_cfg);
    PQNoiseSession client(PQNoiseRole::Client, client_cfg);
    
    // Complete handshake
    std::vector<uint8_t> client_hello, server_hello, dummy;
    BOOST_REQUIRE(client.StartHandshake(client_hello));
    BOOST_REQUIRE(server.OnHandshakeMessage(client_hello, server_hello));
    BOOST_REQUIRE(client.OnHandshakeMessage(server_hello, dummy));
    
    // Encrypt a message
    std::vector<uint8_t> plaintext = {'T', 'e', 's', 't'};
    std::vector<uint8_t> ciphertext, decrypted;
    
    BOOST_REQUIRE(client.Seal(plaintext, ciphertext));
    BOOST_REQUIRE(server.Open(ciphertext, decrypted));
    BOOST_CHECK(decrypted == plaintext);
    
    // Try to replay the same ciphertext - should fail
    std::vector<uint8_t> replay_decrypted;
    BOOST_CHECK(!server.Open(ciphertext, replay_decrypted));
}

BOOST_AUTO_TEST_CASE(pqnoise_wire_format)
{
    std::vector<uint8_t> server_pk, server_sk;
    BOOST_REQUIRE(GenerateKyberKeypair(server_pk, server_sk));
    
    PQNoiseConfig client_cfg{server_pk, std::nullopt, std::nullopt};
    PQNoiseSession client(PQNoiseRole::Client, client_cfg);
    
    std::vector<uint8_t> client_hello;
    BOOST_REQUIRE(client.StartHandshake(client_hello));
    
    // Verify ClientHello wire format
    BOOST_REQUIRE(client_hello.size() >= 8);
    
    // Check magic bytes
    BOOST_CHECK(client_hello[0] == 0xF9);
    BOOST_CHECK(client_hello[1] == 0xBE);
    BOOST_CHECK(client_hello[2] == 0xB4);
    BOOST_CHECK(client_hello[3] == 0xD9);
    
    // Check version
    BOOST_CHECK(client_hello[4] == 0x00);
    BOOST_CHECK(client_hello[5] == 0x01);
    
    // Check ciphertext length field
    uint16_t ct_len = (static_cast<uint16_t>(client_hello[6]) << 8) | client_hello[7];
    BOOST_CHECK(ct_len > 0);
    BOOST_CHECK(client_hello.size() >= 8 + ct_len);
}

BOOST_AUTO_TEST_CASE(pqnoise_large_message_test)
{
    // Setup established session
    std::vector<uint8_t> server_pk, server_sk;
    BOOST_REQUIRE(GenerateKyberKeypair(server_pk, server_sk));
    
    PQNoiseConfig server_cfg{server_pk, server_sk, std::nullopt};
    PQNoiseConfig client_cfg{server_pk, std::nullopt, std::nullopt};
    
    PQNoiseSession server(PQNoiseRole::Server, server_cfg);
    PQNoiseSession client(PQNoiseRole::Client, client_cfg);
    
    // Complete handshake
    std::vector<uint8_t> client_hello, server_hello, dummy;
    BOOST_REQUIRE(client.StartHandshake(client_hello));
    BOOST_REQUIRE(server.OnHandshakeMessage(client_hello, server_hello));
    BOOST_REQUIRE(client.OnHandshakeMessage(server_hello, dummy));
    
    // Test with large message (1KB)
    std::vector<uint8_t> large_plaintext(1024);
    GetRandBytes(large_plaintext);
    
    std::vector<uint8_t> large_ciphertext, large_decrypted;
    
    BOOST_REQUIRE(client.Seal(large_plaintext, large_ciphertext));
    BOOST_REQUIRE(server.Open(large_ciphertext, large_decrypted));
    BOOST_CHECK(large_decrypted == large_plaintext);
}

BOOST_AUTO_TEST_SUITE_END()
