//
//  BRCryptoHasher.h
//  WalletKitCore
//
//  Created by Michael Carrara on 9/23/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoHasher_h
#define BRCryptoHasher_h

#include "BRCryptoBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An enumeration of the possible types of cryptographic hash functions
 */
typedef enum {
    CRYPTO_HASHER_SHA1,
    CRYPTO_HASHER_SHA224,
    CRYPTO_HASHER_SHA256,
    CRYPTO_HASHER_SHA256_2,
    CRYPTO_HASHER_SHA384,
    CRYPTO_HASHER_SHA512,
    CRYPTO_HASHER_SHA3,
    CRYPTO_HASHER_RMD160,
    CRYPTO_HASHER_HASH160,
    CRYPTO_HASHER_KECCAK256,
    CRYPTO_HASHER_MD5
} BRCryptoHasherType;

/**
 * @brief A hasher represents a cryptographic hash function.
 */
typedef struct BRCryptoHasherRecord *BRCryptoHasher;

/**
 * Create a hasher from a hasher type.
 */
extern BRCryptoHasher
cryptoHasherCreate(BRCryptoHasherType type);

/*
 * Get the hasher's length in bytes.  This is the number of byes produced when the hasher is
 * applied.
 */
extern size_t
cryptoHasherLength (BRCryptoHasher hasher);

/**
 * Fill `dst` with the result of applying `hasher` to `src`.  Returns CRYPTO_TRUE is successful.
 * Typically the application would be unsucessfull if the `dstLen` is not at least as large
 * as the `cryptoHasherLength()` result.
 */
extern BRCryptoBoolean
cryptoHasherHash (BRCryptoHasher hasher,
                  uint8_t *dst,
                  size_t dstLen,
                  const uint8_t *src,
                  size_t srcLen);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoHasher, cryptoHasher);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoHasher_h */
