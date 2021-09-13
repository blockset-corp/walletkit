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
    BRAvalancheNetwork avaxNetwork;
} WKAddressCreateContextAVAX;

static void
wkAddressCreateCallbackAVAX (WKAddressCreateContext context,
                                WKAddress address) {
    WKAddressCreateContextAVAX *contextAVAX = (WKAddressCreateContextAVAX*) context;
    WKAddressAVAX addressAVAX = wkAddressCoerceAVAX (address);

    addressAVAX->avaxAddress    = contextAVAX->avaxAddress;
    addressAVAX->avaxNetwork = contextAVAX->avaxNetwork;
}

extern WKAddress
wkAddressCreateAsAVAX (BRAvalancheAddress avaxAddress,
                       BRAvalancheNetwork avaxNetwork) {
    WKAddressCreateContextAVAX contextAVAX = {
        avaxAddress,
        avaxNetwork
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddressAVAXRecord),
                                      WK_NETWORK_TYPE_AVAX,
                                      avalancheAddressHashValue (avaxAddress),
                                      &contextAVAX,
                                      wkAddressCreateCallbackAVAX);
}

private_extern OwnershipKept BRAvalancheAddress
wkAddressAsAVAX (WKAddress address) {
    WKAddressAVAX addressAVAX = wkAddressCoerceAVAX (address);
    return addressAVAX->avaxAddress;
}

// MARK: - Handlers

static void
wkAddressReleaseAVAX (WKAddress address) {
    WKAddressAVAX addressAVAX = wkAddressCoerceAVAX (address);
    (void) addressAVAX;
}

static char *
wkAddressAsStringAVAX (WKAddress address) {
    WKAddressAVAX addressAVAX = wkAddressCoerceAVAX (address);
    return avalancheNetworkAddressToString(addressAVAX->avaxNetwork,
                                           addressAVAX->avaxAddress);
}

static bool
wkAddressIsEqualAVAX (WKAddress address1, WKAddress address2) {
    WKAddressAVAX a1 = wkAddressCoerceAVAX (address1);
    WKAddressAVAX a2 = wkAddressCoerceAVAX (address2);

    return avalancheAddressEqual (a1->avaxAddress, a2->avaxAddress);
}

WKAddressHandlers wkAddressHandlersAVAX = {
    wkAddressReleaseAVAX,
    wkAddressAsStringAVAX,
    wkAddressIsEqualAVAX
};
