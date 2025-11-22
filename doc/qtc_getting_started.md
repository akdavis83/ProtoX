# QTC Getting Started Guide

## Overview
QTC (Quantum Coin) is the world's first production-ready post-quantum cryptocurrency, providing unbreakable security against both classical and quantum computer attacks.

## Quick Installation

### Download Binary (Recommended)
```bash
# Linux
wget https://github.com/qtc/qtc/releases/latest/qtc-linux-x64.tar.gz
tar -xzf qtc-linux-x64.tar.gz
cd qtc/

# Start QTC node
./bin/qtcd -daemon

# Check status
./bin/qtc-cli getinfo
```

### Build from Source
```bash
git clone https://github.com/qtc/qtc.git
cd qtc/

# Install dependencies
sudo apt update
sudo apt install build-essential cmake libtool autotools-dev automake pkg-config bsdmainutils python3 libevent-dev libboost-dev

# Build with PQ support
make -j$(nproc) CONFIG_SITE=$PWD/depends/x86_64-pc-linux-gnu/share/config.site
```

## First Steps

### 1. Create Wallet
```bash
qtc-cli createwallet "my_qtc_wallet"
```

### 2. Generate Address
```bash
qtc-cli getnewaddress
```

### 3. Check PQ Transport Status
```bash
qtc-cli getnetworkinfo
# Look for "pqnoise" section in output
```

## PQ Transport Configuration

### Enable PQ-Only Mode (Recommended for Production)
```bash
qtcd -pq-only=1 -daemon
```

### Allow Classical Fallback (Development Only)
```bash
qtcd -pq-only=1 -allowlegacy=1 -daemon
```

### Custom Rekey Settings
```bash
qtcd -pq-rekey-bytes=16777216 -pq-rekey-time=15 -daemon
```

## Network Information

### Mainnet
- **Default Port**: 8333
- **RPC Port**: 8332
- **PQ Transport**: Enabled by default

### Testnet
- **Default Port**: 18333
- **RPC Port**: 18332
- **Start Command**: `qtcd -testnet -daemon`

### Regtest (Development)
- **Default Port**: 18444
- **RPC Port**: 18443
- **Start Command**: `qtcd -regtest -daemon`

## Troubleshooting

### Common Issues
1. **Port already in use**: Change ports with `-port=` and `-rpcport=`
2. **PQ handshake failures**: Check firewall and network connectivity
3. **Memory issues**: Reduce `-maxconnections=` for low-memory systems

### Enable Debug Logging
```bash
qtcd -debug=pqnoise -debug=net -daemon
tail -f ~/.qtc/debug.log
```

## Next Steps
- Read [PQ Transport Documentation](pqnoise_transport.md)
- Explore [API Documentation](api_reference.md)
- Set up [Development Environment](development_setup.md)
- Join the community at [qtcprotocol.com](https://qtcprotocol.com)
