//
//  BRCryptoAddressXTZ.c
//  Core
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "BRCryptoXTZ.h"
#include "tezos/BRTezosAddress.h"


static BRCryptoAddressXTZ
cryptoAddressCoerce (BRCryptoAddress address) {
    assert (CRYPTO_NETWORK_TYPE_XTZ == address->type);
    return (BRCryptoAddressXTZ) address;
}

typedef struct {
    BRTezosAddress xtzAddress;
} BRCryptoAddressCreateContextXTZ;

static void
cryptoAddressCreateCallbackXTZ (BRCryptoAddressCreateContext context,
                                BRCryptoAddress address) {
    BRCryptoAddressCreateContextXTZ *contextXTZ = (BRCryptoAddressCreateContextXTZ*) context;
    BRCryptoAddressXTZ addressXTZ = cryptoAddressCoerce (address);

    addressXTZ->addr = contextXTZ->xtzAddress;
}

extern BRCryptoAddress
cryptoAddressCreateAsXTZ (BRTezosAddress addr) {
    BRCryptoAddressCreateContextXTZ contextXTZ = {
        addr
    };

    return cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressXTZRecord),
                                      CRYPTO_NETWORK_TYPE_XTZ,
                                      tezosAddressHashValue (addr),
                                      &contextXTZ,
                                      cryptoAddressCreateCallbackXTZ);
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsXTZ (const char *string) {
    assert(string);
    
    BRTezosAddress address = tezosAddressCreateFromString (string, true);
    return (NULL != address
            ? cryptoAddressCreateAsXTZ (address)
            : NULL);
}

private_extern OwnershipKept BRTezosAddress
cryptoAddressAsXTZ (BRCryptoAddress address) {
    BRCryptoAddressXTZ addressXTZ = cryptoAddressCoerce (address);
    return addressXTZ->addr;
}

// MARK: - Handlers

static void
cryptoAddressReleaseXTZ (BRCryptoAddress address) {
    BRCryptoAddressXTZ addressXTZ = cryptoAddressCoerce (address);
    tezosAddressFree (addressXTZ->addr);
}

static char *
cryptoAddressAsStringXTZ (BRCryptoAddress address) {
    BRCryptoAddressXTZ addressXTZ = cryptoAddressCoerce (address);
    return tezosAddressAsString (addressXTZ->addr);
}

static bool
cryptoAddressIsEqualXTZ (BRCryptoAddress address1, BRCryptoAddress address2) {
    BRCryptoAddressXTZ a1 = cryptoAddressCoerce (address1);
    BRCryptoAddressXTZ a2 = cryptoAddressCoerce (address2);

    return tezosAddressEqual (a1->addr, a2->addr);
}

BRCryptoAddressHandlers cryptoAddressHandlersXTZ = {
    cryptoAddressReleaseXTZ,
    cryptoAddressAsStringXTZ,
    cryptoAddressIsEqualXTZ
};
