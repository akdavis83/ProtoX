#include <script/qtc_quantum_script.h>
#include <script/interpreter.h>
#include <crypto/dilithium/dilithium3.h>
#include <qtc_wallet.h>
#include <logging.h>

namespace qtc_script {
    
    // Dilithium3 quantum signature verification
    bool EvalQTCDilithiumCheckSig(std::vector<std::vector<unsigned char>>& stack,
                                  const CScript& script) {
        if (stack.size() < 3) {
            LogPrint(BCLog::SCRIPT, "QTC_DILITHIUM_CHECKSIG: insufficient stack items\n");
            return false;
        }
        
        // Stack: [message_hash] [dilithium_signature] [dilithium_pubkey]
        auto pubkey_bytes = stack.back(); stack.pop_back();
        auto signature_bytes = stack.back(); stack.pop_back();  
        auto message_bytes = stack.back(); stack.pop_back();
        
        try {
            // Validate sizes
            if (pubkey_bytes.size() != qtc_dilithium::DILITHIUM3_PUBLICKEY_BYTES) {
                LogPrint(BCLog::SCRIPT, "QTC_DILITHIUM_CHECKSIG: invalid pubkey size %d\n", 
                        pubkey_bytes.size());
                stack.push_back({0}); // Push false
                return true;
            }
            
            // Convert to Dilithium format
            qtc_dilithium::PublicKey dilithium_pk;
            std::copy(pubkey_bytes.begin(), pubkey_bytes.end(), dilithium_pk.begin());
            
            qtc_dilithium::Signature dilithium_sig(signature_bytes.begin(), signature_bytes.end());
            
            // Verify quantum signature
            bool valid = qtc_dilithium::Verify(dilithium_sig, 
                                              std::span<const uint8_t>{message_bytes.data(), message_bytes.size()},
                                              dilithium_pk);
            
            LogPrint(BCLog::SCRIPT, "QTC_DILITHIUM_CHECKSIG: verification %s\n", 
                    valid ? "SUCCESS" : "FAILED");
            
            // Push result to stack
            stack.push_back(valid ? std::vector<unsigned char>{1} : std::vector<unsigned char>{0});
            
            return true;
            
        } catch (const std::exception& e) {
            LogPrint(BCLog::SCRIPT, "QTC_DILITHIUM_CHECKSIG: exception %s\n", e.what());
            stack.push_back({0}); // Push false
            return true;
        }
    }
    
    // Kyber1024 encryption operation
    bool EvalQTCKyberEncrypt(std::vector<std::vector<unsigned char>>& stack,
                            const CScript& script) {
        if (stack.size() < 2) {
            return false;
        }
        
        // Stack: [message] [kyber_public_key]
        auto pubkey_bytes = stack.back(); stack.pop_back();
        auto message_bytes = stack.back(); stack.pop_back();
        
        try {
            // Convert to Kyber format
            qtc_kyber::PublicKey kyber_pk;
            if (pubkey_bytes.size() != qtc_kyber::KYBER1024_PUBLICKEY_BYTES) {
                stack.push_back({0}); // Push false
                return true;
            }
            std::copy(pubkey_bytes.begin(), pubkey_bytes.end(), kyber_pk.begin());
            
            // Perform encryption
            auto [ciphertext, shared_secret] = qtc_kyber::Encrypt1024(kyber_pk);
            
            // Push ciphertext and shared secret to stack
            std::vector<unsigned char> ciphertext_vec(ciphertext.begin(), ciphertext.end());
            std::vector<unsigned char> secret_vec(shared_secret.begin(), shared_secret.end());
            
            stack.push_back(ciphertext_vec);
            stack.push_back(secret_vec);
            
            LogPrint(BCLog::SCRIPT, "QTC_KYBER_ENCRYPT: successful\n");
            return true;
            
        } catch (const std::exception& e) {
            LogPrint(BCLog::SCRIPT, "QTC_KYBER_ENCRYPT: exception %s\n", e.what());
            stack.push_back({0});
            return true;
        }
    }
    
    // Kyber1024 decryption operation  
    bool EvalQTCKyberDecrypt(std::vector<std::vector<unsigned char>>& stack,
                            const CScript& script) {
        if (stack.size() < 2) {
            return false;
        }
        
        // Stack: [ciphertext] [kyber_secret_key]
        auto seckey_bytes = stack.back(); stack.pop_back();
        auto ciphertext_bytes = stack.back(); stack.pop_back();
        
        try {
            // Convert to Kyber format
            qtc_kyber::SecretKey kyber_sk;
            qtc_kyber::Ciphertext kyber_ct;
            
            if (seckey_bytes.size() != qtc_kyber::KYBER1024_SECRETKEY_BYTES ||
                ciphertext_bytes.size() != qtc_kyber::KYBER1024_CIPHERTEXT_BYTES) {
                stack.push_back({0});
                return true;
            }
            
            std::copy(seckey_bytes.begin(), seckey_bytes.end(), kyber_sk.begin());
            std::copy(ciphertext_bytes.begin(), ciphertext_bytes.end(), kyber_ct.begin());
            
            // Perform decryption
            auto shared_secret = qtc_kyber::Decrypt1024(kyber_ct, kyber_sk);
            
            // Push shared secret to stack
            std::vector<unsigned char> secret_vec(shared_secret.begin(), shared_secret.end());
            stack.push_back(secret_vec);
            
            LogPrint(BCLog::SCRIPT, "QTC_KYBER_DECRYPT: successful\n");
            return true;
            
        } catch (const std::exception& e) {
            LogPrint(BCLog::SCRIPT, "QTC_KYBER_DECRYPT: exception %s\n", e.what());
            stack.push_back({0});
            return true;
        }
    }
    
    // QTC quantum address validation
    bool EvalQTCQuantumAddr(std::vector<std::vector<unsigned char>>& stack,
                           const CScript& script) {
        if (stack.size() < 1) {
            return false;
        }
        
        // Stack: [qtc_address_string]
        auto address_bytes = stack.back(); stack.pop_back();
        
        try {
            std::string address_str(address_bytes.begin(), address_bytes.end());
            
            // Validate QTC address format
            bool valid = IsValidQTCAddress(address_str);
            
            LogPrint(BCLog::SCRIPT, "QTC_QUANTUM_ADDR: validation %s for %s\n", 
                    valid ? "SUCCESS" : "FAILED", address_str);
            
            stack.push_back(valid ? std::vector<unsigned char>{1} : std::vector<unsigned char>{0});
            return true;
            
        } catch (const std::exception& e) {
            LogPrint(BCLog::SCRIPT, "QTC_QUANTUM_ADDR: exception %s\n", e.what());
            stack.push_back({0});
            return true;
        }
    }
    
    // Validate complete quantum transaction
    bool ValidateQuantumTransaction(const QTCQuantumTxIn& input,
                                   const QTCQuantumTxOut& output) {
        try {
            // Verify input signature
            bool sig_valid = qtc_dilithium::Verify(input.dilithium_signature,
                                                  std::span<const uint8_t>{input.message_hash.data(), 
                                                                          input.message_hash.size()},
                                                  input.dilithium_pubkey);
            if (!sig_valid) {
                LogPrint(BCLog::VALIDATION, "Quantum transaction: invalid input signature\n");
                return false;
            }
            
            // Verify output address
            bool addr_valid = IsValidQTCAddress(output.qtc_address);
            if (!addr_valid) {
                LogPrint(BCLog::VALIDATION, "Quantum transaction: invalid output address %s\n", 
                        output.qtc_address);
                return false;
            }
            
            // Verify amount
            if (output.value <= 0) {
                LogPrint(BCLog::VALIDATION, "Quantum transaction: invalid amount %lld\n", 
                        output.value);
                return false;
            }
            
            LogPrint(BCLog::VALIDATION, "Quantum transaction: validation SUCCESS\n");
            return true;
            
        } catch (const std::exception& e) {
            LogPrint(BCLog::VALIDATION, "Quantum transaction: validation exception %s\n", e.what());
            return false;
        }
    }
    
    // Create pay-to-quantum-address script
    CScript CreateQTCPayToQuantumAddress(const std::string& qtc_address) {
        CScript script;
        
        // Push address to stack
        std::vector<unsigned char> addr_bytes(qtc_address.begin(), qtc_address.end());
        script << addr_bytes;
        
        // Add quantum address validation
        script << static_cast<unsigned char>(OP_QTC_QUANTUM_ADDR);
        
        return script;
    }
    
    // Create quantum signature script
    CScript CreateQTCQuantumSignature(const qtc_dilithium::Signature& signature,
                                     const qtc_dilithium::PublicKey& pubkey) {
        CScript script;
        
        // Push signature
        std::vector<unsigned char> sig_bytes(signature.begin(), signature.end());
        script << sig_bytes;
        
        // Push public key  
        std::vector<unsigned char> pk_bytes(pubkey.begin(), pubkey.end());
        script << pk_bytes;
        
        // Add signature check
        script << static_cast<unsigned char>(OP_QTC_DILITHIUM_CHECKSIG);
        
        return script;
    }
}
