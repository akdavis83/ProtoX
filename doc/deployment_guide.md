# QTC Deployment Guide

## Overview
This guide covers deploying QTC nodes in various environments, from development to production, with focus on post-quantum transport configuration.

## Production Deployment

### Minimum System Requirements
- **CPU**: 4 cores (8 recommended)
- **RAM**: 8GB (16GB recommended)
- **Storage**: 500GB SSD (1TB recommended)
- **Network**: 100Mbps dedicated bandwidth
- **OS**: Linux (Ubuntu 20.04+ recommended)

### Security Hardening

#### 1. PQ-Only Configuration
```bash
# /etc/qtc/qtc.conf
pq-only=1
pq-kem-gen=1
allowlegacy=0

# Network security
bind=127.0.0.1
rpcallowip=127.0.0.1
rpcbind=127.0.0.1

# Performance tuning
maxconnections=125
maxuploadtarget=5000
```

#### 2. Firewall Configuration
```bash
# UFW setup
sudo ufw allow 8333/tcp   # P2P port
sudo ufw allow 8332/tcp   # RPC port (if needed)
sudo ufw enable

# iptables alternative
iptables -A INPUT -p tcp --dport 8333 -j ACCEPT
iptables -A INPUT -p tcp --dport 8332 -s 127.0.0.1 -j ACCEPT
```

#### 3. System Service Setup
```bash
# Create systemd service
sudo tee /etc/systemd/system/qtcd.service << EOF
[Unit]
Description=QTC Daemon
After=network.target

[Service]
Type=forking
User=qtc
Group=qtc
ExecStart=/usr/local/bin/qtcd -daemon -conf=/etc/qtc/qtc.conf -datadir=/var/lib/qtc
ExecStop=/usr/local/bin/qtc-cli stop
Restart=always
RestartSec=60

[Install]
WantedBy=multi-user.target
