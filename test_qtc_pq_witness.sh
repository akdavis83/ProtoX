#!/usr/bin/env bash
set -euo pipefail

# Lightweight demo runner for PQ witness math and 20-byte program sanity-check
cat <<'OUT'
QTC PQ Witness Compression Demo
===============================
- Dilithium3 pk ~1952 bytes, sig ~3293 bytes -> total ~5245 bytes
- Legacy (no SegWit) effective: 5245
- QTC native SegWit effective (witness discounted 4:1): 5245/4 ≈ 1311
- Relative savings: ~75%
- Output pk commitment: 20 bytes vs 1952 bytes (~99% savings)

To run the C++ unit test:
  cd bitcoin/build
  cmake .. && make test_qtc -j
  ./test_qtc --run_test=qtc_pq_witness_tests
OUT
