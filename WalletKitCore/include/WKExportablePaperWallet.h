//
//  WKWalletSweeper.h
//  WalletKitCore
//
//  Created by Ed Gamble on 2/23/21
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKExportablePaperWallet_h
#define WKExportablePaperWallet_h

#include "WKCurrency.h"
#include "WKNetwork.h"
#include "WKKey.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: Exportable Paper Wallet

typedef struct WKExportablePaperWalletRecord *WKExportablePaperWallet;

typedef enum {
    WK_EXPORTABLE_PAPER_WALLET_SUCCESS,
    WK_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY,
    WK_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS,
    
    // calling a sweeper function for the wrong type
    WK_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION,
} WKExportablePaperWalletStatus;

extern WKExportablePaperWalletStatus
wkExportablePaperWalletValidateSupported (WKNetwork network,
                                          WKCurrency currency);

extern WKExportablePaperWallet
wkExportablePaperWalletCreate (WKNetwork network,
                               WKCurrency currency);

extern void
wkExportablePaperWalletRelease (WKExportablePaperWallet paperWallet);

extern WKKey
wkExportablePaperWalletGetKey (WKExportablePaperWallet paperWallet);

extern WKAddress
wkExportablePaperWalletGetAddress (WKExportablePaperWallet paperWallet);

#endif /* WKExportablePaperWallet_h */
