//
//  WKWalletSweeperP.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 5/22/20
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletSweeperP_h
#define WKWalletSweeperP_h

#include "WKWalletSweeper.h"


#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Sweeper Handlers

typedef void
(*WKWalletSweeperReleaseHandler) (WKWalletSweeper sweeper);

typedef WKAddress
(*WKWalletSweeperGetAddressHandler) (WKWalletSweeper);

typedef WKAmount
(*WKWalletSweeperGetBalanceHandler) (WKWalletSweeper);

typedef WKWalletSweeperStatus
(*WKWalletSweeperAddTransactionFromBundleHandler) (WKWalletSweeper sweeper,
                                                         OwnershipKept WKClientTransactionBundle bundle);

typedef WKFeeBasis
(*WKWalletSweeperEstimateFeeBasisHandler) (WKWalletManager cwm,
                                                 WKWallet wallet,
                                                 WKCookie cookie,
                                                 WKWalletSweeper sweeper,
                                                 WKNetworkFee fee);

typedef WKTransfer
(*WKWalletSweeperCreateTransferHandler) (WKWalletManager cwm,
                                               WKWallet wallet,
                                               WKWalletSweeper sweeper,
                                               WKFeeBasis estimatedFeeBasis);
typedef WKWalletSweeperStatus
(*WKWalletSweeperValidateHandler) (WKWalletSweeper sweeper);

typedef struct {
    WKWalletSweeperReleaseHandler release;
    WKWalletSweeperGetAddressHandler getAddress;
    WKWalletSweeperGetBalanceHandler getBalance;
    WKWalletSweeperAddTransactionFromBundleHandler addTranactionFromBundle;
    WKWalletSweeperEstimateFeeBasisHandler estimateFeeBasis;
    WKWalletSweeperCreateTransferHandler createTransfer;
    WKWalletSweeperValidateHandler validate;
} WKWalletSweeperHandlers;

// MARK: - Sweeper

struct WKWalletSweeperRecord {
    WKNetworkType type;
    const WKWalletSweeperHandlers *handlers;
    WKKey key;
    WKUnit unit;
};

private_extern WKWalletSweeper
wkWalletSweeperAllocAndInit (size_t sizeInBytes,
                                 WKNetworkType type,
                                 WKKey key,
                                 WKUnit unit);


#ifdef __cplusplus
}
#endif

#endif /* WKWalletSweeperP_h */
