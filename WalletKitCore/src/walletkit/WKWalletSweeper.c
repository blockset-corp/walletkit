//
//  WKWalletSweeper.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 5/22/20
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKWalletP.h"
#include "WKWalletSweeperP.h"
#include "WKHandlersP.h"


extern WKWalletSweeper
wkWalletSweeperAllocAndInit (size_t sizeInBytes,
                                 WKNetworkType type,
                                 WKKey key,
                                 WKUnit unit) {
    assert (sizeInBytes >= sizeof (struct WKWalletSweeperRecord));
    assert (wkKeyHasSecret (key));
    
    WKWalletSweeper sweeper = calloc (1, sizeInBytes);
    
    sweeper->type = type;
    sweeper->handlers = wkHandlersLookup(type)->sweeper;
    sweeper->key = wkKeyTake (key);
    sweeper->unit = wkUnitTake (unit);
    
    return sweeper;
}

extern void
wkWalletSweeperRelease (WKWalletSweeper sweeper) {
    
    sweeper->handlers->release (sweeper);
    
    wkKeyGive (sweeper->key);
    wkUnitGive (sweeper->unit);
    
    memset (sweeper, 0, sizeof(*sweeper));
    free (sweeper);
}

extern WKWalletSweeperStatus
wkWalletSweeperAddTransactionFromBundle (WKWalletSweeper sweeper,
                                             OwnershipKept WKClientTransactionBundle bundle) {
    if (NULL == sweeper->handlers->addTranactionFromBundle) {
        assert(0);
        return WK_WALLET_SWEEPER_ILLEGAL_OPERATION;
    }
    
    return sweeper->handlers->addTranactionFromBundle (sweeper, bundle);
}

extern void
wkWalletManagerEstimateFeeBasisForWalletSweep (WKWalletSweeper sweeper,
                                                   WKWalletManager cwm,
                                                   WKWallet wallet,
                                                   WKCookie cookie,
                                                   WKNetworkFee fee) {
    WKFeeBasis feeBasis = sweeper->handlers->estimateFeeBasis (cwm,
                                                                     wallet,
                                                                     cookie,
                                                                     sweeper,
                                                                     fee);
    
    if (NULL != feeBasis)
        wkWalletGenerateEvent (wallet, wkWalletEventCreateFeeBasisEstimated (WK_SUCCESS, cookie, feeBasis));

    wkFeeBasisGive(feeBasis);
}

extern WKTransfer
wkWalletSweeperCreateTransferForWalletSweep (WKWalletSweeper sweeper,
                                                 WKWalletManager cwm,
                                                 WKWallet wallet,
                                                 WKFeeBasis estimatedFeeBasis) {
    WKTransfer transfer = sweeper->handlers->createTransfer (cwm, wallet, sweeper, estimatedFeeBasis);
    
    wkTransferGenerateEvent (transfer, (WKTransferEvent) {
        WK_TRANSFER_EVENT_CREATED
    });
    
    return transfer;
}

extern WKKey
wkWalletSweeperGetKey (WKWalletSweeper sweeper) {
    return wkKeyTake (sweeper->key);
}

extern WKAddress
wkWalletSweeperGetAddress (WKWalletSweeper sweeper) {
    return sweeper->handlers->getAddress (sweeper);
}

extern WKAmount
wkWalletSweeperGetBalance (WKWalletSweeper sweeper) {
    return sweeper->handlers->getBalance (sweeper);
}

extern WKWalletSweeperStatus
wkWalletSweeperValidate (WKWalletSweeper sweeper) {
    return sweeper->handlers->validate (sweeper);
}
