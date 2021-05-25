//
//  WKWalletManagerBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKBTC.h"

#include "walletkit/WKAccountP.h"
#include "walletkit/WKNetworkP.h"
#include "walletkit/WKKeyP.h"
#include "walletkit/WKClientP.h"
#include "walletkit/WKWalletManagerP.h"
#include "walletkit/WKWalletSweeperP.h"

#include "bitcoin/BRBitcoinPeerManager.h"

/// BRPeerManager Callbacks
static void wkWalletManagerBTCSyncStarted (void *info);
static void wkWalletManagerBTCSyncStopped (void *info, int reason);
static void wkWalletManagerBTCTxStatusUpdate (void *info);
static void wkWalletManagerBTCSaveBlocks (void *info, int replace, BRBitcoinMerkleBlock **blocks, size_t count);
static void wkWalletManagerBTCSavePeers  (void *info, int replace, const BRBitcoinPeer *peers, size_t count);
static int  wkWalletManagerBTCNetworkIsReachable (void *info);
static void wkWalletManagerBTCThreadCleanup (void *info);
static void wkWalletManagerBTCTxPublished (void *info, int error);

typedef struct WKClientP2PManagerRecordBTC {
    struct WKClientP2PManagerRecord base;
    WKWalletManagerBTC manager;
    BRBitcoinPeerManager *btcPeerManager;

    // The begining and end blockheight for an ongoing sync.  The end block height will be increased
    // a the blockchain is extended.  The begining block height is used to compute the completion
    // percentage.  These will have values of BLOCK_HEIGHT_UNBOUND when a sync is inactive.
    WKBlockNumber begBlockHeight;
    WKBlockNumber endBlockHeight;

    // The blockHeight upon completion of a successful sync.
    WKBlockNumber blockHeight;
    
    /**
     * Flag for whether or not the blockchain network is reachable. This is an atomic
     * int and is NOT protected by the mutable state lock as it is accessed by a
     * BRPeerManager callback that is done while holding a BRPeer's lock. To avoid a
     * deadlock, use an atomic here instead.
     */
    atomic_int isNetworkReachable;
} *WKClientP2PManagerBTC;

static WKClientP2PManagerBTC
wkClientP2PManagerCoerce (WKClientP2PManager manager) {
    assert (WK_NETWORK_TYPE_BTC == manager->type ||
            WK_NETWORK_TYPE_BCH == manager->type ||
            WK_NETWORK_TYPE_BSV == manager->type ||
            WK_NETWORK_TYPE_LTC == manager->type );
    return (WKClientP2PManagerBTC) manager;
}

static bool wkClientP2PManagerSyncInProgress (WKClientP2PManagerBTC p2p) {
    return (BLOCK_HEIGHT_UNBOUND != p2p->begBlockHeight ||
            BLOCK_HEIGHT_UNBOUND != p2p->endBlockHeight);
}

static void
wkClientP2PManagerReleaseBTC (WKClientP2PManager baseManager) {
    WKClientP2PManagerBTC manager = wkClientP2PManagerCoerce (baseManager);
    btcPeerManagerFree (manager->btcPeerManager);
}

static void
wkClientP2PManagerConnectBTC (WKClientP2PManager baseManager,
                                  WKPeer peer) {
    WKClientP2PManagerBTC manager = wkClientP2PManagerCoerce (baseManager);

    // This can occur while disconnected or syncing.

    bool syncInProgress = wkClientP2PManagerSyncInProgress (manager);

    // If we are syncing, then we'll stop the sync; the only way to stop a sync is to disconnect
    if (syncInProgress) {
        btcPeerManagerDisconnect (manager->btcPeerManager);
    }
    wkWalletManagerSetState (&manager->manager->base, wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED));

    // Handle a Peer
    {
        // Assume `peer` is NULL; UINT128_ZERO will restore BRPeerManager peer discovery
        UInt128  address = UINT128_ZERO;
        uint16_t port    = 0;

        if (NULL != peer) {
            WKData16 addrAsInt = wkPeerGetAddrAsInt(peer);
            memcpy (address.u8, addrAsInt.data, sizeof (addrAsInt.data));
            port = wkPeerGetPort (peer);
        }

        // Calling `SetFixedPeer` will 100% disconnect.  We could avoid calling SetFixedPeer
        // if we kept a reference to `peer` and checked if it differs.
        btcPeerManagerSetFixedPeer (manager->btcPeerManager, address, port);
    }

    //    if (!syncInProgress)
    // Change state here; before connecting to the `btcPeerManager` produces start/stop/updates.
    wkWalletManagerSetState (&manager->manager->base, wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_SYNCING));

    // Start periodic updates, sync if required.
    btcPeerManagerConnect(manager->btcPeerManager);
}

static void
wkClientP2PManagerDisconnectBTC (WKClientP2PManager baseManager) {
    WKClientP2PManagerBTC manager = wkClientP2PManagerCoerce (baseManager);

    btcPeerManagerDisconnect (manager->btcPeerManager);
}

// MARK: - Sync

static uint32_t
wkClientP2PManagerCalculateSyncDepthHeight(WKSyncDepth depth,
                                               const BRBitcoinChainParams *chainParams,
                                               uint64_t networkBlockHeight,
                                               OwnershipKept BRBitcoinTransaction *lastConfirmedSendTx) {
    switch (depth) {
        case WK_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND:
            return NULL == lastConfirmedSendTx ? 0 : lastConfirmedSendTx->blockHeight;

        case WK_SYNC_DEPTH_FROM_LAST_TRUSTED_BLOCK: {
            const BRBitcoinCheckPoint *checkpoint = btcChainParamsGetCheckpointBeforeBlockNumber (chainParams,
                                                                                          (uint32_t) MIN (networkBlockHeight, UINT32_MAX));
            return NULL == checkpoint ? 0 : checkpoint->height;
        }

        case WK_SYNC_DEPTH_FROM_CREATION: {
            return 0;
        }
    }
}

static void
wkClientP2PManagerSyncBTC (WKClientP2PManager baseManager,
                                      WKSyncDepth depth,
                                      WKBlockNumber height) {
    WKClientP2PManagerBTC manager = wkClientP2PManagerCoerce (baseManager);

    uint32_t calcHeight = wkClientP2PManagerCalculateSyncDepthHeight (depth,
                                                                          btcPeerManagerChainParams(manager->btcPeerManager),
                                                                          height,
                                                                          NULL /* lastConfirmedSendTx */);
    uint32_t lastHeight = btcPeerManagerLastBlockHeight (manager->btcPeerManager);
    uint32_t scanHeight = MIN (calcHeight, lastHeight);

    if (0 != scanHeight) {
        btcPeerManagerRescanFromBlockNumber (manager->btcPeerManager, scanHeight);
    } else {
        btcPeerManagerRescan (manager->btcPeerManager);
    }
}

typedef struct {
    WKWalletManager manager;
    WKWallet   wallet;
    WKTransfer transfer;
} WKClientP2PManagerPublishInfo;

static void
wkClientP2PManagerSendBTC (WKClientP2PManager baseManager,
                               WKWallet   wallet,
                               WKTransfer transfer) {
    WKClientP2PManagerBTC manager = wkClientP2PManagerCoerce (baseManager);

    BRBitcoinTransaction *btcTransaction = wkTransferAsBTC (transfer);

    WKClientP2PManagerPublishInfo *btcInfo = calloc (1, sizeof (WKClientP2PManagerPublishInfo));
    btcInfo->manager  = wkWalletManagerTake (&manager->manager->base);
    btcInfo->wallet   = wkWalletTake   (wallet);
    btcInfo->transfer = wkTransferTake (transfer);

    btcPeerManagerPublishTx (manager->btcPeerManager,
                            btcTransaction,
                            btcInfo,
                            wkWalletManagerBTCTxPublished);
}

static void
wkClientP2PManagerSetNetworkReachableBTC (WKClientP2PManager baseManager,
                                              int isNetworkReachable) {
    WKClientP2PManagerBTC manager = wkClientP2PManagerCoerce (baseManager);
    atomic_store (&manager->isNetworkReachable, isNetworkReachable);
}

static WKClientP2PHandlers p2pHandlersBTC = {
    wkClientP2PManagerReleaseBTC,
    wkClientP2PManagerConnectBTC,
    wkClientP2PManagerDisconnectBTC,
    wkClientP2PManagerSyncBTC,
    wkClientP2PManagerSendBTC,
    wkClientP2PManagerSetNetworkReachableBTC
};

// MARK: BRPeerManager Callbacks

static void wkWalletManagerBTCSyncStarted (void *info) {
    WKWalletManagerBTC manager = info;
    WKClientP2PManagerBTC p2p = wkClientP2PManagerCoerce (manager->base.p2pManager);

    if (NULL == p2p) return;

    pthread_mutex_lock (&p2p->base.lock);

    //
    // This callback occurs when a sync has started.
    //   - if not connected, then stop the sync (?
    //   - if already syncing, stop the prior sync (REQUESTED) and start the new one
    //   - otherwise simply start the sync;
    //
    // In fact, other than a few minimal `p2p` state changes, we are just generating events.
    //

    // TODO: Does anything here imply 'connected vs not-connected'

    // This `SyncStarted` callback implies a prior sync stopped, if one was active
    bool needStop  = wkClientP2PManagerSyncInProgress (p2p);

    // record the starting block
    p2p->begBlockHeight = btcPeerManagerLastBlockHeight (p2p->btcPeerManager);

    // We'll always start anew.
    bool needStart = true;

    pthread_mutex_unlock (&p2p->base.lock);

    // Stop if already running
    if (needStop) {
        wkWalletManagerGenerateEvent (&manager->base, (WKWalletManagerEvent) {
            WK_WALLET_MANAGER_EVENT_SYNC_STOPPED,
            { .syncStopped = { WK_SYNC_STOPPED_REASON_REQUESTED }}
        });
    }

    if (needStart) {
        wkWalletManagerGenerateEvent (&manager->base, (WKWalletManagerEvent) {
            WK_WALLET_MANAGER_EVENT_SYNC_STARTED
        });
    }

}

static void wkWalletManagerBTCSyncStopped (void *info, int reason) {
    WKWalletManagerBTC manager = info;
    WKClientP2PManagerBTC p2p = wkClientP2PManagerCoerce (manager->base.p2pManager);

    if (NULL == p2p) return;

    pthread_mutex_lock (&p2p->base.lock);

    // This callback occurs when a sync has stopped.  A sync stops if a) if it has lost contact
    // with peers (been 'disconnected') or b) it has completed (caught up with the blockchain's
    // head).

    // TODO: Does anything here imply 'connected vs not-connected'

    // We'll always stop iff we've started.
    bool needStop  = wkClientP2PManagerSyncInProgress (p2p);

    // If the btcPeerManager is not disconnected, then the sync stopped upon completion.  Otherwise
    // the sync stopped upon request.
    bool syncCompleted = (BRPeerStatusDisconnected != btcPeerManagerConnectStatus (p2p->btcPeerManager));

    // With this, we've stopped.
    p2p->begBlockHeight = BLOCK_HEIGHT_UNBOUND;
    p2p->endBlockHeight = BLOCK_HEIGHT_UNBOUND;

    // Update the block height through which we've synced successfully.
    p2p->blockHeight = (syncCompleted && 0 == reason
                        ? btcPeerManagerLastBlockHeight (p2p->btcPeerManager)
                        : BLOCK_HEIGHT_UNBOUND);

    pthread_mutex_unlock (&p2p->base.lock);

    if (needStop) {
        WKSyncStoppedReason stopReason = (reason
                                                ? wkSyncStoppedReasonPosix(reason)
                                                : (syncCompleted
                                                   ? wkSyncStoppedReasonComplete()
                                                   : wkSyncStoppedReasonRequested()));

        wkWalletManagerGenerateEvent (&manager->base, (WKWalletManagerEvent) {
            WK_WALLET_MANAGER_EVENT_SYNC_STOPPED,
            { .syncStopped = { stopReason }}
        });
    }
}

static void wkWalletManagerBTCTxStatusUpdate (void *info) {
    WKWalletManagerBTC manager = info;
    WKClientP2PManagerBTC p2p = wkClientP2PManagerCoerce (manager->base.p2pManager);

    if (NULL == p2p) return;

    pthread_mutex_lock (&p2p->base.lock);

    //
    // This callback occurs under a number of scenarios.  One of those scenario is when a block has
    // been relayed by the P2P network. Thus, it provides an opportunity to get the current block
    // height and update accordingly.
    //   - If the block height has changed, signal the new value
    //

    WKBlockNumber blockHeight = btcPeerManagerLastBlockHeight (p2p->btcPeerManager);
    bool needBlockHeight = (blockHeight > p2p->endBlockHeight);

    // If we are syncing, then we'll make progress

    bool needProgress = BLOCK_HEIGHT_UNBOUND != p2p->begBlockHeight;
    WKTimestamp timestamp = NO_WK_TIMESTAMP;
    WKSyncPercentComplete percentComplete = 0.0;

    if (needProgress) {
        // Update the goal; always forward.
        p2p->endBlockHeight = MAX (blockHeight, p2p->endBlockHeight);

        timestamp = btcPeerManagerLastBlockTimestamp (p2p->btcPeerManager);
        percentComplete = (WKSyncPercentComplete) (100.0 * btcPeerManagerSyncProgress (p2p->btcPeerManager, (uint32_t) p2p->begBlockHeight));
    }

    pthread_mutex_unlock (&p2p->base.lock);

    if (needBlockHeight) {
        wkWalletManagerGenerateEvent (&manager->base, (WKWalletManagerEvent) {
            WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED,
            { .blockHeight = blockHeight }
        });
    }

    if (needProgress) {
        wkWalletManagerGenerateEvent (&manager->base, (WKWalletManagerEvent) {
            WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
            { .syncContinues = { timestamp, percentComplete }}
        });
    }
}

static void wkWalletManagerBTCSaveBlocks (void *info, int replace, BRBitcoinMerkleBlock **blocks, size_t count) {
    WKWalletManagerBTC manager = info;

    if (replace) {
        fileServiceReplace (manager->base.fileService, fileServiceTypeBlocksBTC, (const void **) blocks, count);
    }
    else {
        for (size_t index = 0; index < count; index++)
            fileServiceSave (manager->base.fileService, fileServiceTypeBlocksBTC, blocks[index]);
    }
}

static void wkWalletManagerBTCSavePeers  (void *info, int replace, const BRBitcoinPeer *peers, size_t count) {
    WKWalletManagerBTC manager = info;

    // filesystem changes are NOT queued; they are acted upon immediately

    if (!replace) {
        // save each peer, one-by-one
        for (size_t index = 0; index < count; index++)
            fileServiceSave (manager->base.fileService, fileServiceTypePeersBTC, &peers[index]);
    }

    else if (0 == count) {
        // no peers to set, just do a clear
        fileServiceClear (manager->base.fileService, fileServiceTypePeersBTC);
    }

    else {
        // fileServiceReplace expects an array of pointers to entities, instead of an array of
        // structures so let's do the conversion here
        const BRBitcoinPeer **peerRefs = calloc (count, sizeof(BRBitcoinPeer *));

        for (size_t i = 0; i < count; i++) {
            peerRefs[i] = &peers[i];
        }

        fileServiceReplace (manager->base.fileService, fileServiceTypePeersBTC, (const void **) peerRefs, count);
        free (peerRefs);
    }
}

static int wkWalletManagerBTCNetworkIsReachable (void *info) {
    WKWalletManagerBTC manager = info;
    WKClientP2PManager baseP2P = manager->base.p2pManager;
    if (NULL == baseP2P) return 1;
    
    WKClientP2PManagerBTC btcP2P = wkClientP2PManagerCoerce (baseP2P);
    return atomic_load (&btcP2P->isNetworkReachable);
}

static void wkWalletManagerBTCThreadCleanup (void *info) {
    WKWalletManagerBTC manager = info;
    (void) manager;
    // Nothing, ever (?)
}

static void wkWalletManagerBTCTxPublished (void *info, int error) {
    WKClientP2PManagerPublishInfo *btcInfo = info;
    WKWalletManager manager = btcInfo->manager;
    WKTransfer     transfer = btcInfo->transfer;

    pthread_mutex_lock (&manager->lock);

    WKWallet wallet = manager->wallet;
    assert (wkWalletHasTransfer (wallet, transfer));

    WKTransferState oldState = wkTransferGetState (transfer);
    assert (WK_TRANSFER_STATE_SUBMITTED != oldState->type);
    wkTransferStateGive (oldState);

    wkTransferSetState (transfer, wkTransferStateInit (WK_TRANSFER_STATE_SUBMITTED));

    pthread_mutex_unlock (&manager->lock);

    wkWalletManagerGive(manager);
    wkTransferGive (transfer);
    free (info);
}

extern WKClientP2PManager
wkWalletManagerCreateP2PManagerBTC (WKWalletManager manager) {
    assert (NULL == manager->p2pManager);

    WKClientP2PManager p2pManager = wkClientP2PManagerCreate (sizeof (struct WKClientP2PManagerRecordBTC),
                                                                         manager->type,
                                                                         &p2pHandlersBTC);
    WKClientP2PManagerBTC p2pManagerBTC = wkClientP2PManagerCoerce (p2pManager);
    p2pManagerBTC->manager = wkWalletManagerCoerceBTC (manager, p2pManager->type);

    const BRBitcoinChainParams *btcChainParams = wkNetworkAsBTC(manager->network);
    BRBitcoinWallet *btcWallet = wkWalletAsBTC(manager->wallet);
    uint32_t btcEarliestKeyTime = (uint32_t) wkAccountGetTimestamp(manager->account);

    BRArrayOf(BRBitcoinMerkleBlock*) blocks = initialBlocksLoadBTC (manager);
    BRArrayOf(BRBitcoinPeer)         peers  = initialPeersLoadBTC  (manager);

    p2pManagerBTC->begBlockHeight = BLOCK_HEIGHT_UNBOUND;
    p2pManagerBTC->endBlockHeight = BLOCK_HEIGHT_UNBOUND;
    p2pManagerBTC->blockHeight    = BLOCK_HEIGHT_UNBOUND;

    p2pManagerBTC->btcPeerManager = btcPeerManagerNew (btcChainParams,
                                                btcWallet,
                                                btcEarliestKeyTime,
                                                blocks, (NULL == blocks ? 0 : array_count (blocks)),
                                                peers,  (NULL == peers  ? 0 : array_count (peers)));


    assert (NULL != p2pManagerBTC->btcPeerManager);

    btcPeerManagerSetCallbacks (p2pManagerBTC->btcPeerManager,
                               wkWalletManagerCoerceBTC (manager, p2pManager->type),
                               wkWalletManagerBTCSyncStarted,
                               wkWalletManagerBTCSyncStopped,
                               wkWalletManagerBTCTxStatusUpdate,
                               wkWalletManagerBTCSaveBlocks,
                               wkWalletManagerBTCSavePeers,
                               wkWalletManagerBTCNetworkIsReachable,
                               wkWalletManagerBTCThreadCleanup);

    if (NULL != blocks) array_free (blocks);
    if (NULL != peers ) array_free (peers);

    return p2pManager;
}
