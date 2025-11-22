#!/usr/bin/env python3
"""
QTC PQ-Enabled Network Seeder
Lightweight Python crawler for QTC post-quantum network discovery
"""

import socket
import struct
import time
import json
import threading
import argparse
from typing import List, Dict, Set
import logging

# QTC PQ Noise Protocol Constants
QTC_MAGIC_BYTES = b'\xF9\xBE\xB4\xD9'
QTC_VERSION = 1
DEFAULT_PORT = 8333
PQ_HANDSHAKE_TIMEOUT = 30

class QTCPQSeeder:
    def __init__(self, output_file: str = "qtc_peers.json", max_peers: int = 1000):
        self.known_peers: Set[str] = set()
        self.good_peers: List[Dict] = []
        self.output_file = output_file
        self.max_peers = max_peers
        self.lock = threading.Lock()
        
        # Setup logging
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(levelname)s - %(message)s'
        )
        self.logger = logging.getLogger(__name__)
        
    def add_seed_nodes(self):
        """Add initial seed nodes to crawl"""
        seeds = [
            "seed1.qtc.org",
            "seed2.qtc.org", 
            "seed3.qtc.org",
            # Add hardcoded fallbacks
            "127.0.0.1",  # localhost for testing
        ]
        
        for seed in seeds:
            self.known_peers.add(f"{seed}:{DEFAULT_PORT}")
            
    def create_pq_handshake(self, server_pk: bytes = None) -> bytes:
        """Create QTC PQ ClientHello message"""
        if server_pk is None:
            # Generate dummy ciphertext for testing (1568 bytes for Kyber1024)
            server_pk = b'\x00' * 1568
            
        msg = bytearray()
        msg.extend(QTC_MAGIC_BYTES)  # Magic
        msg.extend(struct.pack('<H', QTC_VERSION))  # Version
        msg.extend(struct.pack('<H', len(server_pk)))  # Ciphertext length
        msg.extend(server_pk)  # Kyber1024 ciphertext
        msg.extend(b'\x00' * 32)  # Random padding
        
        return bytes(msg)
        
    def test_pq_handshake(self, host: str, port: int) -> bool:
        """Test PQ handshake with a peer"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(PQ_HANDSHAKE_TIMEOUT)
            
            # Connect to peer
            sock.connect((host, port))
            
            # Send PQ ClientHello
            handshake_msg = self.create_pq_handshake()
            sock.send(handshake_msg)
            
            # Wait for ServerHello response
            response = sock.recv(4096)
            
            if len(response) >= 7:
                # Check magic bytes and status
                if response[:4] == QTC_MAGIC_BYTES and response[6] == 0:
                    self.logger.info(f"✓ PQ handshake successful with {host}:{port}")
                    return True
                    
            sock.close()
            return False
            
        except Exception as e:
            self.logger.debug(f"✗ PQ handshake failed with {host}:{port}: {e}")
            return False
            
    def crawl_peer(self, peer_addr: str):
        """Crawl a single peer for PQ capability"""
        try:
            host, port = peer_addr.rsplit(':', 1)
            port = int(port)
            
            # Test PQ handshake
            if self.test_pq_handshake(host, port):
                with self.lock:
                    peer_info = {
                        "addr": peer_addr,
                        "host": host,
                        "port": port,
                        "pq_capable": True,
                        "last_seen": int(time.time()),
                        "success_count": 1
                    }
                    self.good_peers.append(peer_info)
                    
                    if len(self.good_peers) % 10 == 0:
                        self.logger.info(f"Found {len(self.good_peers)} PQ-capable peers")
                        
        except Exception as e:
            self.logger.error(f"Error crawling {peer_addr}: {e}")
            
    def crawl_network(self, threads: int = 50):
        """Crawl the network for PQ-capable peers"""
        self.logger.info("Starting QTC PQ network crawl...")
        self.add_seed_nodes()
        
        # Create thread pool for crawling
        import concurrent.futures
        
        with concurrent.futures.ThreadPoolExecutor(max_workers=threads) as executor:
            # Submit crawl tasks
            futures = []
            for peer in list(self.known_peers):
                if len(self.good_peers) >= self.max_peers:
                    break
                future = executor.submit(self.crawl_peer, peer)
                futures.append(future)
                
            # Wait for completion
            for future in concurrent.futures.as_completed(futures):
                future.result()
                
        self.logger.info(f"Crawl completed. Found {len(self.good_peers)} PQ-capable peers")
        
    def save_peers(self):
        """Save discovered peers to JSON file"""
        output = {
            "timestamp": int(time.time()),
            "total_peers": len(self.good_peers),
            "peers": self.good_peers
        }
        
        with open(self.output_file, 'w') as f:
            json.dump(output, f, indent=2)
            
        self.logger.info(f"Saved {len(self.good_peers)} peers to {self.output_file}")
        
    def run_continuous(self, interval: int = 3600):
        """Run seeder continuously"""
        while True:
            self.crawl_network()
            self.save_peers()
            self.logger.info(f"Sleeping for {interval} seconds...")
            time.sleep(interval)

def main():
    parser = argparse.ArgumentParser(description='QTC PQ Network Seeder')
    parser.add_argument('--output', '-o', default='qtc_peers.json',
                       help='Output file for peer list')
    parser.add_argument('--max-peers', '-m', type=int, default=1000,
                       help='Maximum peers to discover')
    parser.add_argument('--threads', '-t', type=int, default=50,
                       help='Number of crawling threads')
    parser.add_argument('--continuous', '-c', action='store_true',
                       help='Run continuously')
    parser.add_argument('--interval', '-i', type=int, default=3600,
                       help='Interval between crawls (seconds)')
    
    args = parser.parse_args()
    
    seeder = QTCPQSeeder(args.output, args.max_peers)
    
    if args.continuous:
        seeder.run_continuous(args.interval)
    else:
        seeder.crawl_network(args.threads)
        seeder.save_peers()

if __name__ == "__main__":
    main()
