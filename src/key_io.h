// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QTC_KEY_IO_H
#define QTC_KEY_IO_H

#include <addresstype.h>
#include <chainparams.h>
#include <key.h>
#include <pubkey.h>
#include <crypto/dilithium/dilithium3.h>
#include <crypto/kyber/kyber1024.h>

#include <string>

CKey DecodeSecret(const std::string& str);
std::string EncodeSecret(const CKey& key);

CExtKey DecodeExtKey(const std::string& str);
std::string EncodeExtKey(const CExtKey& extkey);
CExtPubKey DecodeExtPubKey(const std::string& str);
std::string EncodeExtPubKey(const CExtPubKey& extpubkey);

std::string EncodeDestination(const CTxDestination& dest);
CTxDestination DecodeDestination(const std::string& str);
CTxDestination DecodeDestination(const std::string& str, std::string& error_msg, std::vector<int>* error_locations = nullptr);
bool IsValidDestinationString(const std::string& str);
bool IsValidDestinationString(const std::string& str, const CChainParams& params);

// Functions for loading and storing Dilithium3 identity keys
qtc_dilithium::SecretKey LoadDilithiumKey();
void StoreDilithiumKey(const qtc_dilithium::SecretKey& sk);

// Functions for loading and storing Kyber1024 KEM keys
qtc_kyber::keypair LoadKyberKey();
qtc_kyber::keypair LoadPrevKyberKey();
void StoreKyberKey(const qtc_kyber::keypair& key);

#endif // QTC_KEY_IO_H
