//
//  BRTezosBase.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-06-17.
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRTezosBase_h
#define BRTezosBase_h

#include "WKBase.h"
#include "support/BRBase58.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


#define TEZOS_PUBLIC_KEY_SIZE 32
#define TEZOS_HASH_BYTES 34

typedef struct {
    uint8_t bytes[TEZOS_HASH_BYTES];
} BRTezosHash;

#define TEZOS_HASH_EMPTY  ((BRTezosHash) { \
    0, 0,                       \
    0, 0, 0, 0,     0, 0, 0, 0, \
    0, 0, 0, 0,     0, 0, 0, 0, \
    0, 0, 0, 0,     0, 0, 0, 0, \
    0, 0, 0, 0,     0, 0, 0, 0, \
})

typedef int64_t BRTezosUnitMutez;

#define TEZOS_TEZ_SCALE_FACTOR       (1000000)  // 1 TEZ = 1e6 MUTEZ
#define TEZOS_TEZ_TO_MUTEZ(x)        ((x) * TEZOS_TEZ_SCALE_FACTOR)

typedef enum {
    TEZOS_OP_ENDORESEMENT = 0,
    TEZOS_OP_REVEAL = 107,
    TEZOS_OP_TRANSACTION = 108,
    TEZOS_OP_DELEGATION = 110
} BRTezosOperationKind;

static inline const char *
tezosOperationKindDescription (BRTezosOperationKind kind) {
    switch (kind) {
        case TEZOS_OP_ENDORESEMENT: return "Endoresement";
        case TEZOS_OP_REVEAL:       return "Reveal";
        case TEZOS_OP_TRANSACTION:  return "Transaction";
        case TEZOS_OP_DELEGATION:   return "Delegation";
    }
}

// MARK: - Tezos Hash

static inline bool
tezosHashIsEqual (const BRTezosHash h1,
                  const BRTezosHash h2) {
    return 0 == memcmp (h1.bytes, h2.bytes, TEZOS_HASH_BYTES);
}

static inline bool
tezosHashIsEmpty (BRTezosHash hash) {
    return tezosHashIsEqual (hash, TEZOS_HASH_EMPTY);
}

static inline BRTezosHash
tezosHashFromString(const char *input) {
    size_t length = BRBase58CheckDecode(NULL, 0, input);
    assert(length == TEZOS_HASH_BYTES);
    BRTezosHash hash;
    BRBase58CheckDecode(hash.bytes, length, input);
    return hash;
}

static inline char *
tezosHashToString (BRTezosHash hash) {
    char string[64] = {0};
    BRBase58CheckEncode(string, sizeof(string), hash.bytes, sizeof(hash.bytes));
    return strdup(string);
}

#ifdef __cplusplus
}
#endif

#endif /* BRTezosBase_h */
