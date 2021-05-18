//
//  WKAddressP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKAddressP_h
#define WKAddressP_h

#include <stdbool.h>

#include "support/BRSet.h"

#include "WKBaseP.h"
#include "WKAddress.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void
(*WKAddressReleaseHandler) (WKAddress address);

typedef char *
(*WKAddressAsStringHandler) (WKAddress address);

typedef bool
(*WKAddressIsEqualHandler) (WKAddress address1,
                                  WKAddress address2);

typedef struct {
    WKAddressReleaseHandler release;
    WKAddressAsStringHandler asString;
    WKAddressIsEqualHandler isEqual;
} WKAddressHandlers;

struct WKAddressRecord {
    WKNetworkType type;
    const WKAddressHandlers *handlers;
    WKRef ref;
    size_t sizeInBytes;

    size_t hashValue;
};

typedef void *WKAddressCreateContext;
typedef void (*WKAddressCreateCallback) (WKAddressCreateContext context,
                                               WKAddress address);

private_extern WKAddress
wkAddressAllocAndInit (size_t sizeInBytes,
                           WKNetworkType type,
                           size_t hashValue,
                           WKAddressCreateContext  createContext,
                           WKAddressCreateCallback createCallback);

private_extern WKNetworkType
wkAddressGetType (WKAddress address);

private_extern size_t
wkAddressGetHashValue (WKAddress address);

private_extern int
wkAddressIsEqual (WKAddress a1,
                      WKAddress a2);

private_extern BRSetOf(WKAddress)
wkAddressSetCreate (size_t count);

private_extern void
wkAddressSetRelease (BRSetOf(WKAddress) addresses);

#ifdef __cplusplus
}
#endif

#endif /* WKAddressP_h */
