// Copyright (c) 2014-present The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <key_io.h>

#include <base58.h>
#include <bech32.h>
#include <script/interpreter.h>
#include <script/solver.h>
#include <tinyformat.h>
#include <util/strencodings.h>
#include <crypto/qtc_hash.h> // QTC_Program20_From_PK_SHA3_256
#include <common/args.h>
#include <util/fs.h>
#include <fstream>
#include <chrono>

#include <algorithm>
#include <cassert>
#include <cstring>

// Functions for loading and storing Dilithium3 identity keys
std::pair<qtc_dilithium::PublicKey, qtc_dilithium::SecretKey> LoadDilithiumIdentityKey() {
    fs::path dir = gArgs.GetDataDirNet() / "pqnoise";
    fs::create_directory(dir);
    fs::path path = dir / "id_dilithium.sk";
    
    if (!fs::exists(path)) {
        // Generate new identity key
        auto keys = qtc_dilithium::GenerateKeys();
        StoreDilithiumIdentityKey(keys);
        return keys;
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }
    
    qtc_dilithium::SecretKey sk;
    file.read(reinterpret_cast<char*>(sk.data()), sk.size());
    
    // Derive public key from secret key
    qtc_dilithium::PublicKey pk = qtc_dilithium::PublicKeyFromSecretKey(sk);
    
    return {pk, sk};
}

void StoreDilithiumIdentityKey(const std::pair<qtc_dilithium::PublicKey, qtc_dilithium::SecretKey>& keys) {
    fs::path dir = gArgs.GetDataDirNet() / "pqnoise";
    fs::create_directory(dir);
    
    // Store secret key
    fs::path sk_path = dir / "id_dilithium.sk";
    std::ofstream sk_file(sk_path, std::ios::binary);
    if (sk_file.is_open()) {
        sk_file.write(reinterpret_cast<const char*>(keys.second.data()), keys.second.size());
    }
    
    // Store public key
    fs::path pk_path = dir / "id_dilithium.pk";
    std::ofstream pk_file(pk_path, std::ios::binary);
    if (pk_file.is_open()) {
        pk_file.write(reinterpret_cast<const char*>(keys.first.data()), keys.first.size());
    }
}

bool ShouldRotateIdentityKey() {
    fs::path path = gArgs.GetDataDirNet() / "pqnoise" / "id_dilithium.sk";
    if (!fs::exists(path)) {
        return true; // No key exists, need to generate
    }
    
    auto mod_time = fs::last_write_time(path);
    auto now = std::chrono::file_clock::now();
    // Rotate annually (365 * 24 hours)
    return (now - mod_time) > std::chrono::hours(365 * 24);
}

// Functions for loading and storing Kyber1024 KEM keys
std::pair<qtc_kyber::PublicKey, qtc_kyber::SecretKey> LoadKyberKey() {
    fs::path dir = gArgs.GetDataDirNet() / "pqnoise";
    fs::create_directory(dir);
    fs::path path = dir / "kem.sk";
    fs::path prev_path = dir / "kem.sk.prev";

    if (fs::exists(path)) {
        auto mod_time = fs::last_write_time(path);
        auto now = std::chrono::file_clock::now();
        if (now - mod_time > std::chrono::hours(24)) {
            fs::rename(path, prev_path);
        }
    }

    if (!fs::exists(path)) {
        auto keys = qtc_kyber::KeyGen1024();
        StoreKyberKey(keys);
        return keys;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    qtc_kyber::SecretKey sk;
    file.read(reinterpret_cast<char*>(sk.data()), sk.size());
    qtc_kyber::PublicKey pk = qtc_kyber::PublicKeyFromSecretKey(sk);
    return {pk, sk};
}

std::pair<qtc_kyber::PublicKey, qtc_kyber::SecretKey> LoadPrevKyberKey() {
    fs::path path = gArgs.GetDataDirNet() / "pqnoise" / "kem.sk.prev";
    if (!fs::exists(path)) {
        return {};
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    qtc_kyber::SecretKey sk;
    file.read(reinterpret_cast<char*>(sk.data()), sk.size());
    qtc_kyber::PublicKey pk = qtc_kyber::PublicKeyFromSecretKey(sk);
    return {pk, sk};
}

void StoreKyberKey(const std::pair<qtc_kyber::PublicKey, qtc_kyber::SecretKey>& keys) {
    fs::path path = gArgs.GetDataDirNet() / "pqnoise";
    fs::create_directory(path);
    path /= "kem.sk";

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return;
    }

    file.write(reinterpret_cast<const char*>(keys.second.data()), keys.second.size());
}

// Rekey management functions
struct PQRekeyPolicy {
    uint64_t max_bytes = 33554432;  // 32 MB default
    uint64_t max_minutes = 30;      // 30 minutes default
    
    PQRekeyPolicy() {
        // Load from args
        max_bytes = gArgs.GetIntArg("-pq-rekey-bytes", max_bytes);
        max_minutes = gArgs.GetIntArg("-pq-rekey-time", max_minutes);
    }
    
    bool ShouldRekey(uint64_t bytes_sent, std::chrono::minutes time_elapsed) const {
        return bytes_sent >= max_bytes || time_elapsed.count() >= max_minutes;
    }
};

bool ShouldRotateKemKey() {
    fs::path path = gArgs.GetDataDirNet() / "pqnoise" / "kem.sk";
    if (!fs::exists(path)) {
        return true; // No key exists, need to generate
    }
    
    auto mod_time = fs::last_write_time(path);
    auto now = std::chrono::file_clock::now();
    // Rotate daily (24 hours)
    return (now - mod_time) > std::chrono::hours(24);
}

void ForceKemKeyRotation() {
    fs::path dir = gArgs.GetDataDirNet() / "pqnoise";
    fs::path current_path = dir / "kem.sk";
    fs::path prev_path = dir / "kem.sk.prev";
    
    // Move current to previous
    if (fs::exists(current_path)) {
        fs::rename(current_path, prev_path);
    }
    
    // Generate new key
    auto new_keys = qtc_kyber::KeyGen1024();
    StoreKyberKey(new_keys);
}

/// Maximum witness length for Bech32 addresses.
static constexpr std::size_t BECH32_WITNESS_PROG_MAX_LEN = 40;

namespace {
class DestinationEncoder
{
private:
    const CChainParams& m_params;

public:
    explicit DestinationEncoder(const CChainParams& params) : m_params(params) {}

    std::string operator()(const PKHash& id) const
    {
        std::vector<unsigned char> data = m_params.Base58Prefix(CChainParams::PUBKEY_ADDRESS);
        data.insert(data.end(), id.begin(), id.end());
        return EncodeBase58Check(data);
    }

    std::string operator()(const ScriptHash& id) const
    {
        std::vector<unsigned char> data = m_params.Base58Prefix(CChainParams::SCRIPT_ADDRESS);
        data.insert(data.end(), id.begin(), id.end());
        return EncodeBase58Check(data);
    }

    std::string operator()(const WitnessV0KeyHash& id) const
    {
        std::vector<unsigned char> data = {1}; // QTC PQ addresses use witness v1 (bech32m)
        data.reserve(33);
        ConvertBits<8, 5, true>([&](unsigned char c) { data.push_back(c); }, id.begin(), id.end());
        return bech32::Encode(bech32::Encoding::BECH32, m_params.Bech32HRP(), data);
    }

    std::string operator()(const WitnessV0ScriptHash& id) const
    {
        std::vector<unsigned char> data = {1}; // QTC PQ addresses use witness v1 (bech32m)
        data.reserve(53);
        ConvertBits<8, 5, true>([&](unsigned char c) { data.push_back(c); }, id.begin(), id.end());
        return bech32::Encode(bech32::Encoding::BECH32, m_params.Bech32HRP(), data);
    }

    std::string operator()(const WitnessV1Taproot& tap) const
    {
        std::vector<unsigned char> data = {1};
        data.reserve(53);
        ConvertBits<8, 5, true>([&](unsigned char c) { data.push_back(c); }, tap.begin(), tap.end());
        return bech32::Encode(bech32::Encoding::BECH32M, m_params.Bech32HRP(), data);
    }

    std::string operator()(const WitnessUnknown& id) const
    {
        const std::vector<unsigned char>& program = id.GetWitnessProgram();
        if (id.GetWitnessVersion() < 1 || id.GetWitnessVersion() > 16 || program.size() < 2 || program.size() > 40) {
            return {};
        }
        std::vector<unsigned char> data = {(unsigned char)id.GetWitnessVersion()};
        data.reserve(1 + (program.size() * 8 + 4) / 5);
        ConvertBits<8, 5, true>([&](unsigned char c) { data.push_back(c); }, program.begin(), program.end());
        return bech32::Encode(bech32::Encoding::BECH32M, m_params.Bech32HRP(), data);
    }

    // QTC Quantum-Safe Address Encoding
// NOTE: Derive the 20-byte program from SHA3-256(pubkey)[0:20] upstream and encode here
// using bech32m (witness v1).
    std::string operator()(const QKeyHash& id) const
    {
        // QTC uses "qtc" prefix with bech32 for quantum-safe addresses
        std::vector<unsigned char> data = {1}; // QTC PQ addresses use witness v1 (bech32m)
        data.reserve(53); // Reserve space for 32-byte hash + overhead
        ConvertBits<8, 5, true>([&](unsigned char c) { data.push_back(c); }, id.begin(), id.end());
        return bech32::Encode(bech32::Encoding::BECH32M, "qtc", data);
    }

    std::string operator()(const QScriptHash& id) const
    {
        // QTC quantum script hash with "qtc" prefix
        std::vector<unsigned char> data = {1}; // Version 1 for quantum script hash
        data.reserve(53);
        ConvertBits<8, 5, true>([&](unsigned char c) { data.push_back(c); }, id.begin(), id.end());
        return bech32::Encode(bech32::Encoding::BECH32M, "qtc", data);
    }

    std::string operator()(const WitnessV2QKeyHash& id) const
    {
        // QTC witness v2 quantum key hash
        std::vector<unsigned char> data = {2};
        data.reserve(53);
        ConvertBits<8, 5, true>([&](unsigned char c) { data.push_back(c); }, id.begin(), id.end());
        return bech32::Encode(bech32::Encoding::BECH32M, "qtc", data);
    }

    std::string operator()(const CNoDestination& no) const { return {}; }
    std::string operator()(const PubKeyDestination& pk) const { return {}; }
};

CTxDestination DecodeDestination(const std::string& str, const CChainParams& params, std::string& error_str, std::vector<int>* error_locations)
{
    std::vector<unsigned char> data;
    uint160 hash;
    error_str = "";

    // Note this will be false if it is a valid Bech32 address for a different network
    bool is_bech32 = (ToLower(str.substr(0, params.Bech32HRP().size())) == params.Bech32HRP());

    if (!is_bech32 && DecodeBase58Check(str, data, 21)) {
        // base58-encoded Quantum Coin addresses.
        // Public-key-hash-addresses have version 0 (or 111 testnet).
        // The data vector contains RIPEMD160(SHA256(pubkey)), where pubkey is the serialized public key.
        const std::vector<unsigned char>& pubkey_prefix = params.Base58Prefix(CChainParams::PUBKEY_ADDRESS);
        if (data.size() == hash.size() + pubkey_prefix.size() && std::equal(pubkey_prefix.begin(), pubkey_prefix.end(), data.begin())) {
            std::copy(data.begin() + pubkey_prefix.size(), data.end(), hash.begin());
            return PKHash(hash);
        }
        // Script-hash-addresses have version 5 (or 196 testnet).
        // The data vector contains RIPEMD160(SHA256(cscript)), where cscript is the serialized redemption script.
        const std::vector<unsigned char>& script_prefix = params.Base58Prefix(CChainParams::SCRIPT_ADDRESS);
        if (data.size() == hash.size() + script_prefix.size() && std::equal(script_prefix.begin(), script_prefix.end(), data.begin())) {
            std::copy(data.begin() + script_prefix.size(), data.end(), hash.begin());
            return ScriptHash(hash);
        }

        // If the prefix of data matches either the script or pubkey prefix, the length must have been wrong
        if ((data.size() >= script_prefix.size() &&
                std::equal(script_prefix.begin(), script_prefix.end(), data.begin())) ||
            (data.size() >= pubkey_prefix.size() &&
                std::equal(pubkey_prefix.begin(), pubkey_prefix.end(), data.begin()))) {
            error_str = "Invalid length for Base58 address (P2PKH or P2SH)";
        } else {
            error_str = "Invalid or unsupported Base58-encoded address.";
        }
        return CNoDestination();
    } else if (!is_bech32) {
        // Try Base58 decoding without the checksum, using a much larger max length
        if (!DecodeBase58(str, data, 100)) {
            error_str = "Invalid or unsupported Segwit (Bech32) or Base58 encoding.";
        } else {
            error_str = "Invalid checksum or length of Base58 address (P2PKH or P2SH)";
        }
        return CNoDestination();
    }

    data.clear();
    const auto dec = bech32::Decode(str);
    if (dec.encoding == bech32::Encoding::BECH32 || dec.encoding == bech32::Encoding::BECH32M) {
        if (dec.data.empty()) {
            error_str = "Empty Bech32 data section";
            return CNoDestination();
        }
        // Bech32 decoding
        if (dec.hrp != params.Bech32HRP()) {
            error_str = strprintf("Invalid or unsupported prefix for Segwit (Bech32) address (expected %s, got %s).", params.Bech32HRP(), dec.hrp);
            return CNoDestination();
        }
        int version = dec.data[0]; // The first 5 bit symbol is the witness version (0-16)
        if (version == 0 && dec.encoding != bech32::Encoding::BECH32) {
            error_str = "Version 0 witness address must use Bech32 checksum";
            return CNoDestination();
        }
        if (version != 0 && dec.encoding != bech32::Encoding::BECH32M) {
            error_str = "Version 1+ witness address must use Bech32m checksum";
            return CNoDestination();
        }
        // The rest of the symbols are converted witness program bytes.
        data.reserve(((dec.data.size() - 1) * 5) / 8);
        if (ConvertBits<5, 8, false>([&](unsigned char c) { data.push_back(c); }, dec.data.begin() + 1, dec.data.end())) {

            std::string_view byte_str{data.size() == 1 ? "byte" : "bytes"};

            if (version == 0) {
                {
                    WitnessV0KeyHash keyid;
                    if (data.size() == keyid.size()) {
                        std::copy(data.begin(), data.end(), keyid.begin());
                        return keyid;
                    }
                }
                {
                    WitnessV0ScriptHash scriptid;
                    if (data.size() == scriptid.size()) {
                        std::copy(data.begin(), data.end(), scriptid.begin());
                        return scriptid;
                    }
                }

                error_str = strprintf("Invalid Bech32 v0 address program size (%d %s), per BIP141", data.size(), byte_str);
                return CNoDestination();
            }

            if (version == 1 && data.size() == WITNESS_V1_TAPROOT_SIZE) {
                static_assert(WITNESS_V1_TAPROOT_SIZE == WitnessV1Taproot::size());
                WitnessV1Taproot tap;
                std::copy(data.begin(), data.end(), tap.begin());
                return tap;
            }

            // QTC Quantum-Safe Address Decoding
            if (dec.hrp == "qtc") {
                if (version == 1 && data.size() == 20) {
                    // QTC quantum key hash (qtc1...)
                    QKeyHash qkeyhash;
                    std::copy(data.begin(), data.end(), qkeyhash.begin());
                    return qkeyhash;
                }
                if (version == 1 && data.size() == 32) {
                    // QTC quantum script hash
                    QScriptHash qscripthash;
                    std::copy(data.begin(), data.end(), qscripthash.begin());
                    return qscripthash;
                }
                if (version == 2 && data.size() == 32) {
                    // QTC witness v2 quantum key hash
                    WitnessV2QKeyHash wqkeyhash;
                    std::copy(data.begin(), data.end(), wqkeyhash.begin());
                    return wqkeyhash;
                }
            }

            if (CScript::IsPayToAnchor(version, data)) {
                return PayToAnchor();
            }

            if (version > 16) {
                error_str = "Invalid Bech32 address witness version";
                return CNoDestination();
            }

            if (data.size() < 2 || data.size() > BECH32_WITNESS_PROG_MAX_LEN) {
                error_str = strprintf("Invalid Bech32 address program size (%d %s)", data.size(), byte_str);
                return CNoDestination();
            }

            return WitnessUnknown{version, data};
        } else {
            error_str = strprintf("Invalid padding in Bech32 data section");
            return CNoDestination();
        }
    }

    // Perform Bech32 error location
    auto res = bech32::LocateErrors(str);
    error_str = res.first;
    if (error_locations) *error_locations = std::move(res.second);
    return CNoDestination();
}
} // namespace

CKey DecodeSecret(const std::string& str)
{
    CKey key;
    std::vector<unsigned char> data;
    if (DecodeBase58Check(str, data, 34)) {
        const std::vector<unsigned char>& privkey_prefix = Params().Base58Prefix(CChainParams::SECRET_KEY);
        if ((data.size() == 32 + privkey_prefix.size() || (data.size() == 33 + privkey_prefix.size() && data.back() == 1)) &&
            std::equal(privkey_prefix.begin(), privkey_prefix.end(), data.begin())) {
            bool compressed = data.size() == 33 + privkey_prefix.size();
            key.Set(data.begin() + privkey_prefix.size(), data.begin() + privkey_prefix.size() + 32, compressed);
        }
    }
    if (!data.empty()) {
        memory_cleanse(data.data(), data.size());
    }
    return key;
}

std::string EncodeSecret(const CKey& key)
{
    assert(key.IsValid());
    std::vector<unsigned char> data = Params().Base58Prefix(CChainParams::SECRET_KEY);
    data.insert(data.end(), UCharCast(key.begin()), UCharCast(key.end()));
    if (key.IsCompressed()) {
        data.push_back(1);
    }
    std::string ret = EncodeBase58Check(data);
    memory_cleanse(data.data(), data.size());
    return ret;
}

CExtPubKey DecodeExtPubKey(const std::string& str)
{
    CExtPubKey key;
    std::vector<unsigned char> data;
    if (DecodeBase58Check(str, data, 78)) {
        const std::vector<unsigned char>& prefix = Params().Base58Prefix(CChainParams::EXT_PUBLIC_KEY);
        if (data.size() == BIP32_EXTKEY_SIZE + prefix.size() && std::equal(prefix.begin(), prefix.end(), data.begin())) {
            key.Decode(data.data() + prefix.size());
        }
    }
    return key;
}

std::string EncodeExtPubKey(const CExtPubKey& key)
{
    std::vector<unsigned char> data = Params().Base58Prefix(CChainParams::EXT_PUBLIC_KEY);
    size_t size = data.size();
    data.resize(size + BIP32_EXTKEY_SIZE);
    key.Encode(data.data() + size);
    std::string ret = EncodeBase58Check(data);
    return ret;
}

CExtKey DecodeExtKey(const std::string& str)
{
    CExtKey key;
    std::vector<unsigned char> data;
    if (DecodeBase58Check(str, data, 78)) {
        const std::vector<unsigned char>& prefix = Params().Base58Prefix(CChainParams::EXT_SECRET_KEY);
        if (data.size() == BIP32_EXTKEY_SIZE + prefix.size() && std::equal(prefix.begin(), prefix.end(), data.begin())) {
            key.Decode(data.data() + prefix.size());
        }
    }
    if (!data.empty()) {
        memory_cleanse(data.data(), data.size());
    }
    return key;
}

std::string EncodeExtKey(const CExtKey& key)
{
    std::vector<unsigned char> data = Params().Base58Prefix(CChainParams::EXT_SECRET_KEY);
    size_t size = data.size();
    data.resize(size + BIP32_EXTKEY_SIZE);
    key.Encode(data.data() + size);
    std::string ret = EncodeBase58Check(data);
    memory_cleanse(data.data(), data.size());
    return ret;
}

std::string EncodeDestination(const CTxDestination& dest)
{
    return std::visit(DestinationEncoder(Params()), dest);
}

CTxDestination DecodeDestination(const std::string& str, std::string& error_msg, std::vector<int>* error_locations)
{
    return DecodeDestination(str, Params(), error_msg, error_locations);
}

CTxDestination DecodeDestination(const std::string& str)
{
    std::string error_msg;
    return DecodeDestination(str, error_msg);
}

bool IsValidDestinationString(const std::string& str, const CChainParams& params)
{
    std::string error_msg;
    return IsValidDestination(DecodeDestination(str, params, error_msg, nullptr));
}

bool IsValidDestinationString(const std::string& str)
{
    return IsValidDestinationString(str, Params());
}
