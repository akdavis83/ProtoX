# QTC Codebase Audit: Final Report

## 1. Executive Summary

The QTC project is in a pre-alpha state. While it claims to be a "quantum safe blockchain solution," the implementation does not support this claim. Core components of the system are either critically insecure, incomplete, or directly contradict the project's documentation. Deploying the system in its current state would result in a total loss of funds and a complete compromise of the network.

## 2. Critical Vulnerabilities & Issues

### A. Man-in-the-Middle (MitM) Vulnerability in P2P Network
*   **Finding:** The post-quantum P2P protocol (`src/net/pq_noise.cpp`) was critically flawed. It used the Kyber1024 algorithm for key exchange but completely omitted the use of Dilithium3 for server authentication. This allowed any attacker to intercept P2P connections, impersonate nodes, and read or modify all network traffic.
*   **Status:** A patch has been applied to implement proper signature generation and verification, but the original state of this code demonstrates a fundamental misunderstanding of secure protocol design.

### B. Non-Functional and Flawed Mining Algorithm
*   **Finding:** The consensus-critical `CheckProofOfWork` function in `src/pow.cpp` is broken. It calls the `Mine()` function instead of the `Verify()` function, meaning every node would have to re-mine every block to verify it. This would make the blockchain unusably slow and unstable.
*   **Finding:** The `QTCQuantumRandomX` mining algorithm itself is a non-functional placeholder. The implementation file (`src/crypto/qtc_quantum_randomx.cpp`) is filled with comments like `// In production, this would be...` and uses simplified stubs instead of the actual RandomX and Cuckoo Cycle algorithms. It provides none of the claimed ASIC resistance.
*   **Finding:** The "quantum" aspect of the mining algorithm is security theater. It uses the Kyber algorithm in a convoluted way to generate a seed, but the implementation provides no actual quantum security benefit. The private key is never used in a way that would secure the system from a quantum or classical attacker.

### C. Dangerously Misleading Documentation
*   **Finding:** The project's documentation is contradictory and false. `QTC_MINING_ALGORITHM_FINAL.md` describes a detailed Kyber+Argon2+BLAKE3 algorithm that is **not implemented anywhere in the consensus code.** The whitepaper's claim of "proprietary signature compression" also appears to be false, as the P2P handshake did not even use signatures. This level of discrepancy indicates a chaotic development process and an untrustworthy codebase.

### D. Insecure Build System
*   **Finding:** The `CMakeLists.txt` file uses `FetchContent` to download the `liboqs` cryptography library but fails to pin the download with a `URL_HASH`. This is a critical vulnerability that would allow an attacker to inject malicious code into the build process.

## 3. Hashing Algorithm Audit

*   **Finding:** The claim that "All SHA has been changed to SHA3-512" is **false**. The codebase is still heavily reliant on legacy Bitcoin hashing algorithms for consensus-critical functions:
    *   **SHA-256** and **RIPEMD-160** are still used for standard v0 witness addresses (P2WPKH/P2WSH) and legacy address types.
    *   **SHA-1** is still implemented and available via the `OP_SHA1` opcode, which is a known weak algorithm.
*   **Finding:** `SHA3-256` is used, but only for the new, custom `WitnessV2QKeyHash` address type. It generates a 32-byte hash, not a 20-byte hash as some documentation comments suggest.

---

### **Checklist for Mainnet Readiness**

The following is a non-exhaustive list of the most critical issues that MUST be addressed before this project can be considered for mainnet deployment.

**🔴 Critical Priority**
*   `[ ]` **Fix Consensus Logic:** The `CheckProofOfWork` function in `src/pow.cpp` must be rewritten to call the `Verify()` function, not `Mine()`.
*   `[ ]` **Implement a Functional Mining Algorithm:** The `QTCQuantumRandomX` algorithm must be fully implemented. The placeholder code in `qtc_quantum_randomx.cpp` needs to be replaced with a production-ready and tested implementation of RandomX and Cuckoo Cycle.
*   `[ ]` **Secure the Build System:** Add a `URL_HASH` to the `FetchContent_Declare` call for `liboqs` in `CMakeLists.txt` to prevent supply chain attacks.
*   `[ ]` **Review and Correct All Documentation:** All project documentation, especially the whitepaper and mining algorithm specifications, must be rewritten to accurately reflect the code's actual implementation. All false and misleading claims must be removed.
*   `[ ]` **Remove or Redesign "Quantum" Seeding:** The current use of Kyber in the mining algorithm is ineffective. It should be either removed or completely redesigned by a cryptographic expert to provide a meaningful security benefit.

**🟡 High Priority**
*   `[ ]` **Disable `OP_SHA1`:** The `OP_SHA1` opcode in `src/script/interpreter.cpp` should be disabled to remove a weak hashing algorithm from the consensus rules.
*   `[ ]` **Complete the Transition to `SHA3` (If Desired):** If the goal is to fully transition away from legacy hashes, this requires a hard fork and a fundamental redesign of the addressing and transaction-signing systems. This is a massive undertaking and must be carefully planned and executed. The current state is a mix of old and new, which is confusing and potentially insecure.
*   `[ ]` **Peer Review by Cryptographers:** Given the number of fundamental cryptographic errors, the entire codebase must be reviewed by qualified cryptographers before it can be trusted.

This project requires a significant and expert-led engineering effort to remedy its flaws.