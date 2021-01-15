//
//  BRCryptoHashP.h
//  BRCore
//
//  Created by Ed Gamble on 12/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoHashP_h
#define BRCryptoHashP_h

#include "BRCryptoHash.h"

#ifdef __cplusplus
extern "C" {
#endif

// The number of 
#define CRYPTO_HASH_BYTES       (64)

struct BRCryptoHashRecord {
    // The value to use for BRSet
    uint32_t setValue;

    // The raw bytes; ordered for 'proper display' (BTC is reversed from UInt256).
    size_t bytesCount;
    uint8_t bytes[CRYPTO_HASH_BYTES];
    
    BRCryptoBlockChainType type;

    BRCryptoRef ref;
};

private_extern BRCryptoHash
cryptoHashCreateInternal (uint32_t setValue,
                          size_t   bytesCount,
                          uint8_t *bytes,
                          BRCryptoBlockChainType type);

private_extern OwnershipGiven char *
cryptoHashStringAsHex (BRCryptoHash hash, bool includePrefix);

#ifdef __cplusplus
}
#endif


#endif /* BRCryptoHashP_h */
