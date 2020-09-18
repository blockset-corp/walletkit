//
//  BRCryptoAddressXRP.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "BRCryptoXRP.h"
#include "ripple/BRRippleAddress.h"


static BRCryptoAddressXRP
cryptoAddressCoerce (BRCryptoAddress address) {
    assert (CRYPTO_NETWORK_TYPE_XRP == address->type);
    return (BRCryptoAddressXRP) address;
}

typedef struct {
    BRRippleAddress xrpAddress;
} BRCryptoAddressCreateContextXRP;

static void
cryptoAddressCreateCallbackXRP (BRCryptoAddressCreateContext context,
                                BRCryptoAddress address) {
    BRCryptoAddressCreateContextXRP *contextXRP = (BRCryptoAddressCreateContextXRP*) context;
    BRCryptoAddressXRP addressXRP = cryptoAddressCoerce (address);

    addressXRP->addr = contextXRP->xrpAddress;
}

extern BRCryptoAddress
cryptoAddressCreateAsXRP (OwnershipGiven BRRippleAddress addr) {
    BRCryptoAddressCreateContextXRP contextXRP = {
        addr
    };

    return cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressXRPRecord),
                                      CRYPTO_NETWORK_TYPE_XRP,
                                      rippleAddressHashValue(addr),
                                      &contextXRP,
                                      cryptoAddressCreateCallbackXRP);
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsXRP (const char *string) {
    assert(string);
    
    BRRippleAddress address = rippleAddressCreateFromString (string, true);
    return (NULL != address
            ? cryptoAddressCreateAsXRP (address)
            : NULL);
}

private_extern OwnershipKept BRRippleAddress
cryptoAddressAsXRP (BRCryptoAddress address) {
    BRCryptoAddressXRP addressXRP = cryptoAddressCoerce (address);
    return addressXRP->addr;
}

// MARK: - Handlers

static void
cryptoAddressReleaseXRP (BRCryptoAddress address) {
    BRCryptoAddressXRP addressXRP = cryptoAddressCoerce (address);
    rippleAddressFree (addressXRP->addr);
}

static char *
cryptoAddressAsStringXRP (BRCryptoAddress address) {
    BRCryptoAddressXRP addressXRP = cryptoAddressCoerce (address);
    return rippleAddressAsString (addressXRP->addr);
}

static bool
cryptoAddressIsEqualXRP (BRCryptoAddress address1, BRCryptoAddress address2) {
    BRCryptoAddressXRP a1 = cryptoAddressCoerce (address1);
    BRCryptoAddressXRP a2 = cryptoAddressCoerce (address2);

    return rippleAddressEqual (a1->addr, a2->addr);
}

BRCryptoAddressHandlers cryptoAddressHandlersXRP = {
    cryptoAddressReleaseXRP,
    cryptoAddressAsStringXRP,
    cryptoAddressIsEqualXRP
};
