//
//  WKAddressHBAR.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "WKHBAR.h"
#include "hedera/BRHederaAddress.h"


static WKAddressHBAR
wkAddressCoerce (WKAddress address) {
    assert (WK_NETWORK_TYPE_HBAR == address->type);
    return (WKAddressHBAR) address;
}

typedef struct {
    BRHederaAddress xrpAddress;
} WKAddressCreateContextHBAR;

static void
wkAddressCreateCallbackHBAR (WKAddressCreateContext context,
                                 WKAddress address) {
    WKAddressCreateContextHBAR *contextHBAR = (WKAddressCreateContextHBAR*) context;
    WKAddressHBAR addressHBAR = wkAddressCoerce (address);

    addressHBAR->addr = contextHBAR->xrpAddress;
}

extern WKAddress
wkAddressCreateAsHBAR (BRHederaAddress addr) {
    WKAddressCreateContextHBAR contextHBAR = {
        addr
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddressHBARRecord),
                                      WK_NETWORK_TYPE_HBAR,
                                      0, //TODO:HBAR address hash
                                      &contextHBAR,
                                      wkAddressCreateCallbackHBAR);
}

extern WKAddress
wkAddressCreateFromStringAsHBAR (const char *string) {
    assert(string);
    
    BRHederaAddress address = hederaAddressCreateFromString (string, true);
    return (NULL != address
            ? wkAddressCreateAsHBAR (address)
            : NULL);
}

private_extern BRHederaAddress
wkAddressAsHBAR (WKAddress address) {
    WKAddressHBAR addressHBAR = wkAddressCoerce (address);
    return addressHBAR->addr;
}

// MARK: - Handlers

static void
wkAddressReleaseHBAR (WKAddress address) {
    WKAddressHBAR addressHBAR = wkAddressCoerce (address);
    hederaAddressFree (addressHBAR->addr);
}

static char *
wkAddressAsStringHBAR (WKAddress address) {
    WKAddressHBAR addressHBAR = wkAddressCoerce (address);
    return hederaAddressAsString (addressHBAR->addr);
}

static bool
wkAddressIsEqualHBAR (WKAddress address1, WKAddress address2) {
    WKAddressHBAR a1 = wkAddressCoerce (address1);
    WKAddressHBAR a2 = wkAddressCoerce (address1);

    return hederaAddressEqual (a1->addr, a2->addr);
}

WKAddressHandlers wkAddressHandlersHBAR = {
    wkAddressReleaseHBAR,
    wkAddressAsStringHBAR,
    wkAddressIsEqualHBAR
};
