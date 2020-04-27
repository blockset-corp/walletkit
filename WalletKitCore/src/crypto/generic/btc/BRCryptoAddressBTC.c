
#include <assert.h>

#include "BRCryptoBTC.h"
#include "support/BRAddress.h"
#include "bcash/BRBCashAddr.h"

/// A BTC or BCH address
struct BRCryptoAddressBTCRecord {
    struct BRCryptoAddressRecord base;

    // `true` if BTC; `false` if `BCH`
    BRCryptoBoolean isBitcoinAddr;

    /// The 'bitcoin/' address.  For BTC, addr.s is the string; for BCH, addr.s is
    /// encoded in a 'BCH' specific way.
    BRAddress addr;
};

static BRCryptoAddressBTC
cryptoAddressCoerce (BRCryptoAddress address) {
    assert (CRYPTO_NETWORK_TYPE_BTC == address->type);
    return (BRCryptoAddressBTC) address;
}

extern BRCryptoAddress
cryptoAddressCreateAsBTC (BRCryptoBlockChainType type, BRAddress addr, BRCryptoBoolean isBitcoinAddr) {
    BRCryptoAddress    addressBase = cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressBTCRecord), type);
    BRCryptoAddressBTC address     = cryptoAddressCoerce (addressBase);

    address->isBitcoinAddr = isBitcoinAddr;
    address->addr = addr;

    return addressBase;
}


extern BRCryptoAddress
cryptoAddressCreateFromStringAsBTC (BRAddressParams params, const char *btcAddress) {
    assert (btcAddress);

    return (BRAddressIsValid (params, btcAddress)
            ? cryptoAddressCreateAsBTC (CRYPTO_NETWORK_TYPE_BTC,
                                        BRAddressFill(params, btcAddress),
                                        CRYPTO_TRUE)
            : NULL);
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBCH (BRAddressParams params, const char *bchAddress) {
    assert (bchAddress);

    char btcAddr[36];
    return (0 != BRBCashAddrDecode(btcAddr, bchAddress) && !BRAddressIsValid(params, bchAddress)
            ? cryptoAddressCreateAsBTC (CRYPTO_NETWORK_TYPE_BCH,
                                        BRAddressFill(params, btcAddr),
                                        CRYPTO_FALSE)
            : NULL);
}

static void
cryptoAddressReleaseBTC (BRCryptoAddress addressBase) {
}

static char *
cryptoAddressAsStringBTC (BRCryptoAddress addressBase) {
    BRCryptoAddressBTC address = (BRCryptoAddressBTC) addressBase;

    if (CRYPTO_TRUE == address->isBitcoinAddr)
        return strdup (address->addr.s);
    else {
        char *result = malloc (55);
        BRBCashAddrEncode(result, address->addr.s);
        return result;
    }
}

static bool
cryptoAddressIsEqualBTC (BRCryptoAddress address1, BRCryptoAddress address2) {
    BRCryptoAddressBTC a1 = (BRCryptoAddressBTC) address1;
    BRCryptoAddressBTC a2 = (BRCryptoAddressBTC) address2;

    return (0 == strcmp (a1->addr.s, a2->addr.s) &&
            a1->isBitcoinAddr == a2->isBitcoinAddr);
}

private_extern BRAddress
cryptoAddressAsBTC (BRCryptoAddress addressBase,
                    BRCryptoBoolean *isBitcoinAddr) {
    BRCryptoAddressBTC address = (BRCryptoAddressBTC) addressBase;

    assert (NULL != isBitcoinAddr);
    *isBitcoinAddr = address->isBitcoinAddr;
    return address->addr;
}

BRCryptoAddressHandlers cryptoAddressHandlersBTC = {
    cryptoAddressReleaseBTC,
    cryptoAddressAsStringBTC,
    cryptoAddressIsEqualBTC
};
