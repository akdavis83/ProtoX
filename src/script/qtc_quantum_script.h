// QTC Quantum Script Operations
// Quantum-safe transaction validation for QTC

#ifndef QTC_SCRIPT_QUANTUM_SCRIPT_H
#define QTC_SCRIPT_QUANTUM_SCRIPT_H

#include <crypto/dilithium/dilithium3.h>
#include <crypto/kyber/kyber1024.h>
#include <script/script.h>
#include <span>

namespace qtc_script {
    // New QTC quantum script opcodes
    enum QTCOpCode {
        OP_QTC_DILITHIUM_CHECKSIG = 0xf0,      // Dilithium3 signature verification
        OP_QTC_KYBER_ENCRYPT = 0xf1,           // Kyber1024 encryption
        OP_QTC_KYBER_DECRYPT = 0xf2,           // Kyber1024 decryption
        OP_QTC_QUANTUM_ADDR = 0xf3             // Quantum address validation
    };
    
    // Quantum transaction input/output structures
    struct QTCQuantumTxIn {
        std::vector<uint8_t> dilithium_signature;    // Quantum-safe signature
        qtc_dilithium::PublicKey dilithium_pubkey;   // Signer's public key
        std::vector<uint8_t> message_hash;           // Hash of transaction data
    };
    
    struct QTCQuantumTxOut {
        uint64_t value;                              // Amount in satoshis
        std::string qtc_address;                     // QTC quantum-safe address  
        qtc_dilithium::PublicKey recipient_pubkey;   // Recipient's public key
    };
    
    // Quantum script evaluation functions
    bool EvalQTCDilithiumCheckSig(std::vector<std::vector<unsigned char>>& stack,
                                  const CScript& script);
    
    bool EvalQTCKyberEncrypt(std::vector<std::vector<unsigned char>>& stack,
                            const CScript& script);
                            
    bool EvalQTCKyberDecrypt(std::vector<std::vector<unsigned char>>& stack,
                            const CScript& script);
                            
    bool EvalQTCQuantumAddr(std::vector<std::vector<unsigned char>>& stack,
                           const CScript& script);
    
    // Quantum transaction validation
    bool ValidateQuantumTransaction(const QTCQuantumTxIn& input,
                                   const QTCQuantumTxOut& output);
    
    // Create quantum-safe transaction scripts
    CScript CreateQTCPayToQuantumAddress(const std::string& qtc_address);
    CScript CreateQTCQuantumSignature(const qtc_dilithium::Signature& signature,
                                     const qtc_dilithium::PublicKey& pubkey);
}

#endif // QTC_SCRIPT_QUANTUM_SCRIPT_H
