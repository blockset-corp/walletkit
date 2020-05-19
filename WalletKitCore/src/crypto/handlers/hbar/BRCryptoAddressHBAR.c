//
//  BRCryptoAddressHBAR.c
//
//
//  Created by Ehsan Rezaie on 2020-05-19.
//

#include <assert.h>

#include "BRCryptoHBAR.h"
#include "hedera/BRHederaAddress.h"


static BRCryptoAddressHBAR
cryptoAddressCoerce (BRCryptoAddress address) {
    assert (CRYPTO_NETWORK_TYPE_HBAR == address->type);
    return (BRCryptoAddressHBAR) address;
}

extern BRCryptoAddress
cryptoAddressCreateAsHBAR (BRHederaAddress addr) {
    BRCryptoAddress    addressBase = cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressHBARRecord),
                                                                CRYPTO_NETWORK_TYPE_HBAR,
                                                                0); //TODO:HBAR address hash
    BRCryptoAddressHBAR address = cryptoAddressCoerce (addressBase);

    address->addr = addr;

    return addressBase;
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
cryptoAddressAsHBAR (BRCryptoAddress addressBase) {
    BRCryptoAddressHBAR address = (BRCryptoAddressHBAR) addressBase;
    return address->addr;
}

// MARK: - Handlers

static void
cryptoAddressReleaseHBAR (BRCryptoAddress addressBase) {
    BRCryptoAddressHBAR address = (BRCryptoAddressHBAR) addressBase;
    hederaAddressFree (address->addr);
}

static char *
cryptoAddressAsStringHBAR (BRCryptoAddress addressBase) {
    BRCryptoAddressHBAR address = (BRCryptoAddressHBAR) addressBase;
    return hederaAddressAsString (address->addr);
}

static bool
cryptoAddressIsEqualHBAR (BRCryptoAddress address1, BRCryptoAddress address2) {
    BRCryptoAddressHBAR a1 = (BRCryptoAddressHBAR) address1;
    BRCryptoAddressHBAR a2 = (BRCryptoAddressHBAR) address2;

    return hederaAddressEqual (a1->addr, a2->addr);
}

BRCryptoAddressHandlers cryptoAddressHandlersHBAR = {
    cryptoAddressReleaseHBAR,
    cryptoAddressAsStringHBAR,
    cryptoAddressIsEqualHBAR
};
