
#include <assert.h>

#include "BRCryptoXRP.h"
#include "ripple/BRRippleAddress.h"


static BRCryptoAddressXRP
cryptoAddressCoerce (BRCryptoAddress address) {
    assert (CRYPTO_NETWORK_TYPE_XRP == address->type);
    return (BRCryptoAddressXRP) address;
}

extern BRCryptoAddress
cryptoAddressCreateAsXRP (BRRippleAddress addr) {
    BRCryptoAddress    addressBase = cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressXRPRecord),
                                                                CRYPTO_NETWORK_TYPE_XRP,
                                                                0); //TODO:XRP address hash
    BRCryptoAddressXRP address     = cryptoAddressCoerce (addressBase);

    address->addr = addr;

    return addressBase;
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsXRP (const char *string) {
    assert(string);
    
    BRRippleAddress address = rippleAddressCreateFromString (string, true);
    return (NULL != address
            ? cryptoAddressCreateAsXRP (address)
            : NULL);
}

private_extern BRRippleAddress
cryptoAddressAsXRP (BRCryptoAddress addressBase) {
    BRCryptoAddressXRP address = (BRCryptoAddressXRP) addressBase;
    return address->addr;
}

// MARK: - Handlers

static void
cryptoAddressReleaseXRP (BRCryptoAddress addressBase) {
    BRCryptoAddressXRP address = (BRCryptoAddressXRP) addressBase;
    rippleAddressFree (address->addr);
}

static char *
cryptoAddressAsStringXRP (BRCryptoAddress addressBase) {
    BRCryptoAddressXRP address = (BRCryptoAddressXRP) addressBase;
    return rippleAddressAsString (address->addr);
}

static bool
cryptoAddressIsEqualXRP (BRCryptoAddress address1, BRCryptoAddress address2) {
    BRCryptoAddressXRP a1 = (BRCryptoAddressXRP) address1;
    BRCryptoAddressXRP a2 = (BRCryptoAddressXRP) address2;

    return rippleAddressEqual (a1->addr, a2->addr);
}

BRCryptoAddressHandlers cryptoAddressHandlersXRP = {
    cryptoAddressReleaseXRP,
    cryptoAddressAsStringXRP,
    cryptoAddressIsEqualXRP
};
