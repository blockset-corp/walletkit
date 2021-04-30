
//
//  WKExportablePaperWalletBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 2/24/21
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKBTC.h"
#include "walletkit/WKExportablePaperWalletP.h"
#include "walletkit/WKKeyP.h"


static WKExportablePaperWalletStatus
wkExportablePaperWalletValidateSupporteBTC (WKNetwork network,
                                                WKCurrency currency) {
    return WK_EXPORTABLE_PAPER_WALLET_SUCCESS;
}

static WKAddress
wkExportablePaperWalletCreateAddressBTC (WKNetwork network,
                                             WKKey key) {
    BRAddressParams addrParamsBTC = wkNetworkAsBTC (network)->addrParams;
    BRKey          *keyBTC        = wkKeyGetCore (key);

    // encode using legacy format (only supported method for BTC)
    size_t addrLength = BRKeyLegacyAddr (keyBTC, NULL, 0, addrParamsBTC);

    char addr [addrLength + 1];
    BRKeyLegacyAddr (keyBTC, addr, addrLength, addrParamsBTC);
    addr[addrLength] = '\0';

    return wkAddressCreateFromStringAsBTC (addrParamsBTC, addr);
}

static WKExportablePaperWallet
wkExportablePaperWalletCreateBTC (WKNetwork network,
                                      WKCurrency currency) {
    BRKey keyBTC;
    if (1 != BRKeyGenerateRandom (&keyBTC, 1)) return NULL;

    WKKey     key     = wkKeyCreateFromKey (&keyBTC);
    WKAddress address = wkExportablePaperWalletCreateAddressBTC (network, key);

    WKExportablePaperWallet wallet =
    wkExportablePaperWalletAllocAndInit (sizeof (struct WKExportablePaperWalletRecord),
                                             network->type,
                                             network,
                                             address,
                                             key);

    wkAddressGive (address);
    wkKeyGive     (key);

    return wallet;
}

WKExportablePaperWalletHandlers wkExportablePaperWalletHandlersBTC = {
    wkExportablePaperWalletCreateBTC,
    NULL,
    wkExportablePaperWalletValidateSupporteBTC
};

// WKExportablePaperWalletHandlers wkExportablePaperWalletHandlersBCH;
// WKExportablePaperWalletHandlers wkExportablePaperWalletHandlersBSV;
