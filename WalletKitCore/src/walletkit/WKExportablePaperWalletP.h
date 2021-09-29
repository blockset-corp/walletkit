//
//  WKExportablePaperWalletP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 2/23/21
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKExportablePaperWalletP_h
#define WKExportablePaperWalletP_h

#include "WKExportablePaperWallet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef WKExportablePaperWallet
(*WKExportablePaperWalletCreateHandler) (WKNetwork network,
                                               WKCurrency currency);

typedef void
(*WKExportablePaperWalletReleaseHandler) (WKExportablePaperWallet wallet);

typedef WKExportablePaperWalletStatus
(*WKExportablePaperWalletValidateSupportedHandler) (WKNetwork network,
                                                          WKCurrency currency);

typedef struct {
    WKExportablePaperWalletCreateHandler create;
    WKExportablePaperWalletReleaseHandler release;
    WKExportablePaperWalletValidateSupportedHandler validateSupported;
} WKExportablePaperWalletHandlers;

// MARK: - Sweeper

struct WKExportablePaperWalletRecord {
    WKNetworkType type;
    const WKExportablePaperWalletHandlers *handlers;
    size_t sizeInBytes;

    WKNetwork network;
    WKAddress address;
    WKKey key;
};


private_extern WKExportablePaperWallet
wkExportablePaperWalletAllocAndInit (size_t sizeInBytes,
                                         WKNetworkType type,
                                         WKNetwork network,
                                         WKAddress address,
                                         WKKey key);

private_extern void
wkExportablePaperWalletReleaseInternal (WKExportablePaperWallet wallet);

#endif /* WKExportablePaperWalletP_h */
