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
    BRCryptoAddress address = cryptoAddressAllocAndInit (sizeof (struct BRCryptoAddressETHRecord),
                                                         CRYPTO_NETWORK_TYPE_ETH,
                                                         (size_t) ethAddressHashValue (eth));
    BRCryptoAddressETH addressETH = cryptoAddressCoerce (address);
    addressETH->eth = eth;
    
    return address;
}

private_extern BREthereumAddress
cryptoAddressAsETH (BRCryptoAddress address) {
    BRCryptoAddressETH addressETH = cryptoAddressCoerce (address);
    return addressETH->eth;
}

extern BRCryptoAddress
cryptoAddressCreateFromStringAsETH (const char *ethAddress) {
    assert (ethAddress);
    return (ETHEREUM_BOOLEAN_TRUE == ethAddressValidateString (ethAddress)
            ? cryptoAddressCreateAsETH (ethAddressCreate (ethAddress))
            : NULL);
}

static void
cryptoAddressReleaseETH (BRCryptoAddress address) {
    BRCryptoAddressETH addressETH = cryptoAddressCoerce (address);
    (void) addressETH;

    return;
}

static char *
cryptoAddressAsStringETH (BRCryptoAddress address) {
    BRCryptoAddressETH addressETH = cryptoAddressCoerce (address);
    return ethAddressGetEncodedString(addressETH->eth, 1);
}

static bool
cryptoAddressIsEqualETH (BRCryptoAddress address1, BRCryptoAddress address2) {
    BRCryptoAddressETH a1 = cryptoAddressCoerce (address1);
    BRCryptoAddressETH a2 = cryptoAddressCoerce (address2);

    return ETHEREUM_BOOLEAN_IS_TRUE (ethAddressEqual (a1->eth, a2->eth));
}

BRCryptoAddressHandlers cryptoAddressHandlersETH = {
    cryptoAddressReleaseETH,
    cryptoAddressAsStringETH,
    cryptoAddressIsEqualETH
};

