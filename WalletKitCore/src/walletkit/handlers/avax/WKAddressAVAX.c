//
//  WKAddressAVAX.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "WKAVAX.h"
#include "avalanche/BRAvalancheAddress.h"

typedef struct {
    BRAvalancheAddress avaxAddress;
} WKAddressCreateContextAVAX;

static void
wkAddressCreateCallbackAVAX (WKAddressCreateContext context,
                                WKAddress address) {
    WKAddressCreateContextAVAX *contextAVAX = (WKAddressCreateContextAVAX*) context;
    WKAddressAVAX addressAVAX = wkAddressCoerceAVAX (address);

    addressAVAX->addr = contextAVAX->avaxAddress;
}

extern WKAddress
wkAddressCreateAsAVAX (BRAvalancheAddress addr) {
    WKAddressCreateContextAVAX contextAVAX = {
        addr
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddressAVAXRecord),
                                      WK_NETWORK_TYPE_AVAX,
                                      avalancheAddressHashValue (addr),
                                      &contextAVAX,
                                      wkAddressCreateCallbackAVAX);
}

extern WKAddress
wkAddressCreateFromStringAsAVAX (const char *string) {
    assert(string);

    return wkAddressCreateAsAVAX (avalancheAddressCreateFromString (string, true, AVALANCHE_CHAIN_TYPE_X));
}

private_extern OwnershipKept BRAvalancheAddress
wkAddressAsAVAX (WKAddress address) {
    WKAddressAVAX addressAVAX = wkAddressCoerceAVAX (address);
    return addressAVAX->addr;
}

// MARK: - Handlers

static void
wkAddressReleaseAVAX (WKAddress address) {
    WKAddressAVAX addressAVAX = wkAddressCoerceAVAX (address);
}

static char *
wkAddressAsStringAVAX (WKAddress address) {
    WKAddressAVAX addressAVAX = wkAddressCoerceAVAX (address);
    return avalancheAddressAsString (addressAVAX->addr);
}

static bool
wkAddressIsEqualAVAX (WKAddress address1, WKAddress address2) {
    WKAddressAVAX a1 = wkAddressCoerceAVAX (address1);
    WKAddressAVAX a2 = wkAddressCoerceAVAX (address2);

    return avalancheAddressEqual (a1->addr, a2->addr);
}

WKAddressHandlers wkAddressHandlersAVAX = {
    wkAddressReleaseAVAX,
    wkAddressAsStringAVAX,
    wkAddressIsEqualAVAX
};
