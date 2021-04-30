//
//  WKAddressETH.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKETH.h"
#include "ethereum/base/BREthereumAddress.h"

static WKAddressETH
wkAddressCoerce (WKAddress address) {
    assert (WK_NETWORK_TYPE_ETH == address->type);
    return (WKAddressETH) address;
}

typedef struct {
    BREthereumAddress eth;
} WKAddressCreateContextETH;

static void
wkAddressCreateCallbackETH (WKAddressCreateContext context,
                                WKAddress address) {
    WKAddressCreateContextETH *contextETH = (WKAddressCreateContextETH*) context;
    WKAddressETH addressETH = wkAddressCoerce (address);

    addressETH->eth = contextETH->eth;
}

private_extern WKAddress
wkAddressCreateAsETH (BREthereumAddress eth) {
    WKAddressCreateContextETH contextETH = {
        eth
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddressETHRecord),
                                      WK_NETWORK_TYPE_ETH,
                                      (size_t) ethAddressHashValue (eth),
                                      &contextETH,
                                      wkAddressCreateCallbackETH);
}

private_extern BREthereumAddress
wkAddressAsETH (WKAddress address) {
    WKAddressETH addressETH = wkAddressCoerce (address);
    return addressETH->eth;
}

extern WKAddress
wkAddressCreateFromStringAsETH (const char *ethAddress) {
    assert (ethAddress);
    return wkAddressCreateAsETH (ETHEREUM_BOOLEAN_TRUE == ethAddressValidateString (ethAddress)
                                     ? ethAddressCreate (ethAddress)
                                     : ETHEREUM_EMPTY_ADDRESS_INIT);
}

static void
wkAddressReleaseETH (WKAddress address) {
    WKAddressETH addressETH = wkAddressCoerce (address);
    (void) addressETH;

    return;
}

static char *
wkAddressAsStringETH (WKAddress address) {
    WKAddressETH addressETH = wkAddressCoerce (address);
    return ethAddressGetEncodedString(addressETH->eth, 1);
}

static bool
wkAddressIsEqualETH (WKAddress address1, WKAddress address2) {
    WKAddressETH a1 = wkAddressCoerce (address1);
    WKAddressETH a2 = wkAddressCoerce (address2);

    return ETHEREUM_BOOLEAN_IS_TRUE (ethAddressEqual (a1->eth, a2->eth));
}

WKAddressHandlers wkAddressHandlersETH = {
    wkAddressReleaseETH,
    wkAddressAsStringETH,
    wkAddressIsEqualETH
};

