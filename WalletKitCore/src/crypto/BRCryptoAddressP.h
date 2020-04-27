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
};

private_extern BRCryptoAddress
cryptoAddressAllocAndInit (size_t sizeInBytes,
                           BRCryptoBlockChainType type);

private_extern BRCryptoBlockChainType
cryptoAddressGetType (BRCryptoAddress address);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoAddressP_h */
