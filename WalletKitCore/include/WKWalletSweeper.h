//
//  WKWalletSweeper.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 5/22/20
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletSweeper_h
#define WKWalletSweeper_h

#include "WKClient.h"
#include "WKKey.h"
#include "WKAmount.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WKWalletSweeperRecord *WKWalletSweeper;

// MARK: Wallet Sweeper Status

typedef enum {
    WK_WALLET_SWEEPER_SUCCESS,
    WK_WALLET_SWEEPER_UNSUPPORTED_CURRENCY,
    WK_WALLET_SWEEPER_INVALID_KEY,
    WK_WALLET_SWEEPER_INVALID_ARGUMENTS,
    WK_WALLET_SWEEPER_INVALID_TRANSACTION,
    WK_WALLET_SWEEPER_INVALID_SOURCE_WALLET,
    WK_WALLET_SWEEPER_NO_TRANSFERS_FOUND,
    WK_WALLET_SWEEPER_INSUFFICIENT_FUNDS,
    WK_WALLET_SWEEPER_UNABLE_TO_SWEEP,
    
    // calling a sweeper function for the wrong type
    WK_WALLET_SWEEPER_ILLEGAL_OPERATION,
} WKWalletSweeperStatus;

extern void
wkWalletSweeperRelease (WKWalletSweeper sweeper);

extern WKWalletSweeperStatus
wkWalletSweeperAddTransactionFromBundle (WKWalletSweeper sweeper,
                                             OwnershipKept WKClientTransactionBundle bundle);

extern void
wkWalletManagerEstimateFeeBasisForWalletSweep (WKWalletSweeper sweeper,
                                                   WKWalletManager manager,
                                                   WKWallet wallet,
                                                   WKCookie cookie,
                                                   WKNetworkFee fee);

extern WKTransfer
wkWalletSweeperCreateTransferForWalletSweep (WKWalletSweeper sweeper,
                                                 WKWalletManager manager,
                                                 WKWallet wallet,
                                                 WKFeeBasis estimatedFeeBasis);

extern WKKey
wkWalletSweeperGetKey (WKWalletSweeper sweeper);

extern WKAddress
wkWalletSweeperGetAddress (WKWalletSweeper sweeper);

extern WKAmount
wkWalletSweeperGetBalance (WKWalletSweeper sweeper);

extern WKWalletSweeperStatus
wkWalletSweeperValidate (WKWalletSweeper sweeper);

#ifdef __cplusplus
}
#endif

#endif
