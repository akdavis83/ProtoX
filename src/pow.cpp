#include "pow.h"

#include "arith_uint256.h"
#include "primitives/block.h"
#include "uint256.h"
#include "crypto/qtc_quantum_randomx.h"

bool CheckProofOfWork(const QTCBlockHeader& block, const uint256& target)
{
    std::array<uint8_t, 32> hash_array;
    std::copy(block.GetHash().begin(), block.GetHash().end(), hash_array.begin());
    // Convert block to mining context and verify using QTC-QUANTUM-RANDOMX
    qtc_mining::QTCMiningContext ctx;
    
    // Calculate actual epoch from block height (every 2016 blocks = 1 epoch)
    // Since nHeight is not in header, derive from chain context or use timestamp-based approach
    // For now, derive epoch from timestamp (simplified approach for production)
    uint32_t epoch_number = (block.nTime / (2016 * 600)) + 1; // ~2016 blocks * 10min = epoch
    qtc_mining::QTCQuantumRandomX::InitializeEpoch(epoch_number, ctx);
    
    std::array<uint8_t, 80> block_header;
    std::memcpy(block_header.data(), &block, 80);
    
    auto hash = qtc_mining::QTCQuantumRandomX::Mine(ctx, block_header, block.nNonce);
    return memcmp(hash.data(), target.data(), 32) < 0;
}

void MineQTCBlock(QTCBlockHeader& block)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);

    uint256 target = ArithToUint256(bnTarget);

    while (true) {
        block.nNonce++;
        if (CheckProofOfWork(block, target)) {
            break;
        }
    }
}

bool VerifyQTCProofOfWork(const QTCBlockHeader& block, const uint256& target)
{
    return CheckProofOfWork(block, target);
}