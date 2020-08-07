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
} *BRCryptoClientP2PManagerBTC;

static BRCryptoClientP2PManagerBTC
cryptoClientP2PManagerCoerce (BRCryptoClientP2PManager manager) {
    assert (CRYPTO_NETWORK_TYPE_BTC == manager->type ||
            CRYPTO_NETWORK_TYPE_BCH == manager->type ||
            CRYPTO_NETWORK_TYPE_BSV == manager->type);
    return (BRCryptoClientP2PManagerBTC) manager;
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
    BRCryptoTransfer transfer;
} BRCryptoClientP2PManagerPublishInfo;

static void
cryptoClientP2PManagerSendBTC (BRCryptoClientP2PManager baseManager, BRCryptoTransfer transfer) {
    BRCryptoClientP2PManagerBTC manager = cryptoClientP2PManagerCoerce (baseManager);

    BRTransaction *btcTransaction = cryptoTransferAsBTC (transfer);

    BRCryptoClientP2PManagerPublishInfo *btcInfo = calloc (1, sizeof (BRCryptoClientP2PManagerPublishInfo));
    btcInfo->manager  = cryptoWalletManagerTake(&manager->manager->base);
    btcInfo->transfer = cryptoTransferTake (transfer);

    BRPeerManagerPublishTx (manager->btcPeerManager,
                            btcTransaction,
                            btcInfo,
                            cryptoWalletManagerBTCTxPublished);
}

static BRCryptoClientP2PHandlers p2pHandlersBTC = {
    cryptoClientP2PManagerReleaseBTC,
    cryptoClientP2PManagerConnectBTC,
    cryptoClientP2PManagerDisconnectBTC,
    cryptoClientP2PManagerSyncBTC,
    cryptoClientP2PManagerSendBTC
};

// MARK: BRPeerManager Callbacks

static void cryptoWalletManagerBTCSyncStarted (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
#ifdef REFACTOR
    BRPeerSyncManager manager = (BRPeerSyncManager) info;

    // This callback occurs when a sync has started. The behaviour of this function is
    // defined as:
    //   - If we are not in a connected state, signal that we are now connected.
    //   - If we were already in a (full scan) syncing state, signal the termination of that
    //     sync
    //   - Always signal the start of a sync

    if (0 == pthread_mutex_lock (&manager->lock)) {
        uint32_t startBlockHeight = BRPeerManagerLastBlockHeight (manager->peerManager);

        uint8_t needConnectionEvent = !manager->isConnected;
        uint8_t needSyncStartedEvent = 1; // syncStarted callback always indicates a full scan
        uint8_t needSyncStoppedEvent = manager->isFullScan;

        manager->isConnected = needConnectionEvent ? 1 : manager->isConnected;
        manager->isFullScan = needSyncStartedEvent ? 1 : manager->isFullScan;
        manager->successfulScanBlockHeight = MIN (startBlockHeight, manager->successfulScanBlockHeight);

        _peer_log ("BSM: syncStarted needConnect:%"PRIu8", needStart:%"PRIu8", needStop:%"PRIu8"\n",
                   needConnectionEvent, needSyncStartedEvent, needSyncStoppedEvent);

        // Send event while holding the state lock so that we
        // don't broadcast a events out of order.

        if (needSyncStoppedEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_SYNC_STOPPED,
                { .syncStopped = { cryptoSyncStoppedReasonRequested() } }
            });
        }

        if (needConnectionEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_CONNECTED,
            });
        }

        if (needSyncStartedEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_SYNC_STARTED,
            });
        }

        pthread_mutex_unlock (&manager->lock);
    } else {
        assert (0);
    }
#endif
}

static void cryptoWalletManagerBTCSyncStopped (void *info, int reason) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
#ifdef REFACTOR
    BRPeerSyncManager manager = (BRPeerSyncManager) info;

    // This callback occurs when a sync has stopped. This MAY mean we have disconnected or it
    // may mean that we have "caught up" to the blockchain. So, we need to first get the connectivity
    // state of the `BRPeerManager`. The behaviour of this function is defined as:
    //   - If we were in a (full scan) syncing state, signal the termination of that
    //     sync
    //   - If we were connected and are now disconnected, signal that we are now disconnected.

    if (0 == pthread_mutex_lock (&manager->lock)) {
        uint8_t isConnected = BRPeerStatusDisconnected != BRPeerManagerConnectStatus (manager->peerManager);

        uint8_t needSyncStoppedEvent = manager->isFullScan;
        uint8_t needDisconnectionEvent = !isConnected && manager->isConnected;
        uint8_t needSuccessfulScanBlockHeightUpdate = manager->isFullScan && isConnected && !reason;

        manager->isConnected = needDisconnectionEvent ? 0 : isConnected;
        manager->isFullScan = needSyncStoppedEvent ? 0 : manager->isFullScan;
        manager->successfulScanBlockHeight =  needSuccessfulScanBlockHeightUpdate ? BRPeerManagerLastBlockHeight (manager->peerManager) : manager->successfulScanBlockHeight;

        _peer_log ("BSM: syncStopped needStop:%"PRIu8", needDisconnect:%"PRIu8"\n",
                   needSyncStoppedEvent, needDisconnectionEvent);

        // Send event while holding the state lock so that we
        // don't broadcast a events out of order.

        if (needSyncStoppedEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_SYNC_STOPPED,
                { .syncStopped = {
                    (reason ?
                     cryptoSyncStoppedReasonPosix(reason) :
                     (isConnected ?
                      cryptoSyncStoppedReasonComplete() :
                      cryptoSyncStoppedReasonRequested()))
                }
                }
            });
        }

        if (needDisconnectionEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_DISCONNECTED,
                { .disconnected = {
                    (reason ?
                     cryptoWalletManagerDisconnectReasonPosix(reason) :
                     cryptoWalletManagerDisconnectReasonRequested())
                }
                }
            });
        }
        pthread_mutex_unlock (&manager->lock);
    } else {
        assert (0);
    }
#endif
}

static void cryptoWalletManagerBTCTxStatusUpdate (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
#ifdef REFACTOR
    BRPeerSyncManager manager = (BRPeerSyncManager) info;

    // This callback occurs under a number of scenarios.
    //
    // One of those scenario is when a block has been relayed by the P2P network. Thus, it provides an
    // opportunity to get the current block height and update accordingly.
    //
    // The behaviour of this function is defined as:
    //   - If the block height has changed, signal the new value

    if (0 == pthread_mutex_lock (&manager->lock)) {
        uint64_t blockHeight = BRPeerManagerLastBlockHeight (manager->peerManager);

        uint8_t needBlockHeightEvent = blockHeight > manager->networkBlockHeight;

        // Never move the block height "backwards"; always maintain our knowledge
        // of the maximum height observed
        manager->networkBlockHeight = MAX (blockHeight, manager->networkBlockHeight);

        // Send event while holding the state lock so that we
        // don't broadcast a events out of order.

        if (needBlockHeightEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRPeerSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                SYNC_MANAGER_BLOCK_HEIGHT_UPDATED,
                { .blockHeightUpdated = { blockHeight }}
            });
        }

        manager->eventCallback (manager->eventContext,
                                BRPeerSyncManagerAsSyncManager (manager),
                                (BRSyncManagerEvent) {
            SYNC_MANAGER_TXNS_UPDATED
        });

        pthread_mutex_unlock (&manager->lock);
    } else {
        assert (0);
    }
#endif
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

static int  cryptoWalletManagerBTCNetworkIsReachable (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
    return 1;
#ifdef REFACTOR
    BRPeerSyncManager manager = (BRPeerSyncManager) info;
    return BRPeerSyncManagerGetNetworkReachable (manager);
#endif
}

static void cryptoWalletManagerBTCThreadCleanup (void *info) {
    BRCryptoWalletManagerBTC manager = info;
    (void) manager;
#ifdef REFACTOR
#endif
}

static void cryptoWalletManagerBTCTxPublished (void *info, int error) {
    BRCryptoClientP2PManagerPublishInfo *btcInfo = info;
    BRCryptoWalletManager manager = btcInfo->manager;
    BRCryptoTransfer     transfer = btcInfo->transfer;

    pthread_mutex_lock (&manager->lock);

    BRCryptoWallet wallet = manager->wallet;
    assert (cryptoWalletHasTransfer (wallet, transfer));

    BRCryptoTransferState oldState = cryptoTransferGetState (transfer);
    assert (CRYPTO_TRANSFER_STATE_SUBMITTED != oldState.type);

    BRCryptoTransferState newState = cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SUBMITTED);
    cryptoTransferSetState (transfer, newState);

    pthread_mutex_unlock (&manager->lock);
#if 0
    cryptoWalletManagerGenerateTransferEvent (manager, wallet, transfer,
                                              (BRCryptoTransferEvent) {
        CRYPTO_TRANSFER_EVENT_CHANGED,
        { .state = { oldState, newState }}
    });
#endif
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
