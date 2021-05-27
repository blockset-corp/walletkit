//
//  BRCryptoAddressP.h
//  BRCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoAddressP_h
#define BRCryptoAddressP_h

#include <stdbool.h>

#include "support/BRSet.h"

#include "BRCryptoBaseP.h"
#include "BRCryptoAddress.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void
(*BRCryptoAddressReleaseHandler) (BRCryptoAddress address);

typedef char *
(*BRCryptoAddressAsStringHandler) (BRCryptoAddress address);

typedef bool
(*BRCryptoAddressIsEqualHandler) (BRCryptoAddress address1,
                                  BRCryptoAddress address2);

typedef struct {
    BRCryptoAddressReleaseHandler release;
    BRCryptoAddressAsStringHandler asString;
    BRCryptoAddressIsEqualHandler isEqual;
} BRCryptoAddressHandlers;

struct BRCryptoAddressRecord {
    BRCryptoBlockChainType type;
    const BRCryptoAddressHandlers *handlers;
    BRCryptoRef ref;
    size_t sizeInBytes;

    size_t hashValue;
};

typedef void *BRCryptoAddressCreateContext;
typedef void (*BRCryptoAddressCreateCallback) (BRCryptoAddressCreateContext context,
                                               BRCryptoAddress address);

private_extern BRCryptoAddress
cryptoAddressAllocAndInit (size_t sizeInBytes,
                           BRCryptoBlockChainType type,
                           size_t hashValue,
                           BRCryptoAddressCreateContext  createContext,
                           BRCryptoAddressCreateCallback createCallback);

private_extern BRCryptoBlockChainType
cryptoAddressGetType (BRCryptoAddress address);

private_extern size_t
cryptoAddressGetHashValue (BRCryptoAddress address);

private_extern int
cryptoAddressIsEqual (BRCryptoAddress a1,
                      BRCryptoAddress a2);

private_extern BRSetOf(BRCryptoAddress)
cryptoAddressSetCreate (size_t count);

private_extern void
cryptoAddressSetRelease (BRSetOf(BRCryptoAddress) addresses);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoAddressP_h */
