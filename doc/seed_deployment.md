# QTC Seed Node Deployment Guide

## Overview
This guide covers deploying QTC seed nodes for network bootstrapping, including DNS setup, server configuration, and monitoring.

## Seed Node Architecture

### Purpose of Seed Nodes
- **Peer Discovery**: Help new nodes find initial peers
- **Network Bootstrap**: Ensure network connectivity during low-activity periods
- **PQ Transport Advocacy**: Promote post-quantum transport adoption
- **Reliability**: Provide stable, always-online network endpoints

### QTC Seed Domains
```
seed1.qtc.org    → Primary seed node
seed2.qtc.org    → Secondary seed node  
seed3.qtc.org    → Tertiary seed node
```

## DNS Configuration

### Domain Requirements
You **do NOT need websites** for seed domains. These are network endpoints only.

Your main website **qtcprotocol.com** remains separate for:
- Project documentation
- Download links
- Community resources
- Marketing content

### DNS Records Setup
```dns
; Zone file for qtc.org
$TTL 300
@       IN  SOA ns1.qtc.org. admin.qtc.org. (
            2024010101  ; serial
            3600        ; refresh
            1800        ; retry  
            604800      ; expire
            300         ; minimum TTL
            )

@       IN  NS  ns1.qtc.org.
@       IN  NS  ns2.qtc.org.

; Seed node records
seed1   IN  A   198.51.100.10
seed2   IN  A   203.0.113.20
seed3   IN  A   192.0.2.30

; IPv6 support (optional)
seed1   IN  AAAA  2001:db8:1::10
seed2   IN  AAAA  2001:db8:2::20  
seed3   IN  AAAA  2001:db8:3::30
```

### DNS Provider Configuration

#### Cloudflare Example
```bash
# Add A records via Cloudflare API
curl -X POST "https://api.cloudflare.com/client/v4/zones/ZONE_ID/dns_records" \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  --data '{
    "type": "A",
    "name": "seed1",
    "content": "198.51.100.10",
    "ttl": 300
  }'
```

#### Route 53 Example
```bash
# Add records via AWS CLI
aws route53 change-resource-record-sets --hosted-zone-id ZONE_ID --change-batch '{
  "Changes": [{
    "Action": "CREATE",
    "ResourceRecordSet": {
      "Name": "seed1.qtc.org",
      "Type": "A", 
      "TTL": 300,
      "ResourceRecords": [{"Value": "198.51.100.10"}]
    }
  }]
}'
```

## Server Configuration

### Hardware Requirements
- **CPU**: 4 cores minimum (8 recommended)
- **RAM**: 8GB minimum (16GB recommended)  
- **Storage**: 1TB SSD (seeds handle high connection volume)
- **Network**: 1Gbps dedicated bandwidth
- **Uptime**: 99.9% target (seeds must be highly available)

### Seed Node Configuration
```bash
# /etc/qtc/seed.conf
## Basic Configuration
regtest=0
testnet=0
daemon=1
server=1

## Network Configuration  
listen=1
discover=1
dnsseed=1
port=8333

## PQ Transport (Production Ready)
pq-only=1
pq-kem-gen=1
pq-rekey-bytes=33554432
pq-rekey-time=30

## High Capacity Settings
maxconnections=1000
maxuploadtarget=0
timeout=30000

## Resource Management
dbcache=4000
blockmaxsize=32000000

## Logging & Monitoring
debug=pqnoise
debug=net
logips=1
logtimestamps=1

## Security
rpcbind=127.0.0.1
rpcallowip=127.0.0.1
```

### System Service Setup
```bash
# Create qtc user
sudo useradd -r -s /bin/false qtc
sudo mkdir -p /var/lib/qtc /var/log/qtc /etc/qtc
sudo chown qtc:qtc /var/lib/qtc /var/log/qtc
sudo chmod 750 /var/lib/qtc /var/log/qtc

# Install systemd service
sudo tee /etc/systemd/system/qtc-seed.service << EOF
[Unit]
Description=QTC Seed Node
Documentation=https://docs.qtcprotocol.com/
After=network.target
Wants=network.target

[Service]
Type=notify
User=qtc
Group=qtc

ExecStart=/usr/local/bin/qtcd \\
  -conf=/etc/qtc/seed.conf \\
  -datadir=/var/lib/qtc \\
  -debuglogfile=/var/log/qtc/debug.log

# Process management
Restart=on-failure
RestartSec=30
TimeoutStopSec=120

# Security settings
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=full
ProtectHome=true
ReadWritePaths=/var/lib/qtc /var/log/qtc

# Resource limits
LimitNOFILE=65536
MemoryMax=16G

[Install]
WantedBy=multi-user.target
