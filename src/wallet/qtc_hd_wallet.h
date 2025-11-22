// QTC HD Wallet - Core Implementation
// Hierarchical Deterministic Quantum Wallet for QTC Core

#ifndef QTC_WALLET_HD_WALLET_H
#define QTC_WALLET_HD_WALLET_H

#include <qtc_wallet.h>
#include <uint256.h>
#include <serialize.h>
#include <map>
#include <string>

/** QTC HD Wallet Key Derivation Paths */
static constexpr uint32_t QTC_HD_COIN_TYPE = 999;      // QTC coin type
static constexpr uint32_t QTC_HD_PURPOSE = 44;         // BIP 44 compatible
static constexpr uint32_t QTC_HD_ACCOUNT = 0;          // Default account
static constexpr uint32_t QTC_HD_EXTERNAL_CHAIN = 0;   // External addresses
static constexpr uint32_t QTC_HD_INTERNAL_CHAIN = 1;   // Change addresses

namespace qtc_wallet {

/** HD Chain State */
struct HDChain {
    uint32_t nExternalChainCounter{0};
    uint32_t nInternalChainCounter{0};
    uint256 seed_id;
    
    SERIALIZE_METHODS(HDChain, obj) {
        READWRITE(obj.nExternalChainCounter);
        READWRITE(obj.nInternalChainCounter);
        READWRITE(obj.seed_id);
    }
};

/** QTC HD Master Key */
class QTCHDMasterKey {
private:
    std::vector<uint8_t> vchSeed;
    std::string sMnemonic;
    
public:
    QTCHDMasterKey() = default;
    
    /** Generate new master key from entropy */
    void GenerateNewKey();
    
    /** Restore master key from mnemonic */
    bool RestoreFromMnemonic(const std::string& mnemonic);
    
    /** Derive child key at specific path */
    qtc::QuantumWallet DeriveKey(uint32_t purpose, uint32_t coin_type, 
                                uint32_t account, uint32_t chain, uint32_t index) const;
    
    /** Get mnemonic phrase for backup */
    std::string GetMnemonic() const { return sMnemonic; }
    
    /** Check if wallet is HD */
    bool IsHDEnabled() const { return !vchSeed.empty(); }
    
    SERIALIZE_METHODS(QTCHDMasterKey, obj) {
        READWRITE(obj.vchSeed);
        READWRITE(obj.sMnemonic);
    }

private:
    /** Convert mnemonic to seed */
    std::vector<uint8_t> MnemonicToSeed(const std::string& mnemonic) const;
    
    /** Generate mnemonic from entropy */
    std::string GenerateMnemonic(const std::vector<uint8_t>& entropy) const;
};

/** QTC HD Wallet */
class QTCHDWallet {
private:
    QTCHDMasterKey master_key;
    HDChain hd_chain;
    std::map<uint32_t, qtc::QuantumWallet> m_addresses;
    std::map<std::string, std::string> m_labels;

public:
    QTCHDWallet() = default;
    
    /** Initialize new HD wallet */
    bool InitializeHD();
    
    /** Restore HD wallet from mnemonic */
    bool RestoreHD(const std::string& mnemonic);
    
    /** Generate new receiving address */
    std::string GetNewAddress(const std::string& label = "");
    
    /** Generate new change address */
    std::string GetNewChangeAddress();
    
    /** Get all addresses in wallet */
    std::vector<std::string> GetAddresses() const;
    
    /** Get address label */
    std::string GetAddressLabel(const std::string& address) const;
    
    /** Set address label */
    void SetAddressLabel(const std::string& address, const std::string& label);
    
    /** Get quantum wallet for address */
    qtc::QuantumWallet GetQuantumWallet(const std::string& address) const;
    
    /** Check if address belongs to this wallet */
    bool IsAddressMine(const std::string& address) const;
    
    /** Get mnemonic for backup */
    std::string GetMnemonic() const { return master_key.GetMnemonic(); }
    
    /** Check if wallet is HD enabled */
    bool IsHDEnabled() const { return master_key.IsHDEnabled(); }
    
    /** Get HD chain info */
    const HDChain& GetHDChain() const { return hd_chain; }

private:
    /** Generate address at specific index */
    std::string GenerateAddress(uint32_t chain, uint32_t index, const std::string& label = "");
    
    /** Derive key path string */
    std::string GetKeyPath(uint32_t chain, uint32_t index) const;
};

/** BIP 39 Mnemonic utilities */
namespace mnemonic {
    /** Generate random mnemonic */
    std::string GenerateMnemonic(size_t strength = 128);
    
    /** Validate mnemonic phrase */
    bool ValidateMnemonic(const std::string& mnemonic);
    
    /** Convert mnemonic to seed */
    std::vector<uint8_t> MnemonicToSeed(const std::string& mnemonic, const std::string& passphrase = "");
    
    /** Get word list */
    const std::vector<std::string>& GetWordList();
}

} // namespace qtc_wallet

#endif // QTC_WALLET_HD_WALLET_H
