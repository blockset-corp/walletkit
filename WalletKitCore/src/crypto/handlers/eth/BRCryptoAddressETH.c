
#include "BRCryptoETH.h"
#include "ethereum/base/BREthereumAddress.h"

/// A ETH address
struct BRCryptoAddressETHRecord {
    struct BRCryptoAddressRecord base;

    BREthereumAddress eth;
};

static BRCryptoAddress
cryptoAddressCreateAsETH (BREthereumAddress eth) {
    BRCryptoAddress address = cryptoAddressAllocAndInit (CRYPTO_NETWORK_TYPE_ETH,
                                                         sizeof (struct BRCryptoAddressETHRecord));

    ((BRCryptoAddressETH) address)->eth = eth;

    return address;
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsETH (const char *ethAddress) {
    assert (ethAddress);
    return (ETHEREUM_BOOLEAN_TRUE == ethAddressValidateString (ethAddress)
            ? cryptoAddressCreateAsETH (ethAddressCreate (ethAddress))
            : NULL);
}

static void
cryptoAddressReleaseETH (BRCryptoAddress addressBase) {
    return;
}

static char *
cryptoAddressAsStringETH (BRCryptoAddress addressBase) {
    BRCryptoAddressETH address = (BRCryptoAddressETH) addressBase;
    return ethAddressGetEncodedString(address->eth, 1);
}

static bool
cryptoAddressIsEqualETH (BRCryptoAddress addressBase1, BRCryptoAddress addressBase2) {
    BRCryptoAddressETH a1 = (BRCryptoAddressETH) addressBase1;
    BRCryptoAddressETH a2 = (BRCryptoAddressETH) addressBase2;

    return ETHEREUM_BOOLEAN_IS_TRUE (ethAddressEqual (a1->eth, a2->eth));
}

BRCryptoAddressHandlers cryptoAddressHandlersETH = {
    cryptoAddressReleaseETH,
    cryptoAddressAsStringETH,
    cryptoAddressIsEqualETH
};

