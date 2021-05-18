//
//  WKAddressBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include <assert.h>

#include "WKBTC.h"
#include "support/BRAddress.h"
#include "bcash/BRBCashAddr.h"

static WKAddressBTC
wkAddressCoerce (WKAddress address, WKNetworkType type) {
    assert (type == address->type);
    return (WKAddressBTC) address;
}

static WKAddressBTC
wkAddressCoerceANY (WKAddress address) {
    assert (WK_NETWORK_TYPE_BTC == address->type ||
            WK_NETWORK_TYPE_BCH == address->type ||
            WK_NETWORK_TYPE_BSV == address->type);
    return (WKAddressBTC) address;
}

typedef struct {
    BRAddress addr;
} WKAddressCreateContextBTC;

static void
wkAddressCreateCallbackBTC (WKAddressCreateContext context,
                                WKAddress address) {
    WKAddressCreateContextBTC *contextBTC = (WKAddressCreateContextBTC*) context;
    WKAddressBTC addressBTC = wkAddressCoerceANY (address);

    addressBTC->addr = contextBTC->addr;
}

extern WKAddress
wkAddressCreateAsBTC (WKNetworkType type, BRAddress addr) {
    WKAddressCreateContextBTC contextBTC = {
        addr
    };

    return wkAddressAllocAndInit (sizeof (struct WKAddressBTCRecord),
                                      type,
                                      BRAddressHash (addr.s),
                                      &contextBTC,
                                      wkAddressCreateCallbackBTC);
}

extern WKAddress
wkAddressCreateFromStringAsBTC (BRAddressParams params, const char *btcAddress) {
    assert (btcAddress);

    return (BRAddressIsValid (params, btcAddress)
            ? wkAddressCreateAsBTC (WK_NETWORK_TYPE_BTC,
                                        BRAddressFill(params, btcAddress))
            : NULL);
}

extern WKAddress
wkAddressCreateFromStringAsBCH (BRAddressParams params, const char *bchAddress) {
    assert (bchAddress);

    char btcAddr[36];
    return (0 != bchAddrDecode(btcAddr, bchAddress) && !BRAddressIsValid(params, bchAddress)
            ? wkAddressCreateAsBTC (WK_NETWORK_TYPE_BCH,
                                        BRAddressFill(params, btcAddr))
            : NULL);
}

private_extern WKAddress
wkAddressCreateFromLegacyStringAsBCH (BRAddressParams params, const char *btcAddr) {
    assert (btcAddr);

    return (BRAddressIsValid (params, btcAddr)
            ? wkAddressCreateAsBTC (WK_NETWORK_TYPE_BCH,
                                        BRAddressFill(params, btcAddr))
            : NULL);
}

extern WKAddress
wkAddressCreateFromStringAsBSV (BRAddressParams params, const char *bsvAddress) {
    assert (bsvAddress);

    return (BRAddressIsValid (params, bsvAddress)
            ? wkAddressCreateAsBTC (WK_NETWORK_TYPE_BSV,
                                        BRAddressFill(params, bsvAddress))
            : NULL);
}

static void
wkAddressReleaseBTC (WKAddress address) {
    WKAddressBTC addressANY = wkAddressCoerceANY (address);
    (void) addressANY;
}

static char *
wkAddressAsStringBTC (WKAddress address) {
    WKAddressBTC addressBTC = wkAddressCoerce (address, WK_NETWORK_TYPE_BTC);
    return strdup (addressBTC->addr.s);
}

static char *
wkAddressAsStringBCH (WKAddress address) {
    WKAddressBTC addressBCH = wkAddressCoerce (address, WK_NETWORK_TYPE_BCH);

    char *result = malloc (55);
    bchAddrEncode(result, addressBCH->addr.s);
    return result;
}

static char *
wkAddressAsStringBSV (WKAddress address) {
    WKAddressBTC addressBSV = wkAddressCoerce (address, WK_NETWORK_TYPE_BSV);
    return strdup (addressBSV->addr.s);
}


static bool
wkAddressIsEqualBTC (WKAddress address1, WKAddress address2) {
    WKAddressBTC a1 = wkAddressCoerce (address1, address1->type);
    WKAddressBTC a2 = wkAddressCoerce (address2, address2->type);

    return (a1->base.type == a2->base.type &&
            0 == strcmp (a1->addr.s, a2->addr.s));
}

private_extern BRAddress
wkAddressAsBTC (WKAddress address,
                    WKNetworkType *type) {
    WKAddressBTC addressANY = wkAddressCoerce (address, address->type);

    if (NULL != type) *type = address->type;

    return addressANY->addr;
}

WKAddressHandlers wkAddressHandlersBTC = {
    wkAddressReleaseBTC,
    wkAddressAsStringBTC,
    wkAddressIsEqualBTC
};

WKAddressHandlers wkAddressHandlersBCH = {
    wkAddressReleaseBTC,
    wkAddressAsStringBCH,
    wkAddressIsEqualBTC
};

WKAddressHandlers wkAddressHandlersBSV = {
    wkAddressReleaseBTC,
    wkAddressAsStringBSV,
    wkAddressIsEqualBTC
};
