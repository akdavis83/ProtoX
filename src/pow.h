#ifndef QTC_POW_H
#define QTC_POW_H

#include <cstdint>

class QTCBlockHeader;
class CBlockIndex;
class uint256;

/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(const QTCBlockHeader& block, const uint256& target);

/** Mine a QTC block using the QTC-QUANTUM-POW algorithm */
void MineQTCBlock(QTCBlockHeader& block);

/** Verify a QTC block's proof of work using the QTC-QUANTUM-POW algorithm */
bool VerifyQTCProofOfWork(const QTCBlockHeader& block, const uint256& target);

#endif // QTC_POW_H