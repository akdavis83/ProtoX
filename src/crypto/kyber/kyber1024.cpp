#include <crypto/kyber/kyber1024.h>
#include <crypto/sha3.h>
#include <random.h>
#include <support/allocators/secure.h>

namespace qtc_kyber {
    // NTT constants from kyber1024.js
    const std::array<uint16_t, 128> NTT_ZETAS = {{
        2285, 2571, 2970, 1812, 1493, 1422, 287, 202, 3158, 622, 1577, 182, 962,
        2127, 1855, 1468, 573, 2004, 264, 383, 2500, 1458, 1727, 3199, 2648, 1017,
        732, 608, 1787, 411, 3124, 1758, 1223, 652, 2777, 1015, 2036, 1491, 3047,
        1785, 516, 3321, 3009, 2663, 1711, 2167, 126, 1469, 2476, 3239, 3058, 830,
        107, 1908, 3082, 2378, 2931, 961, 1821, 2604, 448, 2264, 677, 2054, 2226,
        430, 555, 843, 2078, 871, 1550, 105, 422, 587, 177, 3094, 3038, 2869, 1574,
        1653, 3083, 778, 1159, 3182, 2552, 1483, 2727, 1119, 1739, 644, 2457, 349,
        418, 329, 3173, 3254, 817, 1097, 603, 610, 1322, 2044, 1864, 384, 2114, 3193,
        1218, 1994, 2455, 220, 2142, 1670, 2144, 1799, 2051, 794, 1819, 2475, 2459,
        478, 3221, 3021, 996, 991, 958, 1869, 1522, 1628
    }};

    const std::array<uint16_t, 128> NTT_ZETAS_INV = {{
        1701, 1807, 1460, 2371, 2338, 2333, 308, 108, 2851, 870, 854, 1510, 2535,
        1278, 1530, 1185, 1659, 1187, 3109, 874, 1335, 2111, 136, 1215, 2945, 1465,
        1285, 2007, 2719, 2726, 2232, 2512, 75, 156, 3000, 2911, 2980, 872, 2685,
        1590, 2210, 602, 1846, 777, 147, 2170, 2551, 246, 1676, 1755, 460, 291, 235,
        3152, 2742, 2907, 3224, 1779, 2458, 1251, 2486, 2774, 2899, 1103, 1275, 2652,
        1065, 2881, 725, 1508, 2368, 398, 951, 247, 1421, 3222, 2499, 271, 90, 853,
        1860, 3203, 1162, 1618, 666, 320, 8, 2813, 1544, 282, 1838, 1293, 2314, 552,
        2677, 2106, 1571, 205, 2918, 1542, 2721, 2597, 2312, 681, 130, 1602, 1871,
        829, 2946, 3065, 1325, 2756, 1861, 1474, 1202, 2367, 3147, 1752, 2707, 171,
        3127, 3042, 1907, 1836, 1517, 359, 758, 1441
    }};

    // Main Kyber1024 KeyGen (matching JavaScript implementation)
    std::pair<PublicKey, SecretKey> KeyGen1024() {
        // Step 1: IND-CPA keypair generation
        auto [pk_vec, sk_indcpa_vec] = indcpaKeyGen();
        
        PublicKey pk;
        SecretKey sk;
        
        // Copy public key
        std::copy(pk_vec.begin(), pk_vec.end(), pk.begin());
        
        // Step 2: FO transform to IND-CCA2
        // Copy IND-CPA secret key
        std::copy(sk_indcpa_vec.begin(), sk_indcpa_vec.end(), sk.begin());
        
        // Append public key  
        std::copy(pk.begin(), pk.end(), sk.begin() + 1536);
        
        // Append hash of public key (SHA3-512)
        std::array<uint8_t, 32> pk_hash;
        CSHA3_512().Write(pk.data(), pk.size()).Finalize(pk_hash.data());
        std::copy(pk_hash.begin(), pk_hash.end(), sk.begin() + 3104);
        
        // Append random 32 bytes
        std::array<uint8_t, 32> randomness;
        GetStrongRandBytes(randomness);
        std::copy(randomness.begin(), randomness.end(), sk.begin() + 3136);
        
        return {pk, sk};
    }

    // Kyber1024 Encrypt (matching JavaScript implementation)  
    std::pair<Ciphertext, SharedSecret> Encrypt1024(const PublicKey& pk) {
        // Step 1: Generate random message m
        std::array<uint8_t, 32> m;
        GetStrongRandBytes(m);
        
        // Step 2: Hash message with SHA3-256
        std::array<uint8_t, 32> m_hash;
        CSHA3_512().Write(m.data(), m.size()).Finalize(m_hash.data());
        
        // Step 3: Hash public key with SHA3-512
        std::array<uint8_t, 32> pk_hash;
        CSHA3_512().Write(pk.data(), pk.size()).Finalize(pk_hash.data());
        
        // Step 4: Hash m_hash || pk_hash with SHA3-512
        std::array<uint8_t, 64> kr;
        CSHA3_512().Write(m_hash.data(), 32).Write(pk_hash.data(), 32).Finalize(kr.data());
        
        std::array<uint8_t, 32> kr1, kr2;
        std::copy(kr.begin(), kr.begin() + 32, kr1.begin());
        std::copy(kr.begin() + 32, kr.end(), kr2.begin());
        
        // Step 5: IND-CPA encrypt
        std::vector<uint8_t> pk_vec(pk.begin(), pk.end());
        std::vector<uint8_t> c_vec = indcpaEncrypt(pk_vec, m_hash, kr2);
        
        Ciphertext c;
        std::copy(c_vec.begin(), c_vec.end(), c.begin());
        
        // Step 6: Hash ciphertext with SHA3-512
        std::array<uint8_t, 32> c_hash;
        CSHA3_512().Write(c.data(), c.size()).Finalize(c_hash.data());
        
        // Step 7: Final shared secret with SHAKE-256  
        SharedSecret ss;
        CSHAKE256().Write(kr1.data(), 32).Write(c_hash.data(), 32).Finalize(ss.data(), 32);
        
        return {c, ss};
    }

    // Kyber1024 Decrypt (matching JavaScript implementation)
    SharedSecret Decrypt1024(const Ciphertext& c, const SecretKey& sk) {
        // Extract components from secret key
        std::vector<uint8_t> sk_indcpa(sk.begin(), sk.begin() + 1536);
        std::array<uint8_t, 32> pk_hash;
        std::array<uint8_t, 32> z;
        
        std::copy(sk.begin() + 3104, sk.begin() + 3136, pk_hash.begin());
        std::copy(sk.begin() + 3136, sk.end(), z.begin());
        
        // IND-CPA decrypt
        std::vector<uint8_t> c_vec(c.begin(), c.end());
        std::array<uint8_t, 32> m = indcpaDecrypt(c_vec, sk_indcpa);
        
        // Hash m || pk_hash with SHA3-512
        std::array<uint8_t, 64> kr;
        CSHA3_512().Write(m.data(), 32).Write(pk_hash.data(), 32).Finalize(kr.data());
        
        std::array<uint8_t, 32> kr1, kr2;
        std::copy(kr.begin(), kr.begin() + 32, kr1.begin());
        std::copy(kr.begin() + 32, kr.end(), kr2.begin());
        
        // Re-encrypt to verify
        std::vector<uint8_t> pk(sk.begin() + 1536, sk.begin() + 3104);
        std::vector<uint8_t> c_check = indcpaEncrypt(pk, m, kr2);
        
        // Compare ciphertexts
        bool decryption_success = (c_vec == c_check);
        
        // Hash ciphertext
        std::array<uint8_t, 32> c_hash;
        CSHA3_512().Write(c.data(), c.size()).Finalize(c_hash.data());
        
        // Generate shared secret
        SharedSecret ss;
        if (decryption_success) {
            // Use kr1 for successful decryption
            CSHAKE256().Write(kr1.data(), 32).Write(c_hash.data(), 32).Finalize(ss.data(), 32);
        } else {
            // Use z for failed decryption  
            CSHAKE256().Write(z.data(), 32).Write(c_hash.data(), 32).Finalize(ss.data(), 32);
        }
        
        return ss;
    }

    // NTT implementation (from kyber1024.js ntt function)
    void ntt(Polynomial& r) {
        size_t k = 1;
        for (size_t len = 128; len >= 2; len >>= 1) {
            for (size_t start = 0; start < KYBER_N; start += 2 * len) {
                int16_t zeta = NTT_ZETAS[k++];
                for (size_t j = start; j < start + len; ++j) {
                    int16_t t = nttFqMul(zeta, r[j + len]);
                    r[j + len] = r[j] - t;
                    r[j] = r[j] + t;
                }
            }
        }
    }
    
    // Inverse NTT (from kyber1024.js nttInverse function) 
    void nttInverse(Polynomial& r) {
        size_t k = 0;
        for (size_t len = 2; len <= 128; len <<= 1) {
            for (size_t start = 0; start < KYBER_N; start += 2 * len) {
                int16_t zeta = NTT_ZETAS_INV[k++];
                for (size_t j = start; j < start + len; ++j) {
                    int16_t t = r[j];
                    r[j] = barrett(t + r[j + len]);
                    r[j + len] = t - r[j + len];
                    r[j + len] = nttFqMul(zeta, r[j + len]);
                }
            }
        }
        for (size_t j = 0; j < KYBER_N; ++j) {
            r[j] = nttFqMul(r[j], NTT_ZETAS_INV[127]);
        }
    }

    // Montgomery multiplication (from kyber1024.js)
    int16_t nttFqMul(int16_t a, int16_t b) {
        return montgomeryReduce(static_cast<int32_t>(a) * b);
    }

    // Montgomery reduction (from kyber1024.js byteopsMontgomeryReduce)
    int16_t montgomeryReduce(int32_t a) {
        int16_t u = static_cast<int16_t>(a * KYBER_QINV);
        int32_t t = u * KYBER_Q;
        t = a - t;
        t >>= 16;
        return static_cast<int16_t>(t);
    }

    // Barrett reduction (from kyber1024.js barrett)
    int16_t barrett(int16_t a) {
        int32_t v = ((1LL << 24) + KYBER_Q / 2) / KYBER_Q;
        int32_t t = (v * a) >> 24;
        t = t * KYBER_Q;
        return a - t;
    }

    // Complete IND-CPA key generation implementation
    std::pair<std::vector<uint8_t>, std::vector<uint8_t>> indcpaKeyGen() {
        std::vector<uint8_t> pk(KYBER1024_PUBLICKEY_BYTES);
        std::vector<uint8_t> sk(1536);
        
        // Generate random seed
        std::array<uint8_t, 32> seed;
        GetStrongRandBytes(seed);
        
        // Expand seed using SHAKE-128
        std::array<uint8_t, 64> buf;
        CSHAKE128().Write(seed.data(), 32).Finalize(buf.data(), 64);
        
        std::array<uint8_t, 32> rho, sigma;
        std::copy(buf.begin(), buf.begin() + 32, rho.begin());
        std::copy(buf.begin() + 32, buf.end(), sigma.begin());
        
        // Generate matrix A and secret vectors
        auto A = generateMatrixA(rho, false);
        
        uint8_t nonce = 0;
        PolyVector s, e;
        for (size_t i = 0; i < KYBER_K; ++i) {
            s[i] = sample(sigma, nonce++);
            e[i] = sample(sigma, nonce++);
        }
        
        // Compute t = As + e
        PolyVector t;
        for (size_t i = 0; i < KYBER_K; ++i) {
            ntt(s[i]);
        }
        
        for (size_t i = 0; i < KYBER_K; ++i) {
            polyMul(t, A, s);
            nttInverse(t[i]);
            polyAdd(t[i], t[i], e[i]);
            polyReduce(t[i]);
        }
        
        // Pack keys
        size_t pk_offset = 0;
        for (size_t i = 0; i < KYBER_K; ++i) {
            polyToBytes({pk.data() + pk_offset, 384}, t[i]);
            pk_offset += 384;
        }
        std::copy(rho.begin(), rho.end(), pk.begin() + pk_offset);
        
        for (size_t i = 0; i < KYBER_K; ++i) {
            polyToBytes({sk.data() + i * 384, 384}, s[i]);
        }
        
        return {pk, sk};
    }

    std::vector<uint8_t> indcpaEncrypt(const std::vector<uint8_t>& pk, 
                                      const std::array<uint8_t, 32>& msg, 
                                      const std::array<uint8_t, 32>& coins) {
        std::vector<uint8_t> c(KYBER1024_CIPHERTEXT_BYTES);
        
        // Unpack public key
        PolyVector t;
        std::array<uint8_t, 32> rho;
        size_t offset = 0;
        
        for (size_t i = 0; i < KYBER_K; ++i) {
            polyFromBytes(t[i], {pk.data() + offset, 384});
            offset += 384;
        }
        std::copy(pk.begin() + offset, pk.begin() + offset + 32, rho.begin());
        
        // Generate matrix A
        auto A = generateMatrixA(rho, true);
        
        // Sample noise vectors
        uint8_t nonce = 0;
        PolyVector r, e1;
        Polynomial e2;
        
        for (size_t i = 0; i < KYBER_K; ++i) {
            r[i] = sample(coins, nonce++);
            e1[i] = sample(coins, nonce++);
        }
        e2 = sample(coins, nonce++);
        
        // Compute u = A^T r + e1
        PolyVector u;
        for (size_t i = 0; i < KYBER_K; ++i) {
            ntt(r[i]);
        }
        
        for (size_t i = 0; i < KYBER_K; ++i) {
            polyMul(u, A, r);
            nttInverse(u[i]);
            polyAdd(u[i], u[i], e1[i]);
        }
        
        // Compute v = t^T r + e2 + decompress(msg)
        Polynomial v, msg_poly;
        polyFromMsg(msg_poly, msg);
        
        for (size_t i = 0; i < KYBER_K; ++i) {
            ntt(t[i]);
        }
        
        // v = sum(t[i] * r[i]) + e2 + msg_poly
        std::fill(v.begin(), v.end(), 0);
        for (size_t i = 0; i < KYBER_K; ++i) {
            Polynomial temp;
            for (size_t j = 0; j < KYBER_N; ++j) {
                temp[j] = nttFqMul(t[i][j], r[i][j]);
            }
            nttInverse(temp);
            polyAdd(v, v, temp);
        }
        polyAdd(v, v, e2);
        polyAdd(v, v, msg_poly);
        
        // Pack ciphertext
        offset = 0;
        for (size_t i = 0; i < KYBER_K; ++i) {
            polyToBytes({c.data() + offset, 384}, u[i]);
            offset += 384;
        }
        
        // Compress and pack v
        auto compressed_v = compress2(v);
        std::copy(compressed_v.begin(), compressed_v.end(), c.begin() + offset);
        
        return c;
    }

    std::array<uint8_t, 32> indcpaDecrypt(const std::vector<uint8_t>& c, 
                                          const std::vector<uint8_t>& sk) {
        // Unpack ciphertext
        PolyVector u;
        std::array<uint8_t, 160> compressed_v;
        
        size_t offset = 0;
        for (size_t i = 0; i < KYBER_K; ++i) {
            polyFromBytes(u[i], {c.data() + offset, 384});
            offset += 384;
        }
        std::copy(c.begin() + offset, c.begin() + offset + 160, compressed_v.begin());
        
        Polynomial v = decompress2(compressed_v);
        
        // Unpack secret key
        PolyVector s;
        for (size_t i = 0; i < KYBER_K; ++i) {
            polyFromBytes(s[i], {sk.data() + i * 384, 384});
        }
        
        // Compute mp = v - s^T u
        for (size_t i = 0; i < KYBER_K; ++i) {
            ntt(s[i]);
            ntt(u[i]);
        }
        
        Polynomial mp = v;
        for (size_t i = 0; i < KYBER_K; ++i) {
            Polynomial temp;
            for (size_t j = 0; j < KYBER_N; ++j) {
                temp[j] = nttFqMul(s[i][j], u[i][j]);
            }
            nttInverse(temp);
            polySubtract(mp, mp, temp);
        }
        
        // Convert back to message
        std::array<uint8_t, 32> msg;
        polyToMsg(msg, mp);
        
        return msg;
    }

    // Complete polynomial operations implementation
    void polyAdd(Polynomial& result, const Polynomial& a, const Polynomial& b) {
        for (size_t i = 0; i < KYBER_N; ++i) {
            result[i] = a[i] + b[i];
        }
    }

    void polySubtract(Polynomial& result, const Polynomial& a, const Polynomial& b) {
        for (size_t i = 0; i < KYBER_N; ++i) {
            result[i] = a[i] - b[i];
        }
    }

    void polyReduce(Polynomial& poly) {
        for (size_t i = 0; i < KYBER_N; ++i) {
            poly[i] = barrett(poly[i]);
        }
    }

    void polyMul(Polynomial& result, const PolyVector& a, const PolyVector& b) {
        std::fill(result.begin(), result.end(), 0);
        for (size_t i = 0; i < KYBER_K; ++i) {
            for (size_t j = 0; j < KYBER_N; ++j) {
                result[j] += nttFqMul(a[i][j], b[i][j]);
            }
        }
    }

    void polyToBytes(std::array<uint8_t, 384>& bytes, const Polynomial& poly) {
        for (size_t i = 0; i < KYBER_N / 2; ++i) {
            uint16_t t0 = poly[2*i] % KYBER_Q;
            uint16_t t1 = poly[2*i + 1] % KYBER_Q;
            bytes[3*i + 0] = static_cast<uint8_t>(t0);
            bytes[3*i + 1] = static_cast<uint8_t>((t0 >> 8) | ((t1 & 0xF) << 4));
            bytes[3*i + 2] = static_cast<uint8_t>(t1 >> 4);
        }
    }

    void polyFromBytes(Polynomial& poly, const std::array<uint8_t, 384>& bytes) {
        for (size_t i = 0; i < KYBER_N / 2; ++i) {
            uint16_t t0 = bytes[3*i + 0] | (static_cast<uint16_t>(bytes[3*i + 1] & 0xF) << 8);
            uint16_t t1 = (bytes[3*i + 1] >> 4) | (static_cast<uint16_t>(bytes[3*i + 2]) << 4);
            poly[2*i] = t0;
            poly[2*i + 1] = t1;
        }
    }

    void polyToMsg(std::array<uint8_t, 32>& msg, const Polynomial& poly) {
        std::fill(msg.begin(), msg.end(), 0);
        for (size_t i = 0; i < KYBER_N; ++i) {
            uint16_t t = ((static_cast<uint32_t>(poly[i]) << 1) + KYBER_Q / 2) / KYBER_Q;
            msg[i / 8] |= (t & 1) << (i % 8);
        }
    }

    void polyFromMsg(Polynomial& poly, const std::array<uint8_t, 32>& msg) {
        for (size_t i = 0; i < KYBER_N; ++i) {
            uint16_t mask = (msg[i / 8] >> (i % 8)) & 1;
            poly[i] = mask * ((KYBER_Q + 1) / 2);
        }
    }

    std::array<uint8_t, 1408> compress1(const PolyVector& u) {
        std::array<uint8_t, 1408> compressed;
        size_t offset = 0;
        for (const auto& poly : u) {
            for (size_t i = 0; i < KYBER_N / 8; ++i) {
                uint8_t byte = 0;
                for (size_t j = 0; j < 8; ++j) {
                    uint16_t t = ((static_cast<uint32_t>(poly[8*i + j]) << 10) + KYBER_Q / 2) / KYBER_Q;
                    byte |= (t & 1) << j;
                }
                compressed[offset++] = byte;
            }
        }
        return compressed;
    }

    std::array<uint8_t, 160> compress2(const Polynomial& v) {
        std::array<uint8_t, 160> compressed;
        for (size_t i = 0; i < KYBER_N / 8; ++i) {
            uint8_t byte = 0;
            for (size_t j = 0; j < 8; ++j) {
                uint16_t t = ((static_cast<uint32_t>(v[8*i + j]) << 4) + KYBER_Q / 2) / KYBER_Q;
                byte |= (t & 0xF) << (4 * (j % 2));
                if (j % 2 == 1) {
                    compressed[i * 4 + j / 2] = byte;
                    byte = 0;
                }
            }
        }
        return compressed;
    }

    PolyVector decompress1(const std::array<uint8_t, 1408>& data) {
        PolyVector result;
        size_t offset = 0;
        for (auto& poly : result) {
            for (size_t i = 0; i < KYBER_N / 8; ++i) {
                uint8_t byte = data[offset++];
                for (size_t j = 0; j < 8; ++j) {
                    poly[8*i + j] = ((byte >> j) & 1) * ((KYBER_Q + 1) / 2);
                }
            }
        }
        return result;
    }

    Polynomial decompress2(const std::array<uint8_t, 160>& data) {
        Polynomial result;
        for (size_t i = 0; i < KYBER_N / 8; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                uint8_t byte = data[i * 4 + j];
                for (size_t k = 0; k < 2; ++k) {
                    uint8_t val = (byte >> (4 * k)) & 0xF;
                    result[8*i + 2*j + k] = val * ((KYBER_Q + 1) / 16);
                }
            }
        }
        return result;
    }

    Polynomial sample(const std::array<uint8_t, 32>& seed, uint8_t nonce) {
        Polynomial result;
        std::array<uint8_t, 168> buf;
        
        // XOF with seed and nonce
        CSHAKE128().Write(seed.data(), 32).Write(&nonce, 1).Finalize(buf.data(), 168);
        
        size_t pos = 0;
        for (size_t i = 0; i < KYBER_N; ++i) {
            uint16_t val;
            do {
                if (pos >= 168) {
                    // Reseed if needed
                    nonce++;
                    CSHAKE128().Write(seed.data(), 32).Write(&nonce, 1).Finalize(buf.data(), 168);
                    pos = 0;
                }
                val = buf[pos] | (static_cast<uint16_t>(buf[pos + 1]) << 8);
                val &= 0x1FFF;
                pos += 2;
            } while (val >= KYBER_Q);
            result[i] = val;
        }
        return result;
    }

    std::array<PolyVector, KYBER_K> generateMatrixA(const std::array<uint8_t, 32>& seed, bool transposed) {
        std::array<PolyVector, KYBER_K> A;
        
        for (size_t i = 0; i < KYBER_K; ++i) {
            for (size_t j = 0; j < KYBER_K; ++j) {
                size_t row = transposed ? j : i;
                size_t col = transposed ? i : j;
                
                std::array<uint8_t, 34> extended_seed;
                std::copy(seed.begin(), seed.end(), extended_seed.begin());
                extended_seed[32] = static_cast<uint8_t>(col);
                extended_seed[33] = static_cast<uint8_t>(row);
                
                A[i][j] = sample({{extended_seed.data(), 32}}, static_cast<uint8_t>(col + row * KYBER_K));
            }
        }
        return A;
    }
}
