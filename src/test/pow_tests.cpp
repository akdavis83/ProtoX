
#include <gtest/gtest.h>
#include "pow.h"
#include "primitives/block.h"
#include "arith_uint256.h"
#include "random.h"

TEST(PoWTest, MineAndVerify) {
    QTCBlockHeader block;
    block.nVersion = 1;
    block.hashPrevBlock = GetRandHash();
    block.hashMerkleRoot = GetRandHash();
    block.nTime = 1678886400; // March 15, 2023
    block.nBits = 0x1f00ffff; // A reasonably easy target for testing

    // Set some dummy quantum data
    block.kyber_challenge.resize(32, 0x01);
    block.kyber_response.resize(32, 0x02);
    block.quantum_salt.resize(32, 0x03);

    // Mine the block
    MineQTCBlock(block);

    // Verify the block
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    uint256 target = ArithToUint256(bnTarget);

    EXPECT_TRUE(VerifyQTCProofOfWork(block, target));
}
