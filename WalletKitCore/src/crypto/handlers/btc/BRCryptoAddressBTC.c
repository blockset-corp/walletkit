
#include <assert.h>

#include "BRCryptoBTC.h"
#include "support/BRAddress.h"
#include "bcash/BRBCashAddr.h"

static BRCryptoAddressBTC
cryptoAddressCoerce (BRCryptoAddress address) {
    assert (CRYPTO_NETWORK_TYPE_BTC == address->type);
    return (BRCryptoAddressBTC) address;
}

extern BRCryptoAddress
cryptoAddressCreateAsBTC (BRCryptoBlockChainType type, BRAddress addr) {
    BRCryptoAddress    addressBase = cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressBTCRecord),
                                                                type,
                                                                BRAddressHash (addr.s));
    BRCryptoAddressBTC address     = cryptoAddressCoerce (addressBase);

    address->addr = addr;

    return addressBase;
}


extern BRCryptoAddress
cryptoAddressCreateFromStringAsBTC (BRAddressParams params, const char *btcAddress) {
    assert (btcAddress);

    return (BRAddressIsValid (params, btcAddress)
            ? cryptoAddressCreateAsBTC (CRYPTO_NETWORK_TYPE_BTC,
                                        BRAddressFill(params, btcAddress))
            : NULL);
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBCH (BRAddressParams params, const char *bchAddress) {
    assert (bchAddress);

    char btcAddr[36];
    return (0 != BRBCashAddrDecode(btcAddr, bchAddress) && !BRAddressIsValid(params, bchAddress)
            ? cryptoAddressCreateAsBTC (CRYPTO_NETWORK_TYPE_BCH,
                                        BRAddressFill(params, btcAddr))
            : NULL);
}

static void
cryptoAddressReleaseBTC (BRCryptoAddress addressBase) {
}

static char *
cryptoAddressAsStringBTC (BRCryptoAddress addressBase) {
    BRCryptoAddressBTC address = (BRCryptoAddressBTC) addressBase;

    switch (address->base.type) {
        case CRYPTO_NETWORK_TYPE_BTC:
            return strdup (address->addr.s);

        case CRYPTO_NETWORK_TYPE_BCH: {
            char *result = malloc (55);
            BRBCashAddrEncode(result, address->addr.s);
            return result;
        }

        default:
            assert (0);
            return NULL;
    }
}

static bool
cryptoAddressIsEqualBTC (BRCryptoAddress address1, BRCryptoAddress address2) {
    BRCryptoAddressBTC a1 = (BRCryptoAddressBTC) address1;
    BRCryptoAddressBTC a2 = (BRCryptoAddressBTC) address2;

    return (a1->base.type == a1->base.type &&
            0 == strcmp (a1->addr.s, a2->addr.s));
}

private_extern BRAddress
cryptoAddressAsBTC (BRCryptoAddress addressBase,
                    BRCryptoBoolean *isBitcoinAddr) {
    BRCryptoAddressBTC address = (BRCryptoAddressBTC) addressBase;

    assert (NULL != isBitcoinAddr);
    *isBitcoinAddr = AS_CRYPTO_BOOLEAN (addressBase->type == CRYPTO_NETWORK_TYPE_BTC);
    return address->addr;
}

BRCryptoAddressHandlers cryptoAddressHandlersBTC = {
    cryptoAddressReleaseBTC,
    cryptoAddressAsStringBTC,
    cryptoAddressIsEqualBTC
};
