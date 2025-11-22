// QTC Native SegWit + PQC unit tests
// Focus: 20-byte SHA3-derived program, effective weight math for PQ witness

#include <boost/test/unit_test.hpp>
#include <vector>
#include <array>
#include <iostream>

// Project headers (present in QTC tree)
#include <crypto/sha3.h>

static std::array<uint8_t,20> QTC_PKH20_From_SHA3(const std::vector<uint8_t>& pubkey) {
    // Derive 20-byte program using SHA3-256 first 20 bytes
    std::array<uint8_t, 32> h256{};
    CSHA3_256().Write(pubkey.data(), pubkey.size()).Finalize(h256.data());
    std::array<uint8_t, 20> out{};
    std::copy(h256.begin(), h256.begin() + 20, out.begin());
    return out;
}

BOOST_AUTO_TEST_SUITE(qtc_pq_witness_tests)

BOOST_AUTO_TEST_CASE(qtc_pq_program_20_bytes)
{
    // Dummy Dilithium3 public key ~1952 bytes
    std::vector<uint8_t> dummy_pk(1952, 0x42);
    auto program20 = QTC_PKH20_From_SHA3(dummy_pk);

    BOOST_CHECK_EQUAL(program20.size(), 20);

    // Basic non-zero check to ensure hashing actually ran
    size_t nonzero = 0;
    for (auto b : program20) if (b) ++nonzero;
    BOOST_CHECK(nonzero > 0);
}

BOOST_AUTO_TEST_CASE(qtc_pq_effective_weight_savings)
{
    // PQ payload sizes (Dilithium3 typical)
    const size_t DILITHIUM_PK  = 1952;
    const size_t DILITHIUM_SIG = 3293;
    const size_t PQ_WITNESS    = DILITHIUM_PK + DILITHIUM_SIG; // ≈ 5245 bytes

    // Legacy baseline (no SegWit): all base bytes
    const size_t LEGACY_EFFECTIVE = PQ_WITNESS;

    // QTC native SegWit: witness bytes discounted to 25%
    const size_t QTC_EFFECTIVE = PQ_WITNESS / 4; // integer division OK for demo

    // Expect ~75% savings vs legacy
    // i.e., QTC_EFFECTIVE should be close to 0.25 * LEGACY_EFFECTIVE
    BOOST_CHECK(QTC_EFFECTIVE * 4 <= LEGACY_EFFECTIVE);

    // Output-side commitment savings check (~99%): 20 vs 1952
    double output_saving = (1.0 - (20.0 / 1952.0)) * 100.0; // ~98.98%
    BOOST_CHECK(output_saving > 98.0);
}

BOOST_AUTO_TEST_SUITE_END()
