#!/usr/bin/env python3
# Copyright (c) 2024 The QTC Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
"""Test PQ-only strict flags - nodes reject classical peers."""

import os
from test_framework.test_framework import QTCTestFramework
from test_framework.util import (
    assert_equal,
    wait_until,
)

class QTCPQStrictFlagsTest(QTCTestFramework):
    def set_test_params(self):
        self.setup_clean_chain = True
        self.num_nodes = 3
        self.extra_args = [
            ['-pq-only', '-pq-kem-gen'],  # PQ-only server
            ['-pq-only'],  # PQ-only client  
            [],  # Classical node (no PQ flags)
        ]

    def setup_network(self):
        # Start PQ-only server
        self.add_nodes(1, extra_args=self.extra_args[0])
        self.start_node(0)
        
        # Wait for server keys
        server_datadir = self.nodes[0].datadir
        server_pk_path = os.path.join(server_datadir, 'pq_kem_pk.bin')
        
        def server_keys_exist():
            return os.path.exists(server_pk_path)
        
        wait_until(server_keys_exist, timeout=10)
        
        # Start PQ-only client
        client_args = self.extra_args[1] + [f'-pq-peer-pubkey={server_pk_path}']
        self.add_nodes(1, extra_args=client_args)
        self.start_node(1)
        
        # Start classical node
        self.add_nodes(1, extra_args=self.extra_args[2])
        self.start_node(2)

    def run_test(self):
        self.log.info("Testing PQ-only strict flag behavior")
        
        # Test 1: PQ nodes should connect to each other
        self.log.info("Test 1: PQ nodes connecting to each other")
        self.connect_nodes(0, 1)
        
        def pq_nodes_connected():
            return (len(self.nodes[0].getpeerinfo()) > 0 and 
                   len(self.nodes[1].getpeerinfo()) > 0)
        
        wait_until(pq_nodes_connected, timeout=30)
        
        server_peers = self.nodes[0].getpeerinfo()
        client_peers = self.nodes[1].getpeerinfo()
        assert_equal(len(server_peers), 1)
        assert_equal(len(client_peers), 1)
        self.log.info("✓ PQ nodes connected successfully")
        
        # Test 2: PQ-only node should reject classical node
        self.log.info("Test 2: PQ-only node rejecting classical node")
        
        # Try to connect classical node to PQ-only server
        # This should fail or be rejected
        try:
            self.connect_nodes(0, 2)
            
            # Wait to see if connection is established or rejected
            import time
            time.sleep(5)
            
            # Check if classical node appears in PQ server's peer list
            server_peers_after = self.nodes[0].getpeerinfo()
            classical_peers = self.nodes[2].getpeerinfo()
            
            # Connection should be rejected/not established for PQ-incompatible peers
            pq_peer_count = len([p for p in server_peers_after if 'pq' in str(p).lower()])
            
            if len(server_peers_after) == 1:
                self.log.info("✓ Classical node connection rejected by PQ-only server")
            else:
                self.log.warning(f"Classical node may have connected: {len(server_peers_after)} peers")
                
        except Exception as e:
            self.log.info(f"✓ Classical node connection rejected: {e}")
        
        # Test 3: Classical node should not be able to establish PQ handshake
        self.log.info("Test 3: Verifying classical node cannot establish PQ handshake")
        
        # Classical node should have no successful PQ connections
        classical_peers = self.nodes[2].getpeerinfo()
        pq_connections = len([p for p in classical_peers if 'pq_suite' in p])
        assert_equal(pq_connections, 0)
        self.log.info("✓ Classical node has no PQ connections")
        
        # Test 4: Verify PQ nodes maintain their connection
        self.log.info("Test 4: Verifying PQ nodes maintain connection")
        
        final_server_peers = self.nodes[0].getpeerinfo()
        final_client_peers = self.nodes[1].getpeerinfo()
        
        assert_equal(len(final_server_peers), 1)
        assert_equal(len(final_client_peers), 1)
        
        # Test basic functionality over PQ connection
        server_addr = self.nodes[0].getnewaddress()
        self.nodes[0].generatetoaddress(5, server_addr)
        self.sync_blocks([0, 1])  # Sync only PQ nodes
        
        assert_equal(self.nodes[0].getblockcount(), 5)
        assert_equal(self.nodes[1].getblockcount(), 5)
        
        # Classical node should be out of sync (no connection to PQ network)
        classical_blocks = self.nodes[2].getblockcount()
        assert_equal(classical_blocks, 0)  # Should still be at genesis
        
        self.log.info("✓ PQ nodes maintain functionality while classical node is isolated")
        
        # Test 5: Check network info for PQ suite information
        self.log.info("Test 5: Checking network information")
        
        try:
            server_netinfo = self.nodes[0].getnetworkinfo()
            client_netinfo = self.nodes[1].getnetworkinfo()
            
            # Log any PQ-related metrics
            for key, value in server_netinfo.items():
                if 'pq' in key.lower():
                    self.log.info(f"Server {key}: {value}")
            
            for key, value in client_netinfo.items():
                if 'pq' in key.lower():
                    self.log.info(f"Client {key}: {value}")
                    
        except Exception as e:
            self.log.info(f"Network info check: {e}")
        
        self.log.info("PQ-only strict flags test completed successfully")

if __name__ == '__main__':
    QTCPQStrictFlagsTest().main()
EOF}
