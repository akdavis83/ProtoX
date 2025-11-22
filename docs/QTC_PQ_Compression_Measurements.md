# QTC PQ Compression Measurements (Transaction-Level)

This document records measured sizes for a post-quantum (PQC) witness spend using QTC native SegWit.

Test method:
- Funding tx: P2WPKH-like PQ output with 20-byte program derived from SHA3-256(pubkey)[0:20].
- Spending tx: Witness stack includes Dilithium signature (~3293 bytes) and Dilithium pk (~1952 bytes).
- We serialize base-only and with-witness, compute sizes and weight:
  - base_size, witness_size, total_size, weight = base*4 + witness
  - legacy_effective = base + witness (no discount)
  - qtc_effective = weight/4

Expected:
- Spend-side effective savings ~ 75% vs legacy baseline for the PQ payload
- Output-side pk commitment savings ~ 99% (20 bytes vs 1952 bytes)

How to reproduce:
1) Build and run unit test `qtc_pq_tx_measure_tests.cpp`:

```
cd bitcoin/build
cmake .. && make test_qtc -j
./test_qtc --run_test=qtc_pq_tx_measure_tests
```

2) The test prints:
- Base size
- Witness size
- Total size
- Weight
- Legacy effective vs QTC effective

3) Optional blueprint integration test:
- See `contrib/tests/qtc_pq_witness_integration.py` for RPC-based scaffolding.

Notes:
- The 20-byte program is derived with SHA3-256(pubkey) truncated to 20 bytes for addresses only; all other SHA3-512 uses remain unchanged.
- Ensure bech32m (qtc) and correct witness version are used per address policy.
