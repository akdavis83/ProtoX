# QTC API Reference

## Overview
QTC extends Bitcoin Core's RPC interface with post-quantum specific functionality. All standard Bitcoin RPC methods are supported, plus additional PQ transport commands.

## Extended RPC Methods

### getnetworkinfo
Enhanced to include PQ transport statistics when PQ mode is enabled.

**Returns:**
```json
{
  "version": 250000,
  "subversion": "/QTC Core:25.0.0/",
  "protocolversion": 70016,
  "localservices": "0000000000001409",
  "localservicesnames": [
    "NETWORK",
    "WITNESS", 
    "PQNOISE"
  ],
  "pqnoise": {
    "handshakes_attempted": 1247,
    "handshakes_successful": 1198,
    "handshakes_failed": 49,
    "bytes_encrypted": 45231872,
    "bytes_decrypted": 44982156,
    "rekeys_performed": 12,
    "sessions_active": 8,
    "suite": "NoisePQ_KYBER1024_DILITHIUM3_SHA3-512_CHACHA20-POLY1305"
  }
}
```

### getpqstatus
**Description:** Returns detailed PQ transport status and configuration.

**Parameters:** None

**Returns:**
```json
{
  "enabled": true,
  "pq_only_mode": true,
  "allow_legacy": false,
  "current_suite": "NoisePQ_KYBER1024_DILITHIUM3_SHA3-512_CHACHA20-POLY1305",
  "kem_algorithm": "Kyber1024",
  "signature_algorithm": "Dilithium3",
  "key_rotation": {
    "kem_keys_last_rotated": "2024-01-15T10:30:00Z",
    "kem_keys_next_rotation": "2024-01-16T10:30:00Z",
    "identity_key_age_days": 45,
    "identity_key_rotation_days": 320
  },
  "rekey_policy": {
    "bytes_threshold": 33554432,
    "time_threshold_minutes": 30
  },
  "metrics": {
    "total_sessions": 156,
    "active_sessions": 8,
    "handshake_success_rate": 0.961
  }
}
```

### rotatepqkeys
**Description:** Manually trigger PQ key rotation.

**Parameters:**
- `key_type` (string): "kem", "identity", or "all"
- `force` (boolean, optional): Force rotation even if not due

**Example:**
```bash
qtc-cli rotatepqkeys "kem" true
```

**Returns:**
```json
{
  "result": "success",
  "rotated": ["kem"],
  "next_rotation": {
    "kem": "2024-01-16T10:30:00Z",
    "identity": "2024-12-15T10:30:00Z"
  }
}
```

### getpqpeers
**Description:** List connected peers using PQ transport.

**Parameters:** None

**Returns:**
```json
[
  {
    "id": 1,
    "addr": "192.168.1.100:8333",
    "services": "0000000000001409",
    "pq_transport": true,
    "pq_suite": "NoisePQ_KYBER1024_DILITHIUM3_SHA3-512_CHACHA20-POLY1305",
    "bytes_sent_encrypted": 1024000,
    "bytes_recv_encrypted": 987000,
    "last_rekey": "2024-01-15T15:45:00Z",
    "connection_time": "2024-01-15T10:30:00Z"
  }
]
```

### testpqhandshake
**Description:** Test PQ handshake with a specific peer (debug only).

**Parameters:**
- `address` (string): IP address or hostname of peer
- `port` (integer, optional): Port number (default: 8333)

**Example:**
```bash
qtc-cli testpqhandshake "192.168.1.100" 8333
```

**Returns:**
```json
{
  "success": true,
  "handshake_time_ms": 45,
  "suite_negotiated": "NoisePQ_KYBER1024_DILITHIUM3_SHA3-512_CHACHA20-POLY1305",
  "peer_public_key": "0x1234567890abcdef...",
  "shared_secret_established": true
}
```

## Configuration Parameters

### Command Line Arguments

#### PQ Transport
- `-pq-only=<0|1>` - Enable PQ-only mode (default: 0)
- `-allowlegacy=<0|1>` - Allow classical transport fallback (default: 0)
- `-pq-kem-gen=<0|1>` - Generate new KEM keys on startup (default: 0)

#### Key Rotation
- `-pq-rekey-bytes=<n>` - Trigger rekey after n bytes (default: 33554432)
- `-pq-rekey-time=<n>` - Trigger rekey after n minutes (default: 30)

#### Debug Options
- `-debug=pqnoise` - Enable PQ transport debug logging
- `-debug=pqmetrics` - Enable PQ metrics debug logging

### Configuration File (qtc.conf)

```ini
# PQ Transport Configuration
pq-only=1
pq-kem-gen=1
pq-rekey-bytes=16777216
pq-rekey-time=15

# Network Configuration
port=8333
rpcport=8332
maxconnections=125

# Debug Options
debug=pqnoise
debug=net
```

## Error Codes

### PQ-Specific Error Codes
- `-32001` - PQ handshake failed
- `-32002` - PQ key generation failed  
- `-32003` - PQ transport not enabled
- `-32004` - Invalid PQ configuration
- `-32005` - PQ peer not found
- `-32006` - Key rotation in progress

## Usage Examples

### Check if PQ transport is working
```bash
qtc-cli getnetworkinfo | jq '.pqnoise'
```

### Monitor PQ handshake activity
```bash
watch -n 5 'qtc-cli getpqstatus | jq .metrics'
```

### Force key rotation
```bash
qtc-cli rotatepqkeys "all" true
```

### List PQ-enabled peers
```bash
qtc-cli getpqpeers | jq '.[].addr'
```

### Test connection to specific peer
```bash
qtc-cli testpqhandshake "seed1.qtc.org"
```
