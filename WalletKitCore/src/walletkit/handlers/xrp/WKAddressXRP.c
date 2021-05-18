//
//  WKAddressXRP.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "WKXRP.h"
#include "ripple/BRRippleAddress.h"


static WKAddressXRP
wkAddressCoerce (WKAddress address) {
    assert (WK_NETWORK_TYPE_XRP == address->type);
    return (WKAddressXRP) address;
}

typedef struct {
    BRRippleAddress xrpAddress;
} WKAddressCreateContextXRP;

static void
wkAddressCreateCallbackXRP (WKAddressCreateContext context,
                                WKAddress address) {
    WKAddressCreateContextXRP *contextXRP = (WKAddressCreateContextXRP*) context;
    WKAddressXRP addressXRP = wkAddressCoerce (address);

    addressXRP->addr = contextXRP->xrpAddress;
}

extern WKAddress
wkAddressCreateAsXRP (OwnershipGiven BRRippleAddress addr) {
    WKAddressCreateContextXRP contextXRP = {
        addr
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddressXRPRecord),
                                      WK_NETWORK_TYPE_XRP,
                                      rippleAddressHashValue(addr),
                                      &contextXRP,
                                      wkAddressCreateCallbackXRP);
}

extern WKAddress
wkAddressCreateFromStringAsXRP (const char *string) {
    assert(string);
    
    BRRippleAddress address = rippleAddressCreateFromString (string, true);
    return (NULL != address
            ? wkAddressCreateAsXRP (address)
            : NULL);
}

private_extern OwnershipKept BRRippleAddress
wkAddressAsXRP (WKAddress address) {
    WKAddressXRP addressXRP = wkAddressCoerce (address);
    return addressXRP->addr;
}

// MARK: - Handlers

static void
wkAddressReleaseXRP (WKAddress address) {
    WKAddressXRP addressXRP = wkAddressCoerce (address);
    rippleAddressFree (addressXRP->addr);
}

static char *
wkAddressAsStringXRP (WKAddress address) {
    WKAddressXRP addressXRP = wkAddressCoerce (address);
    return rippleAddressAsString (addressXRP->addr);
}

static bool
wkAddressIsEqualXRP (WKAddress address1, WKAddress address2) {
    WKAddressXRP a1 = wkAddressCoerce (address1);
    WKAddressXRP a2 = wkAddressCoerce (address2);

    return rippleAddressEqual (a1->addr, a2->addr);
}

WKAddressHandlers wkAddressHandlersXRP = {
    wkAddressReleaseXRP,
    wkAddressAsStringXRP,
    wkAddressIsEqualXRP
};
