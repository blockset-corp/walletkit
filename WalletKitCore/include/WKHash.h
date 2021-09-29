//
//  WKHash.h
//  WalletKitCore
//
//  Created by Ed Gamble on 5/15/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKHash_h
#define WKHash_h

#include "WKBase.h"

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
typedef struct WKHashRecord *WKHash;

/**
 * Check if two hashes are equal
 */
extern WKBoolean
wkHashEqual (WKHash h1, WKHash h2);

/**
 * Encode hash as a string appropriate for the network
 */
extern OwnershipGiven char *
wkHashEncodeString (WKHash hash);

/**
 * Get the hash's integer representation.  This is used for `BRSet` operations.
 */
extern int
wkHashGetHashValue (WKHash hash);

DECLARE_WK_GIVE_TAKE (WKHash, wkHash);

#ifdef __cplusplus
}
#endif

#endif /* WKHash_h */
