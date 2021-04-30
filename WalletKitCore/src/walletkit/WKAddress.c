//
//  WKAddress.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKAddressP.h"
#include "WKNetworkP.h"

#include "WKHandlersP.h"

IMPLEMENT_WK_GIVE_TAKE (WKAddress, wkAddress);

private_extern WKAddress
wkAddressAllocAndInit (size_t sizeInBytes,
                           WKNetworkType type,
                           size_t hashValue,
                           WKAddressCreateContext  createContext,
                           WKAddressCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct WKAddressRecord));

    WKAddress address = calloc (1, sizeInBytes);

    address->type = type;
    address->handlers = wkHandlersLookup(type)->address;
    address->ref = WK_REF_ASSIGN(wkAddressRelease);
    address->sizeInBytes = sizeInBytes;

    address->hashValue = hashValue;

    if (NULL != createCallback) createCallback (createContext, address);

    return address;
}

static void
wkAddressRelease (WKAddress address) {
    address->handlers->release (address);
    memset (address, 0, address->sizeInBytes);
    free (address);
}


private_extern WKNetworkType
wkAddressGetType (WKAddress address) {
    return address->type;
}

private_extern size_t
wkAddressGetHashValue (WKAddress address) {
    return address->hashValue;;
}

private_extern BRSetOf(WKAddress)
wkAddressSetCreate (size_t count) {
    return BRSetNew ((size_t (*) (const void *)) wkAddressGetHashValue,
                     (int (*) (const void *, const void *)) wkAddressIsEqual,
                     count);
}

private_extern void
wkAddressSetRelease (BRSetOf(WKAddress) addresses) {
    BRSetFreeAll (addresses,
                  (void (*) (void *)) wkAddressGive);
}

extern char *
wkAddressAsString (WKAddress address) {
    return address->handlers->asString (address);
}

extern WKBoolean
wkAddressIsIdentical (WKAddress a1,
                          WKAddress a2) {
    return AS_WK_BOOLEAN(wkAddressIsEqual(a1, a2));
}

private_extern int
wkAddressIsEqual (WKAddress a1,
                      WKAddress a2) {
    return (a1 == a2 ||
            (a1->type == a2->type &&
             a1->handlers->isEqual (a1, a2)));
}
