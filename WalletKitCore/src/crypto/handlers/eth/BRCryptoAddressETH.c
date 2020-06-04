//
//  BRCryptoAddressETH.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoETH.h"
#include "ethereum/base/BREthereumAddress.h"

static BRCryptoAddressETH
cryptoAddressCoerce (BRCryptoAddress address) {
    assert (CRYPTO_NETWORK_TYPE_ETH == address->type);
    return (BRCryptoAddressETH) address;
}

private_extern BRCryptoAddress
cryptoAddressCreateAsETH (BREthereumAddress eth) {
    BRCryptoAddress addressBase = cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressETHRecord),
                                                             CRYPTO_NETWORK_TYPE_ETH,
                                                             (size_t) ethAddressHashValue (eth));
    BRCryptoAddressETH address     = cryptoAddressCoerce (addressBase);
    address->eth = eth;

    return addressBase;
}

private_extern BREthereumAddress
cryptoAddressAsETH (BRCryptoAddress addressBase) {
    BRCryptoAddressETH address     = cryptoAddressCoerce (addressBase);
    return address->eth;
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

