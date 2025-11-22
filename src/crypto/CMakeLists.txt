# Copyright (c) 2023-present The Quantum Coin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or https://opensource.org/license/mit/.

add_library(qtc_crypto STATIC EXCLUDE_FROM_ALL
  aes.cpp
  blake3/blake3.cpp
  chacha20.cpp
  chacha20poly1305.cpp
  hex_base.cpp
  hkdf_sha256_32.cpp
  hkdf_sha3_512.cpp
  hmac_sha256.cpp
  hmac_sha512.cpp
  hmac_sha3_512.cpp
  muhash.cpp
  poly1305.cpp
  ripemd160.cpp
  sha1.cpp
  sha256.cpp
  sha256_sse4.cpp
  sha3.cpp
  sha512.cpp
  siphash.cpp
  ../support/cleanse.cpp
)

target_link_libraries(qtc_crypto
  PRIVATE
    core_interface
)

if(HAVE_SSE41)
  target_compile_definitions(qtc_crypto PRIVATE ENABLE_SSE41)
  target_sources(qtc_crypto PRIVATE sha256_sse41.cpp)
  set_property(SOURCE sha256_sse41.cpp PROPERTY
    COMPILE_OPTIONS ${SSE41_CXXFLAGS}
  )
endif()

if(HAVE_AVX2)
  target_compile_definitions(qtc_crypto PRIVATE ENABLE_AVX2)
  target_sources(qtc_crypto PRIVATE sha256_avx2.cpp)
  set_property(SOURCE sha256_avx2.cpp PROPERTY
    COMPILE_OPTIONS ${AVX2_CXXFLAGS}
  )
endif()

if(HAVE_SSE41 AND HAVE_X86_SHANI)
  target_compile_definitions(qtc_crypto PRIVATE ENABLE_SSE41 ENABLE_X86_SHANI)
  target_sources(qtc_crypto PRIVATE sha256_x86_shani.cpp)
  set_property(SOURCE sha256_x86_shani.cpp PROPERTY
    COMPILE_OPTIONS ${SSE41_CXXFLAGS} ${X86_SHANI_CXXFLAGS}
  )
endif()

if(HAVE_ARM_SHANI)
  target_compile_definitions(qtc_crypto PRIVATE ENABLE_ARM_SHANI)
  target_sources(qtc_crypto PRIVATE sha256_arm_shani.cpp)
  set_property(SOURCE sha256_arm_shani.cpp PROPERTY
    COMPILE_OPTIONS ${ARM_SHANI_CXXFLAGS}
  )
endif()

# Add quantum cryptography sources to qtc_crypto library


# QTC Quantum Cryptography
target_sources(qtc_crypto PRIVATE
  kyber/kyber1024.cpp
  dilithium/dilithium3.cpp
)

# Note: Argon2 was previously used for mining but has been removed from Phase 3.
# No external dependency for Argon2 is required anymore.
