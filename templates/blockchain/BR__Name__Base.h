//
//  BR__Name__Base.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BR__Name__Base_h
#define BR__Name__Base_h

#include "WKBase.h"
#include "support/BRBase58.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#define __NAME___PUBLIC_KEY_SIZE 32
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

typedef int64_t BR__Name__UnitMutez;

#define __NAME___TEZ_SCALE_FACTOR       (1000000)  // 1 TEZ = 1e6 MUTEZ
#define __NAME___TEZ_TO_MUTEZ(x)        ((x) * __NAME___TEZ_SCALE_FACTOR)

typedef enum {
    __NAME___OP_ENDORESEMENT = 0,
    __NAME___OP_REVEAL = 107,
    __NAME___OP_TRANSACTION = 108,
    __NAME___OP_DELEGATION = 110
} BR__Name__OperationKind;

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
    size_t length = BRBase58CheckDecode(NULL, 0, input);
    assert(length == __NAME___HASH_BYTES);
    BR__Name__Hash hash;
    BRBase58CheckDecode(hash.bytes, length, input);
    return hash;
}

static inline char *
__name__HashToString (BR__Name__Hash hash) {
    char string[64] = {0};
    BRBase58CheckEncode(string, sizeof(string), hash.bytes, sizeof(hash.bytes));
    return strdup(string);
}

#ifdef __cplusplus
}
#endif

#endif /* BR__Name__Base_h */
