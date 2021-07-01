//
//  WKAddress__SYMBOL__.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "WK__SYMBOL__.h"
#include "__name__/BR__Name__Address.h"


static WKAddress__SYMBOL__
wkAddressCoerce (WKAddress address) {
    assert (WK_NETWORK_TYPE___SYMBOL__ == address->type);
    return (WKAddress__SYMBOL__) address;
}

typedef struct {
    BR__Name__Address __symbol__Address;
} WKAddressCreateContext__SYMBOL__;

static void
wkAddressCreateCallback__SYMBOL__ (WKAddressCreateContext context,
                                WKAddress address) {
    WKAddressCreateContext__SYMBOL__ *context__SYMBOL__ = (WKAddressCreateContext__SYMBOL__*) context;
    WKAddress__SYMBOL__ address__SYMBOL__ = wkAddressCoerce (address);

    address__SYMBOL__->addr = context__SYMBOL__->__symbol__Address;
}

extern WKAddress
wkAddressCreateAs__SYMBOL__ (BR__Name__Address addr) {
    WKAddressCreateContext__SYMBOL__ context__SYMBOL__ = {
        addr
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddress__SYMBOL__Record),
                                      WK_NETWORK_TYPE___SYMBOL__,
                                      __name__AddressHashValue (addr),
                                      &context__SYMBOL__,
                                      wkAddressCreateCallback__SYMBOL__);
}

extern WKAddress
wkAddressCreateFromStringAs__SYMBOL__ (const char *string) {
    assert(string);
    
    BR__Name__Address address = __name__AddressCreateFromString (string, true);
    return (NULL != address
            ? wkAddressCreateAs__SYMBOL__ (address)
            : NULL);
}

private_extern OwnershipKept BR__Name__Address
wkAddressAs__SYMBOL__ (WKAddress address) {
    WKAddress__SYMBOL__ address__SYMBOL__ = wkAddressCoerce (address);
    return address__SYMBOL__->addr;
}

// MARK: - Handlers

static void
wkAddressRelease__SYMBOL__ (WKAddress address) {
    WKAddress__SYMBOL__ address__SYMBOL__ = wkAddressCoerce (address);
    __name__AddressFree (address__SYMBOL__->addr);
}

static char *
wkAddressAsString__SYMBOL__ (WKAddress address) {
    WKAddress__SYMBOL__ address__SYMBOL__ = wkAddressCoerce (address);
    return __name__AddressAsString (address__SYMBOL__->addr);
}

static bool
wkAddressIsEqual__SYMBOL__ (WKAddress address1, WKAddress address2) {
    WKAddress__SYMBOL__ a1 = wkAddressCoerce (address1);
    WKAddress__SYMBOL__ a2 = wkAddressCoerce (address2);

    return __name__AddressEqual (a1->addr, a2->addr);
}

WKAddressHandlers wkAddressHandlers__SYMBOL__ = {
    wkAddressRelease__SYMBOL__,
    wkAddressAsString__SYMBOL__,
    wkAddressIsEqual__SYMBOL__
};
