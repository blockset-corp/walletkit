//
//  BRCryptoAddressHBAR.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "BRCryptoHBAR.h"
#include "hedera/BRHederaAddress.h"


static BRCryptoAddressHBAR
cryptoAddressCoerce (BRCryptoAddress address) {
    assert (CRYPTO_NETWORK_TYPE_HBAR == address->type);
    return (BRCryptoAddressHBAR) address;
}

typedef struct {
    BRHederaAddress xrpAddress;
} BRCryptoAddressCreateContextHBAR;

static void
cryptoAddressCreateCallbackHBAR (BRCryptoAddressCreateContext context,
                                 BRCryptoAddress address) {
    BRCryptoAddressCreateContextHBAR *contextHBAR = (BRCryptoAddressCreateContextHBAR*) context;
    BRCryptoAddressHBAR addressHBAR = cryptoAddressCoerce (address);

    addressHBAR->addr = contextHBAR->xrpAddress;
}

extern BRCryptoAddress
cryptoAddressCreateAsHBAR (BRHederaAddress addr) {
    BRCryptoAddressCreateContextHBAR contextHBAR = {
        addr
    };

    return cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressHBARRecord),
                                      CRYPTO_NETWORK_TYPE_HBAR,
                                      0, //TODO:HBAR address hash
                                      &contextHBAR,
                                      cryptoAddressCreateCallbackHBAR);
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsHBAR (const char *string) {
    assert(string);
    
    BRHederaAddress address = hederaAddressCreateFromString (string, true);
    return (NULL != address
            ? cryptoAddressCreateAsHBAR (address)
            : NULL);
}

private_extern BRHederaAddress
cryptoAddressAsHBAR (BRCryptoAddress address) {
    BRCryptoAddressHBAR addressHBAR = cryptoAddressCoerce (address);
    return addressHBAR->addr;
}

// MARK: - Handlers

static void
cryptoAddressReleaseHBAR (BRCryptoAddress address) {
    BRCryptoAddressHBAR addressHBAR = cryptoAddressCoerce (address);
    hederaAddressFree (addressHBAR->addr);
}

static char *
cryptoAddressAsStringHBAR (BRCryptoAddress address) {
    BRCryptoAddressHBAR addressHBAR = cryptoAddressCoerce (address);
    return hederaAddressAsString (addressHBAR->addr);
}

static bool
cryptoAddressIsEqualHBAR (BRCryptoAddress address1, BRCryptoAddress address2) {
    BRCryptoAddressHBAR a1 = cryptoAddressCoerce (address1);
    BRCryptoAddressHBAR a2 = cryptoAddressCoerce (address2);

    return hederaAddressEqual (a1->addr, a2->addr);
}

BRCryptoAddressHandlers cryptoAddressHandlersHBAR = {
    cryptoAddressReleaseHBAR,
    cryptoAddressAsStringHBAR,
    cryptoAddressIsEqualHBAR
};
