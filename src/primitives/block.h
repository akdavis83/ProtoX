
#ifndef QTC_PRIMITIVES_BLOCK_H
#define QTC_PRIMITIVES_BLOCK_H

#include <cstdint>
#include <vector>
#include "uint256.h"
#include "transaction.h"

// QTC Block Header with Quantum-Safe Fields
class QTCBlockHeader
{
public:
    // Standard Bitcoin header fields
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    // QTC Quantum-Safe Fields
    std::vector<unsigned char> kyber_challenge;
    std::vector<unsigned char> kyber_response;
    std::vector<unsigned char> quantum_salt;

    QTCBlockHeader()
    {
        SetNull();
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        kyber_challenge.clear();
        kyber_response.clear();
        quantum_salt.clear();
    }

    uint256 GetHash() const;

    // Serialization
    template<typename Stream>
    void Serialize(Stream& s) const {
        s << nVersion;
        s << hashPrevBlock;
        s << hashMerkleRoot;
        s << nTime;
        s << nBits;
        s << nNonce;
        s << kyber_challenge;
        s << kyber_response;
        s << quantum_salt;
    }

    template<typename Stream>
    void Unserialize(Stream& s) {
        s >> nVersion;
        s >> hashPrevBlock;
        s >> hashMerkleRoot;
        s >> nTime;
        s >> nBits;
        s >> nNonce;
        s >> kyber_challenge;
        s >> kyber_response;
        s >> quantum_salt;
    }
};

class CBlock : public QTCBlockHeader
{
public:
    // The transactions in this block
    std::vector<CTransactionRef> vtx;

    CBlock()
    {
        SetNull();
    }

    CBlock(const QTCBlockHeader &header)
    {
        SetNull();
        *((QTCBlockHeader*)this) = header;
    }

    void SetNull()
    {
        QTCBlockHeader::SetNull();
        vtx.clear();
    }

    QTCBlockHeader GetBlockHeader() const
    {
        QTCBlockHeader block;
        block.nVersion        = nVersion;
        block.hashPrevBlock   = hashPrevBlock;
        block.hashMerkleRoot  = hashMerkleRoot;
        block.nTime           = nTime;
        block.nBits           = nBits;
        block.nNonce          = nNonce;
        block.kyber_challenge = kyber_challenge;
        block.kyber_response  = kyber_response;
        block.quantum_salt    = quantum_salt;
        return block;
    }
};

#endif // QTC_PRIMITIVES_BLOCK_H
