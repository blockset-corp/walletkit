//
//  BRCryptoHash.h
//  WalletKitCore
//
//  Created by Ed Gamble on 5/15/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoHash_h
#define BRCryptoHash_h

#include "BRCryptoBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A hash represents a cryptographic hash, which is a series of bytes, for a network.
 *
 * @discussion Different cryptocurrency blockchains represent hash values in different ways,
 * specifically with a different number of byts and often computed using a different cryptographic
 * hash functions.
 */
typedef struct BRCryptoHashRecord *BRCryptoHash;

/**
 * Check if two hashes are equal
 */
extern BRCryptoBoolean
cryptoHashEqual (BRCryptoHash h1, BRCryptoHash h2);

/**
 * Encode hash as a string appropriate for the network
 */
extern OwnershipGiven char *
cryptoHashEncodeString (BRCryptoHash hash);

/**
 * Get the hash's integer representation.  This is used for `BRSet` operations.
 */
extern int
cryptoHashGetHashValue (BRCryptoHash hash);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoHash, cryptoHash);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoHash_h */
