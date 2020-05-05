//
//  BRCryptoWalletManagerClient.c
//  BRCrypto
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoClientP.h"

#include <errno.h>
#include <math.h>  // round()
#include <stdbool.h>

static void
cryptoClientP2PManagerSync (BRCryptoClientP2PManager p2p, BRCryptoSyncDepth depth, BRCryptoBlockChainHeight height);

static void
cryptoClientP2PManagerSend (BRCryptoClientP2PManager p2p, BRCryptoTransfer transfer);

static void
cryptoClientQRYManagerSync (BRCryptoClientQRYManager qry, BRCryptoSyncDepth depth, BRCryptoBlockChainHeight height);

static void
cryptoClientQRYManagerSend (BRCryptoClientQRYManager qry, BRCryptoTransfer transfer);


// MARK: Client Sync

extern void
cryptoClientSync (BRCryptoClientSync sync,
                  BRCryptoSyncDepth depth,
                  BRCryptoBlockChainHeight height) {
    switch (sync.type) {
        case CRYPTO_CLIENT_P2P_MANAGER_TYPE:
            cryptoClientP2PManagerSync (sync.u.p2pManager, depth, height);
            break;
        case CRYPTO_CLIENT_QRY_MANAGER_TYPE:
            cryptoClientQRYManagerSync (sync.u.qryManager, depth, height);
            break;
    }
}

extern void
cryptoClientSyncPeriodic (BRCryptoClientSync sync) {
    switch (sync.type) {
        case CRYPTO_CLIENT_P2P_MANAGER_TYPE: break;
        case CRYPTO_CLIENT_QRY_MANAGER_TYPE: cryptoClientQRYManagerTickTock (sync.u.qryManager);
    }
}

// MARK: Client Send

extern void
cryptoClientSend (BRCryptoClientSend send, BRCryptoTransfer transfer) {
    switch (send.type) {
        case CRYPTO_CLIENT_P2P_MANAGER_TYPE:
            cryptoClientP2PManagerSend (send.u.p2pManager, transfer);
            break;
        case CRYPTO_CLIENT_QRY_MANAGER_TYPE:
            cryptoClientQRYManagerSend (send.u.qryManager, transfer);
            break;
    }
}

// MARK: Client P2P (Peer-to-Peer)

extern BRCryptoClientP2PManager
cryptoClientP2PManagerCreate (size_t sizeInBytes,
                              BRCryptoBlockChainType type,
                              const BRCryptoClientP2PHandlers *handlers) {
    assert (sizeInBytes >= sizeof (struct BRCryptoClientP2PManagerRecord));

    BRCryptoClientP2PManager p2pManager = calloc (1, sizeInBytes);

    p2pManager->type        = type;
    p2pManager->handlers    = handlers;
    p2pManager->sizeInBytes = sizeInBytes;

    return p2pManager;
}

extern void
cryptoClientP2PManagerRelease (BRCryptoClientP2PManager p2p) {
    p2p->handlers->release (p2p);
    memset (p2p, 0, p2p->sizeInBytes);
    free (p2p);
}

#include <stdio.h>

extern void
cryptoClientP2PManagerConnect (BRCryptoClientP2PManager p2p,
                               BRCryptoPeer peer) {
    printf ("P2P: Want to Connect\n");
}

extern void
cryptoClientP2PManagerDisconnect (BRCryptoClientP2PManager p2p) {
    printf ("P2P: Want to Disconnect\n");

}

static void
cryptoClientP2PManagerSync (BRCryptoClientP2PManager p2p,
                            BRCryptoSyncDepth depth,
                            BRCryptoBlockChainHeight height) {
    p2p->handlers->sync (p2p, depth, height);
}

static void
cryptoClientP2PManagerSend (BRCryptoClientP2PManager p2p, BRCryptoTransfer transfer) {
    p2p->handlers->send (p2p, transfer);
}

// MARK: Client QRY (QueRY)

extern BRCryptoClientQRYManager
cryptoClientQRYManagerCreate (BRCryptoClient client,
                              BRCryptoWalletManager manager) {
    BRCryptoClientQRYManager qryManager = calloc (1, sizeof (struct BRCryptoClientQRYManagerRecord));

    qryManager->client  = client;
    qryManager->manager = manager;

    return qryManager;
}

extern void
cryptoClientQRYManagerRelease (BRCryptoClientQRYManager qry) {
    memset (qry, 0, sizeof(*qry));
    free (qry);
}

extern void
cryptoClientQRYManagerConnect (BRCryptoClientQRYManager qry) {
    printf ("QRY: Want to Connect\n");
}

extern void
cryptoClientQRYManagerDisconnect (BRCryptoClientQRYManager qry) {
    printf ("QRY: Want to Disconnect\n");
}

extern void
cryptoClientQRYManagerTickTock (BRCryptoClientQRYManager qry) {
#if 0
    qry->client.funcGetBlockNumber (qry->client.context,
                                    qry,
                                    qry->requestId++)
#endif

#ifdef REFACTOR /* BTC */
static void
BRClientSyncManagerUpdateBlockNumber(BRClientSyncManager manager) {
    uint8_t needClientCall = 0;
    int rid                = -1;

    if (0 == pthread_mutex_lock (&manager->lock)) {
        needClientCall = manager->isConnected;
        rid = needClientCall ? BRClientSyncManagerGenerateRid (manager) : rid;

        pthread_mutex_unlock (&manager->lock);
    } else {
        assert (0);
    }

    if (needClientCall) {
        manager->clientCallbacks.funcGetBlockNumber (manager->clientContext,
                                                     BRClientSyncManagerAsSyncManager (manager),
                                                     rid);
    }

static void
BRClientSyncManagerUpdateTransactions (BRClientSyncManager manager) {
    int rid                      = 0;
    uint8_t needSyncEvent        = 0;
    uint8_t needClientCall       = 0;
    uint64_t begBlockNumber      = 0;
    uint64_t endBlockNumber      = 0;
    size_t addressCount          = 0;
    BRArrayOf(char *) addresses  = NULL;

    if (0 == pthread_mutex_lock (&manager->lock)) {
        // check if we are connect and the prior sync has completed.
        if (!BRClientSyncManagerScanStateIsInProgress (&manager->scanState) &&
            manager->isConnected) {

            BRClientSyncManagerScanStateInit (&manager->scanState,
                                              manager->wallet,
                                              BRChainParamsIsBitcoin (manager->chainParams),
                                              manager->syncedBlockHeight,
                                              manager->networkBlockHeight,
                                              BRClientSyncManagerGenerateRid (manager));

            // get the addresses to query the BDB with
            addresses = BRClientSyncManagerConvertAddressToString
            (manager, BRClientSyncManagerScanStateGetAddresses (&manager->scanState));

            assert (NULL != addresses);
            addressCount = array_count (addresses);
            assert (0 != addressCount);

            // store sync data for callback outside of lock
            rid = BRClientSyncManagerScanStateGetRequestId (&manager->scanState);
            begBlockNumber = BRClientSyncManagerScanStateGetStartBlockNumber (&manager->scanState);
            endBlockNumber = BRClientSyncManagerScanStateGetEndBlockNumber (&manager->scanState);

            // store control flow flags
            needSyncEvent = BRClientSyncManagerScanStateIsFullScan (&manager->scanState);
            needClientCall = 1;
        }

        // Send event while holding the state lock so that event
        // callbacks are ordered to reflect state transitions.

        if (needSyncEvent) {
            manager->eventCallback (manager->eventContext,
                                    BRClientSyncManagerAsSyncManager (manager),
                                    (BRSyncManagerEvent) {
                                        SYNC_MANAGER_SYNC_STARTED,
                                    });
        }

        pthread_mutex_unlock (&manager->lock);
    } else {
        assert (0);
    }

    if (needClientCall) {
        // Callback to 'client' to get all transactions (for all wallet addresses) between
        // a {beg,end}BlockNumber.  The client will gather the transactions and then call
        // bwmAnnounceTransaction()  (for each one or with all of them).
        manager->clientCallbacks.funcGetTransactions (manager->clientContext,
                                                      BRClientSyncManagerAsSyncManager (manager),
                                                      (const char **) addresses,
                                                      addressCount,
                                                      begBlockNumber,
                                                      endBlockNumber,
                                                      rid);
    }

    if (NULL != addresses) {
        for (size_t index = 0; index < addressCount; index++) {
            free (addresses[index]);
        }
        array_free (addresses);
    }
}
#endif

#ifdef REFACTOR /* GEN */
    BRGenericManager gwm = (BRGenericManager) event->context;

     gwm->client.getBlockNumber (gwm->client.context,
                                 gwm,
                                 gwm->requestId++);

     // Handle a BRD Sync:

     // 1) check if the prior sync has completed successfully
     if (gwm->brdSync.completed && gwm->brdSync.success) {
         // 1a) if so, advance the sync range by updating `begBlockNumber`
         gwm->brdSync.begBlockNumber = (gwm->brdSync.endBlockNumber >=  GWM_BRD_SYNC_START_BLOCK_OFFSET
                                        ? gwm->brdSync.endBlockNumber - GWM_BRD_SYNC_START_BLOCK_OFFSET
                                        : 0);
     }

     // 2) completed or not, update the `endBlockNumber` to the current block height.
     gwm->brdSync.endBlockNumber = MAX (gwm->blockHeight, gwm->brdSync.begBlockNumber);

     // 3) we'll update transactions if there are more blocks to examine
     if (gwm->brdSync.begBlockNumber != gwm->brdSync.endBlockNumber) {
         BRGenericAddress accountAddress = genManagerGetAccountAddress(gwm);
         char *address = genAddressAsString (accountAddress);

         // 3a) Save the current requestId and mark as not completed.
         gwm->brdSync.rid = gwm->requestId;
         gwm->brdSync.completed = false;
         gwm->brdSync.success = false;

         // 3b) Query all transactions; each one found will have bwmAnnounceTransaction() invoked
         // which will process the transaction into the wallet.

         // Callback to 'client' to get all transactions (for all wallet addresses) between
         // a {beg,end}BlockNumber.  The client will gather the transactions and then call
         // bwmAnnounceTransaction()  (for each one or with all of them).
         if (gwm->handlers->manager.apiSyncType() == GENERIC_SYNC_TYPE_TRANSFER) {
             gwm->client.getTransfers (gwm->client.context,
                                       gwm,
                                       address,
                                       gwm->brdSync.begBlockNumber,
                                       gwm->brdSync.endBlockNumber,
                                       gwm->brdSync.rid);
         } else {
             gwm->client.getTransactions (gwm->client.context,
                                          gwm,
                                          address,
                                          gwm->brdSync.begBlockNumber,
                                          gwm->brdSync.endBlockNumber,
                                          gwm->brdSync.rid);
         }

         // 3c) On to the next rid
         gwm->requestId += 1;

         free (address);
         genAddressRelease(accountAddress);
     }

     if (NULL != gwm->syncCallback)
         gwm->syncCallback (gwm->syncContext,
                            gwm,
                            gwm->brdSync.begBlockNumber,
                            gwm->brdSync.endBlockNumber,
                            // lots of incremental sync 'slop'
                            2 * GWM_BRD_SYNC_START_BLOCK_OFFSET);

     // End handling a BRD Sync
#endif
}

static void
cryptoClientQRYManagerSync (BRCryptoClientQRYManager qry,
                            BRCryptoSyncDepth depth,
                            BRCryptoBlockChainHeight height) {

}

static void
cryptoClientQRYManagerSend (BRCryptoClientQRYManager qry, BRCryptoTransfer transfer) {
//    typedef void
//    (*BRCryptoClientSubmitTransactionCallback) (BRCryptoClientContext context,
//                                                OwnershipGiven BRCryptoWalletManager manager,
//                                                OwnershipGiven BRCryptoClientCallbackState callbackState,
//                                                OwnershipKept const uint8_t *transaction,
//                                                size_t transactionLength,
//                                                OwnershipKept const char *hashAsHex);

//    qry->client.funcSubmitTransaction (qry->client.context,
//                                       cwm,
//             );
    
}

