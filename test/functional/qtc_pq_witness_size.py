#!/usr/bin/env python3
from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal
from test_framework.address import program_to_witness
from test_framework.script import CScript, OP_1, OP_RETURN
from test_framework.messages import CTransaction, CTxIn, CTxOut, COutPoint, ToHex
from test_framework.blocktools import COIN
import hashlib, json, time, pathlib

class QTCPQWitnessSizeTest(BitcoinTestFramework):
    def set_test_params(self):
        self.num_nodes = 1
        self.setup_clean_chain = True

    def sha3_256_20(self, data: bytes) -> bytes:
        return hashlib.sha3_256(data).digest()[:20]

    def run_test(self):
        node = self.nodes[0]
        node.generate(101)

        # 1) Program20 from Dilithium pk (dummy 1952 bytes)
        dil_pk = bytes([0x42]) * 1952
        program20 = self.sha3_256_20(dil_pk)

        # 2) Create v1 witness output (OP_1 <20-byte program>), bech32m address
        addr = program_to_witness(1, program20)
        self.log.info(f"PQ bech32m address: {addr}")

        # 3) Fund address
        txid = node.sendtoaddress(addr, 0.5)
        node.generate(1)
        self.log.info(f"Funded UTXO: {txid}")

        # 4) Build spend skeleton and measure
        utxo = [u for u in node.listunspent(0, 9999999, [addr])][0]
        spending = CTransaction()
        spending.vin = [CTxIn(COutPoint(int(utxo["txid"],16), utxo["vout"]), b", 0xffffffff)]
        spending.vout = [CTxOut(int(utxo["amount"]*COIN) - 10_000, CScript([OP_RETURN, b"PQ"]))]

        hex_base = ToHex(spending)
        dec_base = node.decoderawtransaction(hex_base)
        base_size = dec_base["size"]
        txid_no_witness = dec_base["txid"]

        # Mock PQ witness sizes
        mock_sig = bytes([0x99]) * 3293
        mock_pk  = dil_pk
        witness_size = len(mock_sig) + len(mock_pk)

        qtc_weight = base_size * 4 + witness_size
        legacy_effective = base_size + witness_size
        qtc_effective = qtc_weight // 4

        artifacts_dir = pathlib.Path(self.options.tmpdir) / "artifacts"
        artifacts_dir.mkdir(parents=True, exist_ok=True)
        js = {
            "base_size": base_size,
            "witness_size": witness_size,
            "weight": qtc_weight,
            "legacy_effective": legacy_effective,
            "qtc_effective": qtc_effective,
            "program_bytes": 20,
            "pq_sig_bytes": len(mock_sig),
            "pq_pk_bytes": len(mock_pk),
            "program_method": "SHA3_256_trunc",
            "txid_no_witness": txid_no_witness,
            "note": "Witness not attached (PQC signing pending); metrics combine serialization + theoretical witness."
        }
        out = artifacts_dir / f"qtc_pq_witness_{int(time.time())}.json"
        out.write_text(json.dumps(js, indent=2))
        self.log.info(f"Archived: {out}")

        assert qtc_effective < legacy_effective

if __name__ == '__main__':
    QTCPQWitnessSizeTest().main()
