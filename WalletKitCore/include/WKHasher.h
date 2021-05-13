//
//  WKHasher.h
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKHasher_h
#define WKHasher_h

#include "WKBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An enumeration of the possible types of cryptographic hash functions
 */
typedef enum {
    WK_HASHER_SHA1,
    WK_HASHER_SHA224,
    WK_HASHER_SHA256,
    WK_HASHER_SHA256_2,
    WK_HASHER_SHA384,
    WK_HASHER_SHA512,
    WK_HASHER_SHA3,
    WK_HASHER_RMD160,
    WK_HASHER_HASH160,
    WK_HASHER_KECCAK256,
    WK_HASHER_MD5
} WKHasherType;

/**
 * @brief A hasher represents a cryptographic hash function.
 */
typedef struct WKHasherRecord *WKHasher;

/**
 * Create a hasher from a hasher type.
 */
extern WKHasher
wkHasherCreate(WKHasherType type);

/*
 * Get the hasher's length in bytes.  This is the number of byes produced when the hasher is
 * applied.
 */
extern size_t
wkHasherLength (WKHasher hasher);

/**
 * Fill `dst` with the result of applying `hasher` to `src`.  Returns WK_TRUE is successful.
 * Typically the application would be unsucessfull if the `dstLen` is not at least as large
 * as the `wkHasherLength()` result.
 */
extern WKBoolean
wkHasherHash (WKHasher hasher,
              uint8_t *dst,
              size_t dstLen,
              const uint8_t *src,
              size_t srcLen);

DECLARE_WK_GIVE_TAKE (WKHasher, wkHasher);

#ifdef __cplusplus
}
#endif

#endif /* WKHasher_h */
