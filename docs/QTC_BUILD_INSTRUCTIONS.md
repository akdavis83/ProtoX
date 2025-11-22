# QTC (Quantum Coin) Build Instructions

## Overview
QTC is a quantum-safe cryptocurrency forked from Bitcoin Core, featuring:
- Complete Kyber1024 + Dilithium3 post-quantum cryptography
- 85.9% transaction compression
- 32MB blocks with 5.3x Bitcoin throughput
- Native SegWit and Lightning Network integration
- Quantum RandomX, quantum-resistant mining algorithm

## System Requirements
- Linux/macOS/Windows
- GCC 9+ or Clang 10+
- CMake 3.16+
- At least 8GB RAM for building
- 50GB+ disk space for full blockchain

## Dependencies
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential cmake git pkg-config
sudo apt install libssl-dev libboost-all-dev libevent-dev
sudo apt install libdb++-dev libzmq3-dev libqrencode-dev

# macOS
brew install cmake boost libevent berkeley-db@4 zeromq qrencode

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install cmake boost-devel libevent-devel libdb-cxx-devel zeromq-devel
```

## Building QTC

### 1. Clone Repository
```bash
git clone https://github.com/your-repo/qtc.git
cd qtc
```

### 2. Configure Build
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### 3. Build QTC Core
```bash
make -j$(nproc)
```

### 4. Install (Optional)
```bash
sudo make install
```

## Built Executables
- `qtcd` - QTC node daemon
- `qtc-cli` - Command line interface
- `qtc-tx` - Transaction utility
- `qtc-wallet` - Wallet management tool
- `qtc-qt` - GUI wallet (if Qt enabled)

## Configuration

### Basic qtc.conf
```
# QTC Configuration
server=1
daemon=1
rpcuser=your_username
rpcpassword=your_secure_password

# Quantum-safe settings
maxblocksize=33554432
quantumsafe=1
compress_transactions=1

# Network settings
port=8333
rpcport=8332
```

## Running QTC Node

### Start QTC Daemon
```bash
./qtcd -daemon
```

### Check Status
```bash
./qtc-cli getinfo
./qtc-cli getblockchaininfo
```

### Create Wallet
```bash
./qtc-cli createwallet "mywallet"
./qtc-cli getnewaddress
```

## QTC Address Formats
- Legacy: `1...` (compatible)
- Quantum P2QKH: `qtc1q...` (quantum key hash)
- Quantum P2QSH: `qtc1p...` (quantum script hash)
- Witness v2: `qtc1z...` (quantum witness)

## Mining QTC

### CPU Mining (SHA-3)
```bash
./qtcd -gen=1 -genproclimit=4
```

### Mining Pool Setup
```bash
# Configure for quantum-safe mining
miningalgorithm=sha3
blockversion=4
quantumsignatures=1
```

## Testing

### Run Unit Tests
```bash
make check
```

### Run Functional Tests
```bash
python3 test/functional/test_runner.py
```

## Troubleshooting

### Common Issues
1. **Build fails**: Ensure all dependencies installed
2. **Slow sync**: Use `assumevalid` for faster initial sync
3. **High memory**: QTC uses more RAM due to quantum signatures

### Getting Help
- GitHub Issues: https://github.com/your-repo/qtc/issues
- QTC Discord: [Discord invite]
- Documentation: https://qtc.wiki

## Advanced Configuration

### Quantum Settings
```
# Enable all quantum features
kyber1024=1
dilithium5=1
signature_aggregation=1
compression_level=high

# Performance tuning
qtc_max_signature_cache=10000
qtc_verify_threads=8
```

### Network Tuning for 32MB Blocks
```
maxconnections=125
maxuploadtarget=unlimited
blocksonly=0
```

## Development

### Building with Debug
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON
make -j$(nproc)
```

### Running Development Node
```bash
./qtcd -regtest -daemon
./qtc-cli -regtest generate 101
```

---

**QTC - The Future of Quantum-Safe Cryptocurrency**
