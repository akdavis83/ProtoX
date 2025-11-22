// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-present The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef QTC_CONSENSUS_CONSENSUS_H
#define QTC_CONSENSUS_CONSENSUS_H

#include <cstdint>
#include <cstdlib>

/** The maximum allowed size for a serialized block, in bytes (only for buffer size limits) */
static const unsigned int MAX_BLOCK_SERIALIZED_SIZE = 33554432; // 32MB for quantum-safe transactions
/** The maximum allowed weight for a block, see BIP 141 (network rule) */
static const unsigned int MAX_BLOCK_WEIGHT = 33554432; // 32MB weight limit for QTC
/** The maximum allowed number of signature check operations in a block (network rule) */
static const int64_t MAX_BLOCK_SIGOPS_COST = 320000; // 4x increase for 32MB blocks

// QTC Block Validation Performance Targets
static const int64_t QTC_TARGET_BLOCK_PROPAGATION_SECONDS = 5;  // Target 5 seconds for 32MB block propagation
static const int64_t QTC_TARGET_TRANSACTIONS_PER_BLOCK = 15000; // Target ~15,000 compressed transactions per 32MB block
static const double QTC_COMPRESSION_EFFICIENCY_TARGET = 0.85;   // Target 85% compression efficiency

// QTC Quantum-Safe Signature Limits
static const int64_t MAX_QTC_SIGNATURE_SIZE = 6000;     // Max Dilithium5 signature size
static const int64_t MIN_QTC_SIGNATURE_SIZE = 1000;     // Min compressed signature size
static const int64_t MAX_QTC_AGGREGATED_INPUTS = 50;    // Max inputs per aggregated signature

// QTC Compression Constants and Ratios
static const int64_t QTC_COMPRESSION_TARGET_RATIO = 85; // Target 85% compression vs naive PQ
static const int64_t QTC_PUBKEY_COMPRESSION_RATIO = 98; // Pubkey: 1568 → 32 bytes (98%)
static const int64_t QTC_CIPHERTEXT_COMPRESSION_RATIO = 97; // Ciphertext: 1568 → 40 bytes (97%)
static const int64_t QTC_SIGNATURE_COMPRESSION_RATIO = 60;  // Signature: 4595 → 1838 bytes (60%)

// QTC Key and Signature Size Validation
static const size_t QTC_COMPRESSED_PUBKEY_SIZE = 64;    // SHA3-512 hash of full pubkey
static const size_t QTC_CIPHERTEXT_COMMITMENT_SIZE = 40; // Hash + index + nonce
static const size_t QTC_MAX_AGGREGATION_OVERHEAD = 200; // Max overhead per aggregated input

// QTC Performance Limits
static const int64_t QTC_MAX_SIGNATURE_VERIFY_TIME_MS = 5;    // Max 5ms per signature verification
static const int64_t QTC_MAX_BLOCK_VERIFY_TIME_SECONDS = 30;  // Max 30 seconds for 32MB block
static const int64_t QTC_TARGET_COMPRESSION_SPEED_MBS = 100;  // Target 100MB/s compression speed
/** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
static const int COINBASE_MATURITY = 100;

static const int WITNESS_SCALE_FACTOR = 4;

static const size_t MIN_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 60; // 60 is the lower bound for the size of a valid serialized CTransaction
static const size_t MIN_SERIALIZABLE_TRANSACTION_WEIGHT = WITNESS_SCALE_FACTOR * 10; // 10 is the lower bound for the size of a serialized CTransaction

/** Flags for nSequence and nLockTime locks */
/** Interpret sequence numbers as relative lock-time constraints. */
static constexpr unsigned int LOCKTIME_VERIFY_SEQUENCE = (1 << 0);

/**
 * Maximum number of seconds that the timestamp of the first
 * block of a difficulty adjustment period is allowed to
 * be earlier than the last block of the previous period (BIP94).
 */
static constexpr int64_t MAX_TIMEWARP = 600;

#endif // QTC_CONSENSUS_CONSENSUS_H
