// Copyright (c) 2024 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <atomic>
#include <string>

/** PQ Noise transport metrics and logging */
class PQMetrics
{
public:
    // Counters
    static std::atomic<uint64_t> handshakes_attempted;
    static std::atomic<uint64_t> handshakes_successful;
    static std::atomic<uint64_t> handshakes_failed;
    static std::atomic<uint64_t> bytes_encrypted;
    static std::atomic<uint64_t> bytes_decrypted;
    static std::atomic<uint64_t> rekeys_performed;
    static std::atomic<uint64_t> sessions_active;

    // Suite tracking
    static std::atomic<uint64_t> kyber1024_handshakes;
    static std::atomic<uint64_t> chacha20poly1305_sessions;

    static void RecordHandshakeAttempt();
    static void RecordHandshakeSuccess();
    static void RecordHandshakeFailure(const std::string& reason);
    static void RecordBytesEncrypted(size_t bytes);
    static void RecordBytesDecrypted(size_t bytes);
    static void RecordRekey();
    static void RecordSessionStart();
    static void RecordSessionEnd();

    // Get current metrics as key-value pairs
    static std::map<std::string, uint64_t> GetMetrics();
    
    // Reset all counters (for testing)
    static void Reset();
};

// Logging helpers
void LogPQHandshakeStage(const std::string& stage, const std::string& details = );
void LogPQError(const std::string& error, const std::string& context = );
void LogPQSuite(const std::string& suite);
EOF}
