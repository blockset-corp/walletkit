//
//  WKAddressXLM.c
//  WalletKitCore
//
//  Created by Carl Cherry on 2021-05-19.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "WKXLM.h"
#include "stellar/BRStellarAddress.h"


static WKAddressXLM
wkAddressCoerce (WKAddress address) {
    assert (WK_NETWORK_TYPE_XLM == address->type);
    return (WKAddressXLM) address;
}

typedef struct {
    BRStellarAddress xlmAddress;
} WKAddressCreateContextXLM;

static void
wkAddressCreateCallbackXLM (WKAddressCreateContext context,
                                WKAddress address) {
    WKAddressCreateContextXLM *contextXLM = (WKAddressCreateContextXLM*) context;
    WKAddressXLM addressXLM = wkAddressCoerce (address);

    addressXLM->addr = contextXLM->xlmAddress;
}

extern WKAddress
wkAddressCreateAsXLM (OwnershipGiven BRStellarAddress addr) {
    WKAddressCreateContextXLM contextXLM = {
        addr
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddressXLMRecord),
                                      WK_NETWORK_TYPE_XLM,
                                      stellarAddressHashValue(addr),
                                      &contextXLM,
                                      wkAddressCreateCallbackXLM);
}

extern WKAddress
wkAddressCreateFromStringAsXLM (const char *string) {
    assert(string);
    
    BRStellarAddress address = stellarAddressCreateFromString (string, true);
    return (NULL != address
            ? wkAddressCreateAsXLM (address)
            : NULL);
}

private_extern OwnershipKept BRStellarAddress
wkAddressAsXLM (WKAddress address) {
    WKAddressXLM addressXLM = wkAddressCoerce (address);
    return addressXLM->addr;
}

// MARK: - Handlers

static void
wkAddressReleaseXLM (WKAddress address) {
    WKAddressXLM addressXLM = wkAddressCoerce (address);
    stellarAddressFree (addressXLM->addr);
}

static char *
wkAddressAsStringXLM (WKAddress address) {
    WKAddressXLM addressXLM = wkAddressCoerce (address);
    return stellarAddressAsString (addressXLM->addr);
}

static bool
wkAddressIsEqualXLM (WKAddress address1, WKAddress address2) {
    WKAddressXLM a1 = wkAddressCoerce (address1);
    WKAddressXLM a2 = wkAddressCoerce (address2);

    return stellarAddressEqual (a1->addr, a2->addr);
}

WKAddressHandlers wkAddressHandlersXLM = {
    wkAddressReleaseXLM,
    wkAddressAsStringXLM,
    wkAddressIsEqualXLM
};
