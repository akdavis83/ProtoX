/*
 * QTC BLAKE3 Implementation Header
 * Quantum-safe mining algorithm for QTC cryptocurrency
 * Based on BLAKE3 reference implementation
 */

#ifndef QTC_CRYPTO_BLAKE3_BLAKE3_H
#define QTC_CRYPTO_BLAKE3_BLAKE3_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BLAKE3_VERSION_STRING "1.5.0"
#define BLAKE3_KEY_LEN 32
#define BLAKE3_OUT_LEN 32
#define BLAKE3_BLOCK_LEN 64
#define BLAKE3_CHUNK_LEN 1024
#define BLAKE3_MAX_DEPTH 54

// BLAKE3 hasher state
typedef struct {
  uint32_t key[8];
  uint64_t counter;
  uint8_t buf[BLAKE3_BLOCK_LEN];
  uint8_t buf_len;
  uint8_t blocks_compressed;
  uint8_t flags;
} blake3_chunk_state;

typedef struct {
  uint32_t key[8];
  blake3_chunk_state chunk;
  uint8_t cv_stack_len;
  // The stack size is MAX_DEPTH + 1 because we do lazy merging. For example,
  // with 7 chunks, we have 3 entries in the stack. Adding an 8th chunk
  // requires a 4th entry, rather than merging everything down. Only when we
  // finalize do we reduce the stack to 1 entry.
  uint8_t cv_stack[(BLAKE3_MAX_DEPTH + 1) * BLAKE3_OUT_LEN];
} blake3_hasher;

// QTC-specific BLAKE3 functions
void blake3_hasher_init(blake3_hasher *self);
void blake3_hasher_init_keyed(blake3_hasher *self, const uint8_t key[BLAKE3_KEY_LEN]);
void blake3_hasher_init_derive_key(blake3_hasher *self, const char *context);
void blake3_hasher_update(blake3_hasher *self, const void *input, size_t input_len);
void blake3_hasher_finalize(const blake3_hasher *self, uint8_t *out, size_t out_len);
void blake3_hasher_finalize_seek(const blake3_hasher *self, uint64_t seek, uint8_t *out, size_t out_len);
void blake3_hasher_reset(blake3_hasher *self);

// Convenience functions for QTC mining
void blake3_hash(const void *input, size_t input_len, uint8_t out[BLAKE3_OUT_LEN]);
void blake3_hash_keyed(const void *input, size_t input_len, const uint8_t key[BLAKE3_KEY_LEN], uint8_t out[BLAKE3_OUT_LEN]);
void blake3_hash_derive_key(const char *context, const void *key_material, size_t key_material_len, uint8_t out[BLAKE3_OUT_LEN]);

// QTC-specific mining optimized functions
void qtc_blake3_hash_block_header(const void *header, size_t header_len, uint8_t out[BLAKE3_OUT_LEN]);
int qtc_blake3_verify_pow(const uint8_t hash[BLAKE3_OUT_LEN], const uint8_t target[BLAKE3_OUT_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* QTC_CRYPTO_BLAKE3_BLAKE3_H */
