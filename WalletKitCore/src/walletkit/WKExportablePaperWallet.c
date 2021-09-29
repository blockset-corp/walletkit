//
//  WKExportablePaperWallet.c
//  WalletKitCore
//
//  Created by Ed Gamble on 2/23/21
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKExportablePaperWalletP.h"
#include "WKHandlersP.h"

private_extern WKExportablePaperWallet
wkExportablePaperWalletAllocAndInit (size_t sizeInBytes,
                                         WKNetworkType type,
                                         WKNetwork network,
                                         WKAddress address,
                                         WKKey key) {
    assert (sizeInBytes >= sizeof (struct WKExportablePaperWalletRecord));
    assert (wkKeyHasSecret (key));

    WKExportablePaperWallet wallet = calloc (1, sizeInBytes);

    wallet->type = type;
    wallet->sizeInBytes = sizeInBytes;
    wallet->handlers = wkHandlersLookup(type)->exportablePaperWallet;

    wallet->network  = wkNetworkTake (network);
    wallet->address  = wkAddressTake (address);
    wallet->key      = wkKeyTake     (key);

    return wallet;
}

private_extern void
wkExportablePaperWalletReleaseInternal (WKExportablePaperWallet wallet) {
    wkNetworkGive (wallet->network);
    wkAddressGive (wallet->address);
    wkKeyGive     (wallet->key);

    memset (wallet, 0, wallet->sizeInBytes);
    free (wallet);
}

extern WKExportablePaperWalletStatus
wkExportablePaperWalletValidateSupported (WKNetwork network,
                                              WKCurrency currency) {
    if (WK_FALSE == wkNetworkHasCurrency (network, currency))
        return WK_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS;

    const WKHandlers *handlers = wkHandlersLookup (network->type);

    return ((NULL != handlers->exportablePaperWallet &&
             NULL != handlers->exportablePaperWallet->validateSupported)
            ? handlers->exportablePaperWallet->validateSupported (network, currency)
            : WK_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION);
}

extern WKExportablePaperWallet
wkExportablePaperWalletCreate (WKNetwork network,
                                   WKCurrency currency) {
    const WKHandlers *handlers = wkHandlersLookup (network->type);

    return ((NULL != handlers->exportablePaperWallet &&
             NULL != handlers->exportablePaperWallet->create)
            ? handlers->exportablePaperWallet->create (network, currency)
            : NULL);
}

extern void
wkExportablePaperWalletRelease (WKExportablePaperWallet wallet) {
    if (NULL != wallet->handlers->release)
        wallet->handlers->release (wallet);
    else
        wkExportablePaperWalletReleaseInternal (wallet);
}

extern WKKey
wkExportablePaperWalletGetKey (WKExportablePaperWallet wallet) {
    return wkKeyTake (wallet->key);
}

extern WKAddress
wkExportablePaperWalletGetAddress (WKExportablePaperWallet wallet) {
    return wkAddressTake (wallet->address);
}
