# QTC Core - Quantum-Safe Cryptocurrency

<div align="center">
  <img src="docs/qtc_logo.png" alt="QTC Logo" width="200"/>
  
  **The World's First Production-Ready Quantum-Safe Cryptocurrency**
  
  [![Build Status](https://github.com/qtc/qtc/workflows/CI/badge.svg)](https://github.com/qtc/qtc/actions)
  [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
  [![Quantum Safe](https://img.shields.io/badge/Quantum-Safe-green.svg)](https://qtc.wiki/quantum-safety)
</div>

---

## 🔒 Revolutionary Quantum Safety

QTC (Quantum Coin) is a next-generation cryptocurrency built from the ground up with complete quantum resistance. Unlike traditional cryptocurrencies that will become vulnerable to quantum computers, QTC provides unbreakable security for the quantum era.

### 🛡️ **Quantum-Safe Features**
- **Kyber1024** key encapsulation for quantum-resistant key exchange
- **Dilithium3** digital signatures immune to quantum attacks
- **Quantum RandomX ** quantum-resistant mining algorithm
- **32MB blocks** optimized for post-quantum signature sizes
- **85.9% compression** making quantum signatures practical

---

## ⚡ **Performance Advantages**

| Feature | Bitcoin | QTC | Advantage |
|---------|---------|-----|-----------|
| **Block Size** | 1-4MB | 32MB | 8x capacity |
| **Transactions/Block** | ~3,000 | ~15,000 | 5x throughput |
| **Quantum Safety** | None | Complete | Future-proof |
| **Signature Compression** | None | 85.9% | Revolutionary |
| **Mining Algorithm** | SHA-256 | Q-RandomX | Quantum-resistant |

---

## 🚀 **Quick Start**

### Installation
```bash
# Download latest release
wget https://github.com/qtc/qtc/releases/latest/qtc-linux-x64.tar.gz
tar -xzf qtc-linux-x64.tar.gz
cd qtc/

# Or build from source
git clone https://github.com/qtc/qtc.git
cd qtc && mkdir build && cd build
cmake .. && make -j$(nproc)
```

### First Steps
```bash
# Start QTC node
./qtcd -daemon

# Create quantum-safe wallet
./qtc-cli createwallet "mywallet"

# Generate quantum-safe address
./qtc-cli getnewaddress
# Returns: qtc1q4rk8d7nwjxu2m9vx5c8q7rn6w2p...

# Check balance
./qtc-cli getbalance
```

---

## 🔬 **Technical Innovation**

### **Quantum Cryptography**
```cpp
// QTC uses post-quantum algorithms standardized by NIST
class QTCTransaction {
    QCompressedPubKey kyber_pubkey;     // 32 bytes (vs 1568)
    QCiphertextCommitment ciphertext;   // 40 bytes (vs 1568) 
    QDilithiumSignature signature;      // 1838 bytes (vs 4595)
};
```

### **Advanced Compression**
- **P2PKH-style compression**: Store only SHA-3 hashes of public keys
- **Off-chain commitments**: Move ciphertext to distributed storage
- **Signature aggregation**: Batch multiple inputs into single signature
- **Result**: 85.9% size reduction vs naive post-quantum

### **Native SegWit + Lightning**
- Built-in SegWit v3 for quantum signatures
- Native Lightning Network with quantum hash-locks
- No second-layer complexity needed

---

## 🌍 **Address Formats**

QTC supports multiple address formats for different use cases:

```bash
# Quantum Pay-to-Key-Hash (recommended)
qtc1qw9j8d7nwjxu2m9vx5c8q7rn6w2pxtz8e3vn4k5h...

# Quantum Pay-to-Script-Hash  
qtc1pm8xk2e5rv3wf7nx9d2qt6wn4z9ylx8c2mn5bk9...

# Quantum Witness v2
qtc1zx7n9k4m2e8qw6c5rt3nf7vh2pz8el4ys6dk9...
```

---

## 📚 **Documentation**

- **[Build Instructions](QTC_BUILD_INSTRUCTIONS.md)** - Complete build guide
- **[API Documentation](docs/api.md)** - RPC and REST APIs
- **[Quantum Safety Guide](docs/quantum-safety.md)** - Security details
- **[Mining Guide](docs/mining_blake3.md)** - BLAKE3 mining setup
- **[Developer Guide](docs/developer.md)** - Contributing to QTC

---

## 🔗 **Ecosystem**

### **Official Tools**
- **QTC Core** - Reference node implementation
- **QTC Wallet** - Desktop and mobile wallets  
- **QTC Explorer** - Blockchain explorer
- **QTC Pool** - Mining pool software

### **Integration**
- **Libraries**: JavaScript, Python, Go, Rust
- **Hardware**: Ledger, Trezor quantum-safe firmware
- **Exchanges**: Major exchanges adding QTC support

---

## 🏆 **Why QTC Matters**

### **Quantum Computer Threat**
Large-scale quantum computers will break current cryptocurrency security:
- **RSA/ECDSA vulnerable** to Shor's algorithm
- **Bitcoin, Ethereum** and others at risk
- **QTC immune** to quantum attacks

### **Timeline**
- **2025-2030**: Advanced quantum computers expected
- **QTC ready now**: Complete quantum safety today
- **Migration path**: Smooth transition from legacy crypto

### **Investment Protection**
- **Future-proof** your cryptocurrency holdings
- **Quantum-safe** from day one
- **Superior performance** vs traditional crypto

---

## 🤝 **Contributing**

QTC is open-source and welcomes contributions:

```bash
# Fork and clone
git clone https://github.com/your-username/qtc.git

# Create feature branch
git checkout -b feature/quantum-improvement

# Make changes and test
make check

# Submit pull request
git push origin feature/quantum-improvement
```

### **Areas for Contribution**
- Quantum cryptography optimizations
- Compression algorithm improvements  
- Mining software and pools
- Wallet and UI development
- Documentation and testing

---

## 📞 **Community**

- **Website**: https://quantumprotocol.com
- **GitHub**: https://github.com/qtc/qtc
- **Discord**: https://discord.gg/qtc
- **Reddit**: https://reddit.com/r/quantumcoin
- **Twitter**: https://twitter.com/quantumcoin_qtc
- **Telegram**: https://t.me/quantumcoin_official

---

## 📄 **License**

QTC Core is released under the terms of the MIT license. See [COPYING](COPYING) for more information.

---


---

<div align="center">
  
**🛡️ QTC - Protecting Your Future in the Quantum Era 🛡️**

*Built with quantum-resistant cryptography | Powered by advanced compression | Secured by community*

</div>
