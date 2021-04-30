//
//  WKAddressXTZ.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "WKXTZ.h"
#include "tezos/BRTezosAddress.h"


static WKAddressXTZ
wkAddressCoerce (WKAddress address) {
    assert (WK_NETWORK_TYPE_XTZ == address->type);
    return (WKAddressXTZ) address;
}

typedef struct {
    BRTezosAddress xtzAddress;
} WKAddressCreateContextXTZ;

static void
wkAddressCreateCallbackXTZ (WKAddressCreateContext context,
                                WKAddress address) {
    WKAddressCreateContextXTZ *contextXTZ = (WKAddressCreateContextXTZ*) context;
    WKAddressXTZ addressXTZ = wkAddressCoerce (address);

    addressXTZ->addr = contextXTZ->xtzAddress;
}

extern WKAddress
wkAddressCreateAsXTZ (BRTezosAddress addr) {
    WKAddressCreateContextXTZ contextXTZ = {
        addr
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddressXTZRecord),
                                      WK_NETWORK_TYPE_XTZ,
                                      tezosAddressHashValue (addr),
                                      &contextXTZ,
                                      wkAddressCreateCallbackXTZ);
}

extern WKAddress
wkAddressCreateFromStringAsXTZ (const char *string) {
    assert(string);
    
    BRTezosAddress address = tezosAddressCreateFromString (string, true);
    return (NULL != address
            ? wkAddressCreateAsXTZ (address)
            : NULL);
}

private_extern OwnershipKept BRTezosAddress
wkAddressAsXTZ (WKAddress address) {
    WKAddressXTZ addressXTZ = wkAddressCoerce (address);
    return addressXTZ->addr;
}

// MARK: - Handlers

static void
wkAddressReleaseXTZ (WKAddress address) {
    WKAddressXTZ addressXTZ = wkAddressCoerce (address);
    tezosAddressFree (addressXTZ->addr);
}

static char *
wkAddressAsStringXTZ (WKAddress address) {
    WKAddressXTZ addressXTZ = wkAddressCoerce (address);
    return tezosAddressAsString (addressXTZ->addr);
}

static bool
wkAddressIsEqualXTZ (WKAddress address1, WKAddress address2) {
    WKAddressXTZ a1 = wkAddressCoerce (address1);
    WKAddressXTZ a2 = wkAddressCoerce (address2);

    return tezosAddressEqual (a1->addr, a2->addr);
}

WKAddressHandlers wkAddressHandlersXTZ = {
    wkAddressReleaseXTZ,
    wkAddressAsStringXTZ,
    wkAddressIsEqualXTZ
};
