// Copyright (c) 2024 The QTC Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <net/pq_metrics.h>
#include <logging.h>
#include <map>

// Static counter definitions
std::atomic<uint64_t> PQMetrics::handshakes_attempted{0};
std::atomic<uint64_t> PQMetrics::handshakes_successful{0};
std::atomic<uint64_t> PQMetrics::handshakes_failed{0};
std::atomic<uint64_t> PQMetrics::bytes_encrypted{0};
std::atomic<uint64_t> PQMetrics::bytes_decrypted{0};
std::atomic<uint64_t> PQMetrics::rekeys_performed{0};
std::atomic<uint64_t> PQMetrics::sessions_active{0};
std::atomic<uint64_t> PQMetrics::kyber1024_handshakes{0};
std::atomic<uint64_t> PQMetrics::chacha20poly1305_sessions{0};

void PQMetrics::RecordHandshakeAttempt()
{
    handshakes_attempted.fetch_add(1, std::memory_order_relaxed);
    LogPrint(BCLog::NET, "PQ: Handshake attempt recorded (total: %d)\n", 
             handshakes_attempted.load());
}

void PQMetrics::RecordHandshakeSuccess()
{
    handshakes_successful.fetch_add(1, std::memory_order_relaxed);
    kyber1024_handshakes.fetch_add(1, std::memory_order_relaxed);
    chacha20poly1305_sessions.fetch_add(1, std::memory_order_relaxed);
    LogPrint(BCLog::NET, "PQ: Handshake successful (total: %d)\n", 
             handshakes_successful.load());
}

void PQMetrics::RecordHandshakeFailure(const std::string& reason)
{
    handshakes_failed.fetch_add(1, std::memory_order_relaxed);
    LogPrint(BCLog::NET, "PQ: Handshake failed - %s (total failures: %d)\n", 
             reason, handshakes_failed.load());
}

void PQMetrics::RecordBytesEncrypted(size_t bytes)
{
    bytes_encrypted.fetch_add(bytes, std::memory_order_relaxed);
    LogPrint(BCLog::NET, "PQ: Encrypted %d bytes (total: %d)\n", 
             bytes, bytes_encrypted.load());
}

void PQMetrics::RecordBytesDecrypted(size_t bytes)
{
    bytes_decrypted.fetch_add(bytes, std::memory_order_relaxed);
    LogPrint(BCLog::NET, "PQ: Decrypted %d bytes (total: %d)\n", 
             bytes, bytes_decrypted.load());
}

void PQMetrics::RecordRekey()
{
    rekeys_performed.fetch_add(1, std::memory_order_relaxed);
    LogPrint(BCLog::NET, "PQ: Rekey performed (total: %d)\n", 
             rekeys_performed.load());
}

void PQMetrics::RecordSessionStart()
{
    sessions_active.fetch_add(1, std::memory_order_relaxed);
    LogPrint(BCLog::NET, "PQ: Session started (active: %d)\n", 
             sessions_active.load());
}

void PQMetrics::RecordSessionEnd()
{
    if (sessions_active.load() > 0) {
        sessions_active.fetch_sub(1, std::memory_order_relaxed);
    }
    LogPrint(BCLog::NET, "PQ: Session ended (active: %d)\n", 
             sessions_active.load());
}

std::map<std::string, uint64_t> PQMetrics::GetMetrics()
{
    return {
        {"handshakes_attempted", handshakes_attempted.load()},
        {"handshakes_successful", handshakes_successful.load()},
        {"handshakes_failed", handshakes_failed.load()},
        {"bytes_encrypted", bytes_encrypted.load()},
        {"bytes_decrypted", bytes_decrypted.load()},
        {"rekeys_performed", rekeys_performed.load()},
        {"sessions_active", sessions_active.load()},
        {"kyber1024_handshakes", kyber1024_handshakes.load()},
        {"chacha20poly1305_sessions", chacha20poly1305_sessions.load()},
    };
}

void PQMetrics::Reset()
{
    handshakes_attempted.store(0);
    handshakes_successful.store(0);
    handshakes_failed.store(0);
    bytes_encrypted.store(0);
    bytes_decrypted.store(0);
    rekeys_performed.store(0);
    sessions_active.store(0);
    kyber1024_handshakes.store(0);
    chacha20poly1305_sessions.store(0);
    LogPrint(BCLog::NET, "PQ: Metrics reset\n");
}

void LogPQHandshakeStage(const std::string& stage, const std::string& details)
{
    if (details.empty()) {
        LogPrint(BCLog::NET, "PQ Handshake: %s\n", stage);
    } else {
        LogPrint(BCLog::NET, "PQ Handshake: %s - %s\n", stage, details);
    }
}

void LogPQError(const std::string& error, const std::string& context)
{
    if (context.empty()) {
        LogPrint(BCLog::NET, "PQ Error: %s\n", error);
    } else {
        LogPrint(BCLog::NET, "PQ Error [%s]: %s\n", context, error);
    }
}

void LogPQSuite(const std::string& suite)
{
    LogPrint(BCLog::NET, "PQ Suite: %s\n", suite);
}
EOF}
