//
//  BRCryptoWalletManagerBTC.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoBTC.h"

#include "crypto/BRCryptoAccountP.h"
#include "crypto/BRCryptoNetworkP.h"
#include "crypto/BRCryptoKeyP.h"
#include "crypto/BRCryptoClientP.h"
#include "crypto/BRCryptoWalletManagerP.h"
#include "crypto/BRCryptoWalletSweeperP.h"

#include "bitcoin/BRPeerManager.h"

/// BRPeerManager Callbacks
static void cryptoWalletManagerBTCSyncStarted (void *info);
static void cryptoWalletManagerBTCSyncStopped (void *info, int reason);
static void cryptoWalletManagerBTCTxStatusUpdate (void *info);
static void cryptoWalletManagerBTCSaveBlocks (void *info, int replace, BRMerkleBlock **blocks, size_t count);
static void cryptoWalletManagerBTCSavePeers  (void *info, int replace, const BRPeer *peers, size_t count);
static int  cryptoWalletManagerBTCNetworkIsReachable (void *info);
static void cryptoWalletManagerBTCThreadCleanup (void *info);
static void cryptoWalletManagerBTCTxPublished (void *info, int error);

typedef struct BRCryptoClientP2PManagerRecordBTC {
    struct BRCryptoClientP2PManagerRecord base;
    BRCryptoWalletManagerBTC manager;
    BRPeerManager *btcPeerManager;

    // The begining and end blockheight for an ongoing sync.  The end block height will be increased
    // a the blockchain is extended.  The begining block height is used to compute the completion
    // percentage.  These will have values of BLOCK_HEIGHT_UNBOUND when a sync is inactive.
    BRCryptoBlockNumber begBlockHeight;
    BRCryptoBlockNumber endBlockHeight;

    // The blockHeight upon completion of a successful sync.
    BRCryptoBlockNumber blockHeight;
    
    /**
     * Flag for whether or not the blockchain network is reachable. This is an atomic
     * int and is NOT protected by the mutable state lock as it is accessed by a
     * BRPeerManager callback that is done while holding a BRPeer's lock. To avoid a
     * deadlock, use an atomic here instead.
     */
    atomic_int isNetworkReachable;
} *BRCryptoClientP2PManagerBTC;

static BRCryptoClientP2PManagerBTC
cryptoClientP2PManagerCoerce (BRCryptoClientP2PManager manager) {
    assert (CRYPTO_NETWORK_TYPE_BTC == manager->type ||
            CRYPTO_NETWORK_TYPE_BCH == manager->type ||
            CRYPTO_NETWORK_TYPE_BSV == manager->type);
    return (BRCryptoClientP2PManagerBTC) manager;
}

static bool cryptoClientP2PManagerSyncInProgress (BRCryptoClientP2PManagerBTC p2p) {
    return (BLOCK_HEIGHT_UNBOUND != p2p->begBlockHeight ||
            BLOCK_HEIGHT_UNBOUND != p2p->endBlockHeight);
}

static void
cryptoClientP2PManagerReleaseBTC (BRCryptoClientP2PManager baseManager) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    BRPeerManagerFree (manager->btcPeerManager);
}

static void
cryptoClientP2PManagerConnectBTC (BRCryptoClientP2PManager baseManager,
                                  BRCryptoPeer peer) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    // This can occur while disconnected or syncing.

    bool syncInProgress = cryptoClientP2PManagerSyncInProgress (manager);

    // If we are syncing, then we'll stop the sync; the only way to stop a sync is to disconnect
    if (syncInProgress) {
        BRPeerManagerDisconnect (manager->btcPeerManager);
    }
    cryptoWalletManagerSetState (&manager->manager->base, cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_CONNECTED));

    // Handle a Peer
    {
        // Assume `peer` is NULL; UINT128_ZERO will restore BRPeerManager peer discovery
        UInt128  address = UINT128_ZERO;
        uint16_t port    = 0;

        if (NULL != peer) {
            BRCryptoData16 addrAsInt = cryptoPeerGetAddrAsInt(peer);
            memcpy (address.u8, addrAsInt.data, sizeof (addrAsInt.data));
            port = cryptoPeerGetPort (peer);
        }

        // Calling `SetFixedPeer` will 100% disconnect.  We could avoid calling SetFixedPeer
        // if we kept a reference to `peer` and checked if it differs.
        BRPeerManagerSetFixedPeer (manager->btcPeerManager, address, port);
    }

    //    if (!syncInProgress)
    // Change state here; before connecting to the `btcPeerManager` produces start/stop/updates.
    cryptoWalletManagerSetState (&manager->manager->base, cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_SYNCING));

    // Start periodic updates, sync if required.
    BRPeerManagerConnect(manager->btcPeerManager);
}

static void
cryptoClientP2PManagerDisconnectBTC (BRCryptoClientP2PManager baseManager) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    BRPeerManagerDisconnect (manager->btcPeerManager);
}

// MARK: - Sync

static uint32_t
cryptoClientP2PManagerCalculateSyncDepthHeight(BRCryptoSyncDepth depth,
                                               const BRChainParams *chainParams,
                                               uint64_t networkBlockHeight,
                                               OwnershipKept BRTransaction *lastConfirmedSendTx) {
    switch (depth) {
        case CRYPTO_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND:
            return NULL == lastConfirmedSendTx ? 0 : lastConfirmedSendTx->blockHeight;

        case CRYPTO_SYNC_DEPTH_FROM_LAST_TRUSTED_BLOCK: {
            const BRCheckPoint *checkpoint = BRChainParamsGetCheckpointBeforeBlockNumber (chainParams,
                                                                                          (uint32_t) MIN (networkBlockHeight, UINT32_MAX));
            return NULL == checkpoint ? 0 : checkpoint->height;
        }

        case CRYPTO_SYNC_DEPTH_FROM_CREATION: {
            return 0;
        }
    }
}

static void
cryptoClientP2PManagerSyncBTC (BRCryptoClientP2PManager baseManager,
                                      BRCryptoSyncDepth depth,
                                      BRCryptoBlockNumber height) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    uint32_t calcHeight = cryptoClientP2PManagerCalculateSyncDepthHeight (depth,
                                                                          BRPeerManagerChainParams(manager->btcPeerManager),
                                                                          height,
                                                                          NULL /* lastConfirmedSendTx */);
    uint32_t lastHeight = BRPeerManagerLastBlockHeight (manager->btcPeerManager);
    uint32_t scanHeight = MIN (calcHeight, lastHeight);

    if (0 != scanHeight) {
        BRPeerManagerRescanFromBlockNumber (manager->btcPeerManager, scanHeight);
    } else {
        BRPeerManagerRescan (manager->btcPeerManager);
    }
}

typedef struct {
    BRCryptoWalletManager manager;
    BRCryptoWallet   wallet;
    BRCryptoTransfer transfer;
} BRCryptoClientP2PManagerPublishInfo;

static void
cryptoClientP2PManagerSendBTC (BRCryptoClientP2PManager baseManager,
                               BRCryptoWallet   wallet,
                               BRCryptoTransfer transfer) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    BRTransaction *btcTransaction = cryptoTransferAsBTC (transfer);

    BRCryptoClientP2PManagerPublishInfo *btcInfo = calloc (1, sizeof (BRCryptoClientP2PManagerPublishInfo));
    btcInfo->manager  = cryptoWalletManagerTake (&manager->manager->base);
    btcInfo->wallet   = cryptoWalletTake   (wallet);
    btcInfo->transfer = cryptoTransferTake (transfer);

    BRPeerManagerPublishTx (manager->btcPeerManager,
                            btcTransaction,
                            btcInfo,
                            cryptoWalletManagerBTCTxPublished);
}

static void
cryptoClientP2PManagerSetNetworkReachableBTC (BRCryptoClientP2PManager baseManager,
                                              int isNetworkReachable) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);
    atomic_store (&manager->isNetworkReachable, isNetworkReachable);
}

static BRCryptoClientP2PHandlers p2pHandlersBTC = {
    cryptoClientP2PManagerReleaseBTC,
    cryptoClientP2PManagerConnectBTC,
    cryptoClientP2PManagerDisconnectBTC,
    cryptoClientP2PManagerSyncBTC,
    cryptoClientP2PManagerSendBTC,
    cryptoClientP2PManagerSetNetworkReachableBTC
};

// MARK: BRPeerManager Callbacks

static void cryptoWalletManagerBTCSyncStarted (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    BRCryptoClientP2PManagerBTC p2p = cryptoClientP2PManagerCoerce (manager->base.p2pManager);

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
    bool needStop  = cryptoClientP2PManagerSyncInProgress (p2p);

    // record the starting block
    p2p->begBlockHeight = BRPeerManagerLastBlockHeight (p2p->btcPeerManager);

    // We'll always start anew.
    bool needStart = true;

    pthread_mutex_unlock (&p2p->base.lock);

    // Stop if already running
    if (needStop) {
        cryptoWalletManagerGenerateEvent (&manager->base, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED,
            { .syncStopped = { CRYPTO_SYNC_STOPPED_REASON_REQUESTED }}
        });
    }

    if (needStart) {
        cryptoWalletManagerGenerateEvent (&manager->base, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED
        });
    }

}

static void cryptoWalletManagerBTCSyncStopped (void *info, int reason) {
    BRCryptoWalletManagerBTC manager = info;
    BRCryptoClientP2PManagerBTC p2p = cryptoClientP2PManagerCoerce (manager->base.p2pManager);

    if (NULL == p2p) return;

    pthread_mutex_lock (&p2p->base.lock);

    // This callback occurs when a sync has stopped.  A sync stops if a) if it has lost contact
    // with peers (been 'disconnected') or b) it has completed (caught up with the blockchain's
    // head).

    // TODO: Does anything here imply 'connected vs not-connected'

    // We'll always stop iff we've started.
    bool needStop  = cryptoClientP2PManagerSyncInProgress (p2p);

    // If the btcPeerManager is not disconnected, then the sync stopped upon completion.  Otherwise
    // the sync stopped upon request.
    bool syncCompleted = (BRPeerStatusDisconnected != BRPeerManagerConnectStatus (p2p->btcPeerManager));

    // With this, we've stopped.
    p2p->begBlockHeight = BLOCK_HEIGHT_UNBOUND;
    p2p->endBlockHeight = BLOCK_HEIGHT_UNBOUND;

    // Update the block height through which we've synced successfully.
    p2p->blockHeight = (syncCompleted && 0 == reason
                        ? BRPeerManagerLastBlockHeight (p2p->btcPeerManager)
                        : BLOCK_HEIGHT_UNBOUND);

    pthread_mutex_unlock (&p2p->base.lock);

    if (needStop) {
        BRCryptoSyncStoppedReason stopReason = (reason
                                                ? cryptoSyncStoppedReasonPosix(reason)
                                                : (syncCompleted
                                                   ? cryptoSyncStoppedReasonComplete()
                                                   : cryptoSyncStoppedReasonRequested()));

        cryptoWalletManagerGenerateEvent (&manager->base, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED,
            { .syncStopped = { stopReason }}
        });
    }
}

static void cryptoWalletManagerBTCTxStatusUpdate (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    BRCryptoClientP2PManagerBTC p2p = cryptoClientP2PManagerCoerce (manager->base.p2pManager);

    if (NULL == p2p) return;

    pthread_mutex_lock (&p2p->base.lock);

    //
    // This callback occurs under a number of scenarios.  One of those scenario is when a block has
    // been relayed by the P2P network. Thus, it provides an opportunity to get the current block
    // height and update accordingly.
    //   - If the block height has changed, signal the new value
    //

    BRCryptoBlockNumber blockHeight = BRPeerManagerLastBlockHeight (p2p->btcPeerManager);
    bool needBlockHeight = (blockHeight > p2p->endBlockHeight);

    // If we are syncing, then we'll make progress

    bool needProgress = BLOCK_HEIGHT_UNBOUND != p2p->begBlockHeight;
    BRCryptoTimestamp timestamp = NO_CRYPTO_TIMESTAMP;
    BRCryptoSyncPercentComplete percentComplete = 0.0;

    if (needProgress) {
        // Update the goal; always forward.
        p2p->endBlockHeight = MAX (blockHeight, p2p->endBlockHeight);

        timestamp = BRPeerManagerLastBlockTimestamp (p2p->btcPeerManager);
        percentComplete = (BRCryptoSyncPercentComplete) (100.0 * BRPeerManagerSyncProgress (p2p->btcPeerManager, (uint32_t) p2p->begBlockHeight));
    }

    pthread_mutex_unlock (&p2p->base.lock);

    if (needBlockHeight) {
        cryptoWalletManagerGenerateEvent (&manager->base, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED,
            { .blockHeight = blockHeight }
        });
    }

    if (needProgress) {
        cryptoWalletManagerGenerateEvent (&manager->base, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
            { .syncContinues = { timestamp, percentComplete }}
        });
    }
}

static void cryptoWalletManagerBTCSaveBlocks (void *info, int replace, BRMerkleBlock **blocks, size_t count) {
    BRCryptoWalletManagerBTC manager = info;

    if (replace) {
        fileServiceReplace (manager->base.fileService, fileServiceTypeBlocksBTC, (const void **) blocks, count);
    }
    else {
        for (size_t index = 0; index < count; index++)
            fileServiceSave (manager->base.fileService, fileServiceTypeBlocksBTC, blocks[index]);
    }
}

static void cryptoWalletManagerBTCSavePeers  (void *info, int replace, const BRPeer *peers, size_t count) {
    BRCryptoWalletManagerBTC manager = info;

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
        const BRPeer **peerRefs = calloc (count, sizeof(BRPeer *));

        for (size_t i = 0; i < count; i++) {
            peerRefs[i] = &peers[i];
        }

        fileServiceReplace (manager->base.fileService, fileServiceTypePeersBTC, (const void **) peerRefs, count);
        free (peerRefs);
    }
}

static int cryptoWalletManagerBTCNetworkIsReachable (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    BRCryptoClientP2PManager baseP2P = manager->base.p2pManager;
    if (NULL == baseP2P) return 1;
    
    BRCryptoClientP2PManagerBTC btcP2P = cryptoClientP2PManagerCoerce (baseP2P);
    return atomic_load (&btcP2P->isNetworkReachable);
}

static void cryptoWalletManagerBTCThreadCleanup (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
    // Nothing, ever (?)
}

static void cryptoWalletManagerBTCTxPublished (void *info, int error) {
    BRCryptoClientP2PManagerPublishInfo *btcInfo = info;
    BRCryptoWalletManager manager = btcInfo->manager;
    BRCryptoTransfer     transfer = btcInfo->transfer;

    pthread_mutex_lock (&manager->lock);

    BRCryptoWallet wallet = manager->wallet;
    assert (cryptoWalletHasTransfer (wallet, transfer));

    BRCryptoTransferState oldState = cryptoTransferGetState (transfer);
    assert (CRYPTO_TRANSFER_STATE_SUBMITTED != oldState->type);
    cryptoTransferStateGive (oldState);

    cryptoTransferSetState (transfer, cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SUBMITTED));

    pthread_mutex_unlock (&manager->lock);

    cryptoWalletManagerGive(manager);
    cryptoTransferGive (transfer);
    free (info);
}

extern BRCryptoClientP2PManager
cryptoWalletManagerCreateP2PManagerBTC (BRCryptoWalletManager manager) {
    assert (NULL == manager->p2pManager);

    BRCryptoClientP2PManager p2pManager = cryptoClientP2PManagerCreate (sizeof (struct BRCryptoClientP2PManagerRecordBTC),
                                                                         manager->type,
                                                                         &p2pHandlersBTC);
    BRCryptoClientP2PManagerBTC p2pManagerBTC = cryptoClientP2PManagerCoerce (p2pManager);
    p2pManagerBTC->manager = cryptoWalletManagerCoerceBTC (manager, p2pManager->type);

    const BRChainParams *btcChainParams = cryptoNetworkAsBTC(manager->network);
    BRWallet *btcWallet = cryptoWalletAsBTC(manager->wallet);
    uint32_t btcEarliestKeyTime = (uint32_t) cryptoAccountGetTimestamp(manager->account);

    BRArrayOf(BRMerkleBlock*) blocks = initialBlocksLoadBTC (manager);
    BRArrayOf(BRPeer)         peers  = initialPeersLoadBTC  (manager);

    p2pManagerBTC->begBlockHeight = BLOCK_HEIGHT_UNBOUND;
    p2pManagerBTC->endBlockHeight = BLOCK_HEIGHT_UNBOUND;
    p2pManagerBTC->blockHeight    = BLOCK_HEIGHT_UNBOUND;

    p2pManagerBTC->btcPeerManager = BRPeerManagerNew (btcChainParams,
                                                btcWallet,
                                                btcEarliestKeyTime,
                                                blocks, (NULL == blocks ? 0 : array_count (blocks)),
                                                peers,  (NULL == peers  ? 0 : array_count (peers)));


    assert (NULL != p2pManagerBTC->btcPeerManager);

    BRPeerManagerSetCallbacks (p2pManagerBTC->btcPeerManager,
                               cryptoWalletManagerCoerceBTC (manager, p2pManager->type),
                               cryptoWalletManagerBTCSyncStarted,
                               cryptoWalletManagerBTCSyncStopped,
                               cryptoWalletManagerBTCTxStatusUpdate,
                               cryptoWalletManagerBTCSaveBlocks,
                               cryptoWalletManagerBTCSavePeers,
                               cryptoWalletManagerBTCNetworkIsReachable,
                               cryptoWalletManagerBTCThreadCleanup);

    if (NULL != blocks) array_free (blocks);
    if (NULL != peers ) array_free (peers);

    return p2pManager;
}
