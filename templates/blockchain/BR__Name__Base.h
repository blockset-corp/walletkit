//
//  BR__Name__Base.h
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BR__Name__Base_h
#define BR__Name__Base_h

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <memory.h>
#include <assert.h>

#include "support/BRBase.h"
#include "support/BRInt.h"

#if !defined(ASSERT_UNIMPLEMENTED)
#define ASSERT_UNIMPLEMENTED    assert(false);
#endif

#ifdef __cplusplus
extern "C" {
#endif

// The integer amount for __Name__
typedef uint64_t BR__Name__Amount;

#if 0
#define __NAME___AMOUNT_DIGITS          (8)
#define __NAME___AMOUNT_SCALE_FACTOR    (100000000)  // 1
#endif

// MARK: - __Name__ Hash

#define __NAME___HASH_BYTES 34

typedef struct {
    uint8_t bytes[__NAME___HASH_BYTES];
} BR__Name__Hash;

#define __NAME___HASH_EMPTY  ((BR__Name__Hash) { \
    0, 0,                       \
    0, 0, 0, 0,     0, 0, 0, 0, \
    0, 0, 0, 0,     0, 0, 0, 0, \
    0, 0, 0, 0,     0, 0, 0, 0, \
    0, 0, 0, 0,     0, 0, 0, 0, \
})

static inline bool
__name__HashIsEqual (const BR__Name__Hash h1,
                     const BR__Name__Hash h2) {
    return 0 == memcmp (h1.bytes, h2.bytes, __NAME___HASH_BYTES);
}

static inline bool
__name__HashIsEmpty (BR__Name__Hash hash) {
    return __name__HashIsEqual (hash, __NAME___HASH_EMPTY);
}

static inline BR__Name__Hash
__name__HashFromString(const char *input) {
    assert (0);
#if 0
    size_t length = BRBase58CheckDecode(NULL, 0, input);
    assert(length == __NAME___HASH_BYTES);
    BR__Name__Hash hash;
    BRBase58CheckDecode(hash.bytes, length, input);
#endif
    BR__Name__Hash hash = { 0 };
    return hash;
}

static inline char *
__name__HashToString (BR__Name__Hash hash) {
#if 0
    char string[64] = {0};
    BRBase58CheckEncode(string, sizeof(string), hash.bytes, sizeof(hash.bytes));
    return strdup(string);
#endif
    return strdup ("");
}

// For use with BRSet
static inline uint32_t
__name__HashSetValue (const BR__Name__Hash *hash) {
    // First foun bytes as a uint32; unlikely to be sufficient (?)
    return (uint32_t) ((UInt256 *) hash->bytes)->u32[0];
}

#ifdef __cplusplus
}
#endif

#endif /* BR__Name__Base_h */
