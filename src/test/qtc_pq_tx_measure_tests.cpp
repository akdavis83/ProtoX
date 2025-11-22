#include <boost/test/unit_test.hpp>
#include <primitives/transaction.h>
#include <script/script.h>
#include <streams.h>
#include <crypto/qtc_hash.h>

static CTxOut MakePqOutput(const std::vector<uint8_t>& dilithium_pk, CAmount value)
{
    auto program20 = QTC_Program20_From_PK_SHA3_256(dilithium_pk.data(), dilithium_pk.size());
    CScript spk; spk << OP_1 << std::vector<unsigned char>(program20.begin(), program20.end());
    return CTxOut(value, spk);
}

BOOST_AUTO_TEST_SUITE(qtc_pq_tx_measure_tests)

BOOST_AUTO_TEST_CASE(qtc_pq_endtoend_weight_measurement)
{
    // Funding tx with PQ output
    CMutableTransaction funding; funding.nVersion = 2;
    funding.vin.resize(1);
    funding.vin[0].prevout.SetNull();
    funding.vin[0].scriptSig = CScript() << OP_0;

    std::vector<uint8_t> dilithium_pk(1952, 0x42);
    funding.vout.emplace_back(MakePqOutput(dilithium_pk, 100000));

    CTransaction funding_tx(funding);

    // Spending tx with PQ witness
    CMutableTransaction spend; spend.nVersion = 2;
    spend.vin.resize(1);
    spend.vin[0].prevout = COutPoint(funding_tx.GetHash(), 0);

    CScript nulldata; nulldata << OP_RETURN << std::vector<unsigned char>{'P','Q'};
    spend.vout.emplace_back(90000, nulldata);

    std::vector<unsigned char> mock_sig(3293, 0x99);
    std::vector<unsigned char> mock_pk(1952, 0x42);
    spend.vin[0].scriptWitness.stack.emplace_back(mock_sig.begin(), mock_sig.end());
    spend.vin[0].scriptWitness.stack.emplace_back(mock_pk.begin(), mock_pk.end());

    // Base-only
    CDataStream ssb(SER_NETWORK, PROTOCOL_VERSION);
    ssb << CTransaction(spend).SerializeWithoutWitness();
    size_t base_size = ssb.size();

    // With witness
    CDataStream ssw(SER_NETWORK, PROTOCOL_VERSION);
    ssw << CTransaction(spend);
    size_t total_size = ssw.size();

    size_t witness_size = total_size - base_size;
    size_t weight = base_size * 4 + witness_size;
    size_t legacy_effective = base_size + witness_size; // no discount counterfactual
    size_t qtc_effective = weight / 4;

    BOOST_TEST_MESSAGE("Base size:     " << base_size);
    BOOST_TEST_MESSAGE("Witness size:  " << witness_size);
    BOOST_TEST_MESSAGE("Total size:    " << total_size);
    BOOST_TEST_MESSAGE("Weight:        " << weight);
    BOOST_TEST_MESSAGE("Legacy eff:    " << legacy_effective);
    BOOST_TEST_MESSAGE("QTC effective: " << qtc_effective);

    BOOST_CHECK(qtc_effective < legacy_effective);
    BOOST_CHECK(witness_size >= (mock_sig.size() + mock_pk.size()));
}

BOOST_AUTO_TEST_SUITE_END()
