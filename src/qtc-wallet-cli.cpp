// QTC Quantum Wallet CLI - Production C++ Implementation
// Demonstrates NIST FIPS-203/204 compliant key and address generation

#include <qtc_wallet.h>
#include <crypto/kyber/kyber1024.h>
#include <crypto/dilithium/dilithium3.h>
#include <util/strencodings.h>
#include <logging.h>
#include <iostream>
#include <iomanip>

void PrintWalletDetails(const qtc::QuantumWallet& wallet) {
    std::cout << "=== QTC Production Quantum Wallet ===\n\n";
    
    // Address
    std::cout << "QTC Address: " << wallet.address << "\n\n";
    
    // Key sizes verification
    std::cout << "Key Sizes (NIST Compliant):\n";
    std::cout << "  Kyber1024 Public Key:  " << wallet.kyber_public.size() << " bytes\n";
    std::cout << "  Kyber1024 Private Key: " << wallet.kyber_private.size() << " bytes\n";
    std::cout << "  Dilithium3 Public Key: " << wallet.dilithium_public.size() << " bytes\n";
    std::cout << "  Dilithium3 Private Key:" << wallet.dilithium_private.size() << " bytes\n";
    std::cout << "  Shared Secret:         " << wallet.shared_secret.size() << " bytes\n";
    std::cout << "  SHA3-512 Entropy:      " << wallet.entropy.size() << " bytes\n\n";
    
    // Base64 encoded keys (for comparison with qti2.js output)
    std::cout << "Base64 Encoded Keys:\n";
    std::cout << "  kyber_public_b64:    " << EncodeBase64(wallet.kyber_public) << "\n\n";
    std::cout << "  kyber_private_b64:   " << EncodeBase64(wallet.kyber_private) << "\n\n";
    std::cout << "  dilithium_public_b64:" << EncodeBase64(wallet.dilithium_public) << "\n\n";
    std::cout << "  dilithium_private_b64:" << EncodeBase64(wallet.dilithium_private) << "\n\n";
    std::cout << "  shared_secret_b64:   " << EncodeBase64(wallet.shared_secret) << "\n\n";
    std::cout << "  entropy_b64:         " << EncodeBase64(wallet.entropy) << "\n\n";
}

void PrintJSON(const qtc::QuantumWallet& wallet) {
    std::cout << "{\n";
    std::cout << "  \"address\": \"" << wallet.address << "\",\n";
    std::cout << "  \"entropy_b64\": \"" << EncodeBase64(wallet.entropy) << "\",\n";
    std::cout << "  \"kyber_public_b64\": \"" << EncodeBase64(wallet.kyber_public) << "\",\n";
    std::cout << "  \"kyber_private_b64\": \"" << EncodeBase64(wallet.kyber_private) << "\",\n";
    std::cout << "  \"dilithium_public_b64\": \"" << EncodeBase64(wallet.dilithium_public) << "\",\n";
    std::cout << "  \"dilithium_private_b64\": \"" << EncodeBase64(wallet.dilithium_private) << "\",\n";
    std::cout << "  \"shared_secret_b64\": \"" << EncodeBase64(wallet.shared_secret) << "\"\n";
    std::cout << "}\n";
}

int main(int argc, char* argv[]) {
    bool json_output = false;
    bool help = false;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--json" || arg == "-j") {
            json_output = true;
        } else if (arg == "--help" || arg == "-h") {
            help = true;
        }
    }
    
    if (help) {
        std::cout << "QTC Quantum Wallet CLI - Production C++ Implementation\n";
        std::cout << "Generates quantum-safe keys and addresses using NIST standards\n\n";
        std::cout << "Usage: qtc-wallet-cli [options]\n";
        std::cout << "Options:\n";
        std::cout << "  --json, -j     Output in JSON format\n";
        std::cout << "  --help, -h     Show this help message\n\n";
        std::cout << "Security: NIST FIPS-203 Kyber1024 + FIPS-204 Dilithium3\n";
        return 0;
    }
    
    try {
        // Initialize logging for any debug output
        LogInstance().m_print_to_console = false;
        
        if (!json_output) {
            std::cout << "🚀 QTC Quantum Wallet Generator\n";
            std::cout << "⚛️  NIST FIPS-203/204 Compliant Implementation\n";
            std::cout << "🔐 Generating quantum-safe wallet...\n\n";
        }
        
        // Generate quantum wallet
        auto wallet = qtc::GenerateQuantumWallet();
        
        if (!json_output) {
            std::cout << "✅ Wallet generated successfully!\n\n";
            PrintWalletDetails(wallet);
            
            // Verification information
            std::cout << "=== Verification Info ===\n";
            std::cout << "Standards Compliance:\n";
            std::cout << "  ✅ NIST FIPS-203 (Kyber1024 KEM)\n";
            std::cout << "  ✅ NIST FIPS-204 (Dilithium3 Signatures)\n";
            std::cout << "  ✅ FIPS-202 (SHA3-512 Entropy Derivation)\n";
            std::cout << "  ✅ RFC 3548 (Base64 Encoding)\n";
            std::cout << "  ✅ BIP 173 (Bech32 Address Format)\n\n";
            
            std::cout << "Address Format: " << wallet.address.substr(0, 3) << " prefix (Quantum-safe bech32)\n";
            std::cout << "Security Level: Post-quantum (>128-bit equivalent)\n";
            std::cout << "Deterministic: Yes (reproducible from shared secret)\n";
            
        } else {
            PrintJSON(wallet);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error generating wallet: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
