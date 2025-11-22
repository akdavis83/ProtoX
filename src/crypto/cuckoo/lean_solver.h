// QTC Lean Cuckoo Cycle Solver
// Week 3: Ultra-Fast 24-Edge Cycle Finding

#ifndef QTC_CRYPTO_CUCKOO_LEAN_SOLVER_H
#define QTC_CRYPTO_CUCKOO_LEAN_SOLVER_H

#include <cstdint>
#include <array>
#include <vector>
#include <immintrin.h>

namespace qtc_cuckoo_lean {

// Lean Cuckoo Cycle parameters (optimized for speed)
static constexpr size_t CUCKOO_SIZE_LOG = 20;                    // 1M nodes (small for speed)
static constexpr size_t CUCKOO_SIZE = 1 << CUCKOO_SIZE_LOG;      // 1,048,576 nodes
static constexpr size_t CUCKOO_MASK = CUCKOO_SIZE - 1;          // Bit mask
static constexpr size_t PROOF_SIZE = 24;                         // 24-edge cycle (small)
static constexpr size_t MAX_PATH_LENGTH = 8192;                  // Path search limit

// Ultra-compact edge representation
struct CompactEdge {
    uint32_t u : 20;  // Source node (20 bits = 1M nodes)
    uint32_t v : 20;  // Target node (20 bits = 1M nodes) 
    uint32_t nonce : 24; // Edge nonce (24 bits)
} __attribute__((packed));

// High-performance edge generation
class EdgeGenerator {
private:
    alignas(32) uint8_t m_siphash_keys[32];     // SipHash keys for edge generation
    alignas(32) __m256i m_simd_state[4];        // SIMD state for parallel generation

public:
    explicit EdgeGenerator(const std::array<uint8_t, 32>& seed);
    
    // SIMD-optimized edge generation (8 edges at once)
    void GenerateEdgeBatch(__m256i nonce_batch, CompactEdge* edges) noexcept;
    
    // Single edge generation (fallback)
    CompactEdge GenerateEdge(uint32_t nonce) noexcept;
    
    // Vectorized edge validation
    uint32_t ValidateEdgeBatch(const CompactEdge* edges, size_t count) noexcept;

private:
    // Optimized SipHash for edge generation
    uint64_t siphash_2_4_optimized(uint64_t data) noexcept;
    __m256i siphash_simd_batch(__m256i data_batch) noexcept;
};

// Lightning-fast cycle finder
class LeanCycleFinder {
private:
    // Compact adjacency representation
    struct NodeInfo {
        uint32_t first_edge : 20;    // First edge index
        uint32_t edge_count : 12;    // Number of edges (max 4096)
    };
    
    alignas(64) NodeInfo m_nodes[CUCKOO_SIZE];           // Node adjacency info
    alignas(64) uint32_t m_edge_targets[CUCKOO_SIZE * 4]; // Edge target storage
    alignas(64) uint32_t m_visited[CUCKOO_SIZE / 32];     // Visited bitmap
    
    // Path tracking for cycle detection
    alignas(32) uint32_t m_path[MAX_PATH_LENGTH];
    alignas(32) uint32_t m_parent[CUCKOO_SIZE];
    
    size_t m_total_edges{0};
    uint64_t m_cycles_found{0};

public:
    LeanCycleFinder();
    
    // Graph construction
    void BuildGraph(const std::vector<CompactEdge>& edges) noexcept;
    void ClearGraph() noexcept;
    
    // Ultra-fast cycle finding
    std::vector<uint32_t> FindCycle24() noexcept;
    bool FindCycleFromNode(uint32_t start_node, std::vector<uint32_t>& cycle) noexcept;
    
    // Performance monitoring
    uint64_t GetCyclesFound() const noexcept { return m_cycles_found; }
    double GetGraphDensity() const noexcept;
    
private:
    // Optimized graph operations
    bool dfs_cycle_search(uint32_t node, uint32_t target, uint32_t depth) noexcept;
    void mark_visited_simd(uint32_t node) noexcept;
    bool is_visited_simd(uint32_t node) const noexcept;
    void clear_visited_simd() noexcept;
};

// Complete lean solver
class LeanCuckooSolver {
private:
    EdgeGenerator m_generator;
    LeanCycleFinder m_finder;
    
    // Performance counters
    uint64_t m_solve_attempts{0};
    uint64_t m_successful_solves{0};
    uint64_t m_total_solve_time_us{0};

public:
    explicit LeanCuckooSolver(const std::array<uint8_t, 32>& seed);
    
    // Main solving interface
    std::vector<uint32_t> SolveFast(uint32_t max_nonces = 1024) noexcept;
    bool VerifyProof(const std::vector<uint32_t>& proof) noexcept;
    
    // Performance monitoring
    double GetSuccessRate() const noexcept;
    uint64_t GetAverageSolveTime() const noexcept;
    void ResetCounters() noexcept;
    
private:
    // Optimized solving strategy
    bool attempt_solve_batch(uint32_t start_nonce, uint32_t batch_size, 
                            std::vector<uint32_t>& proof) noexcept;
};

// SIMD utilities for Cuckoo operations
namespace simd_cuckoo {
    // Parallel edge processing
    void process_edge_batch_avx2(const CompactEdge* edges, size_t count,
                                 uint32_t* u_nodes, uint32_t* v_nodes) noexcept;
    
    // Parallel node operations
    __m256i gather_node_info(__m256i node_indices, const void* base_addr) noexcept;
    void scatter_node_info(__m256i node_indices, __m256i data, void* base_addr) noexcept;
    
    // Bit manipulation utilities
    uint32_t count_set_bits_avx2(const uint32_t* bitmap, size_t words) noexcept;
    void clear_bitmap_avx2(uint32_t* bitmap, size_t words) noexcept;
}

} // namespace qtc_cuckoo_lean

#endif // QTC_CRYPTO_CUCKOO_LEAN_SOLVER_H
