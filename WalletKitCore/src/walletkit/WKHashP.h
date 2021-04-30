//
//  WKHashP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 12/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKHashP_h
#define WKHashP_h

#include "WKHash.h"

#ifdef __cplusplus
extern "C" {
#endif

// The number of 
#define WK_HASH_BYTES       (64)

struct WKHashRecord {
    // The value to use for BRSet
    uint32_t setValue;

    // The raw bytes; ordered for 'proper display' (BTC is reversed from UInt256).
    size_t bytesCount;
    uint8_t bytes[WK_HASH_BYTES];
    
    WKNetworkType type;

    WKRef ref;
};

private_extern WKHash
wkHashCreateInternal (uint32_t setValue,
                          size_t   bytesCount,
                          uint8_t *bytes,
                          WKNetworkType type);

private_extern OwnershipGiven char *
wkHashStringAsHex (WKHash hash, bool includePrefix);

#ifdef __cplusplus
}
#endif


#endif /* WKHashP_h */
