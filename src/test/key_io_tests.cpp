// Copyright (c) 2021 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <key_io.h>
#include <util/fs.h>
#include <test/util/setup_common.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(key_io_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(key_io_dilithium)
{
    // Test storing and loading a dilithium key
    auto keys = qtc_dilithium::GenerateKeys();
    StoreDilithiumKey(keys.second);
    qtc_dilithium::SecretKey loaded_sk = LoadDilithiumKey();
    BOOST_CHECK(keys.second == loaded_sk);
}

BOOST_AUTO_TEST_CASE(key_io_kyber)
{
    // Test storing and loading a kyber key
    auto keys = qtc_kyber::KeyGen1024();
    StoreKyberKey(keys);
    qtc_kyber::keypair loaded_keys = LoadKyberKey();
    BOOST_CHECK(keys.second == loaded_keys.second);
}

BOOST_AUTO_TEST_SUITE_END()