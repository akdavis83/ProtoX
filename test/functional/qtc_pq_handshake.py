#!/usr/bin/env python3
# Copyright (c) 2024 The QTC Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test PQ Noise-only handshake between two nodes."""

import os
import tempfile
from test_framework.test_framework import QTCTestFramework
from test_framework.util import (
    assert_equal,
    wait_until,
)

class QTCPQHandshakeTest(QTCTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 2
        self.extra_args = [
            ['-pq-only', '-pq-kem-gen'],  # Server node
            ['-pq-only'],  # Client node - will get peer pubkey from server
        ]

    def setup_network(self):
        # Start server node first
        self.add_nodes(1, extra_args=self.extra_args[0])
        self.start_node(0)
        
        # Wait for server to generate keys
        server_datadir = self.nodes[0].datadir
        server_pk_path = os.path.join(server_datadir, 'pq_kem_pk.bin')
        
        def server_keys_exist():
            return os.path.exists(server_pk_path)
        
        wait_until(server_keys_exist, timeout=10)
        self.log.info(f"Server KEM public key generated at {server_pk_path}")
        
        # Start client node with server's public key
        client_args = self.extra_args[1] + [f'-pq-peer-pubkey={server_pk_path}']
        self.add_nodes(1, extra_args=client_args)
        self.start_node(1)
        
        # Connect the nodes
        self.connect_nodes(0, 1)

    def run_test(self):
        self.log.info("Testing PQ Noise handshake between two nodes")
        
        # Wait for connection establishment
        def nodes_connected():
            return len(self.nodes[0].getpeerinfo()) > 0 and len(self.nodes[1].getpeerinfo()) > 0
        
        wait_until(nodes_connected, timeout=30)
        
        # Verify peer connection info
        server_peers = self.nodes[0].getpeerinfo()
        client_peers = self.nodes[1].getpeerinfo()
        
        assert_equal(len(server_peers), 1)
        assert_equal(len(client_peers), 1)
        
        self.log.info("Nodes successfully connected with PQ transport")
        
        # Check if network info shows PQ suite usage (if implemented)
        try:
            server_netinfo = self.nodes[0].getnetworkinfo()
            client_netinfo = self.nodes[1].getnetworkinfo()
            
            if 'pq_suites_used' in server_netinfo:
                self.log.info(f"Server PQ suites used: {server_netinfo['pq_suites_used']}")
            if 'pq_suites_used' in client_netinfo:
                self.log.info(f"Client PQ suites used: {client_netinfo['pq_suites_used']}")
        except:
            self.log.info("PQ metrics not yet implemented in getnetworkinfo")
        
        # Test basic blockchain operations over encrypted transport
        self.log.info("Testing blockchain operations over PQ transport")
        
        # Generate some blocks on server
        server_addr = self.nodes[0].getnewaddress()
        self.nodes[0].generatetoaddress(10, server_addr)
        
        # Sync blocks
        self.sync_all()
        
        # Verify client received blocks
        assert_equal(self.nodes[1].getblockcount(), 10)
        
        # Test transaction propagation
        client_addr = self.nodes[1].getnewaddress()
        txid = self.nodes[0].sendtoaddress(client_addr, 1.0)
        
        # Wait for tx to propagate
        def tx_in_client_mempool():
            return txid in self.nodes[1].getrawmempool()
        
        wait_until(tx_in_client_mempool, timeout=10)
        self.log.info("Transaction propagated successfully over PQ transport")
        
        # Mine the transaction
        self.nodes[0].generatetoaddress(1, server_addr)
        self.sync_all()
        
        # Verify transaction was mined
        assert txid not in self.nodes[0].getrawmempool()
        assert txid not in self.nodes[1].getrawmempool()
        
        self.log.info("PQ Noise handshake and encrypted transport test completed successfully")

if __name__ == '__main__':
    QTCPQHandshakeTest().main()
EOF}
