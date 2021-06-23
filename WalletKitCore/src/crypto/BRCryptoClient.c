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
#include <stdio.h>          // printf

#include "support/BRArray.h"
#include "support/BRCrypto.h"
#include "support/BROSCompat.h"

#include "BRCryptoAddressP.h"
#include "BRCryptoHashP.h"
#include "BRCryptoFileService.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoWalletP.h"
#include "BRCryptoWalletManagerP.h"
#include "BRCryptoSystemP.h"

#define OFFSET_BLOCKS_IN_SECONDS       (3 * 24 * 60 * 60)  // 3 days

// MARK: Client Sync/Send Forward Declarations

static void
cryptoClientP2PManagerSync (BRCryptoClientP2PManager p2p, BRCryptoSyncDepth depth, BRCryptoBlockNumber height);

static void
cryptoClientP2PManagerSend (BRCryptoClientP2PManager p2p, BRCryptoWallet wallet, BRCryptoTransfer transfer);

static void
cryptoClientQRYManagerSync (BRCryptoClientQRYManager qry, BRCryptoSyncDepth depth, BRCryptoBlockNumber height);

static void
cryptoClientQRYManagerSend (BRCryptoClientQRYManager qry,  BRCryptoWallet wallet, BRCryptoTransfer transfer);

// MARK: Client Sync

extern void
cryptoClientSync (BRCryptoClientSync sync,
                  BRCryptoSyncDepth depth,
                  BRCryptoBlockNumber height) {
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
        case CRYPTO_CLIENT_P2P_MANAGER_TYPE:
            /* Nothing */
            break;
        case CRYPTO_CLIENT_QRY_MANAGER_TYPE:
            cryptoClientQRYManagerTickTock (sync.u.qryManager);
            break;
    }
}

// MARK: Client Send

extern void
cryptoClientSend (BRCryptoClientSend send,
                  BRCryptoWallet wallet,
                  BRCryptoTransfer transfer) {
    switch (send.type) {
        case CRYPTO_CLIENT_P2P_MANAGER_TYPE:
            cryptoClientP2PManagerSend (send.u.p2pManager, wallet, transfer);
            break;
        case CRYPTO_CLIENT_QRY_MANAGER_TYPE:
            cryptoClientQRYManagerSend (send.u.qryManager, wallet, transfer);
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

    pthread_mutex_init_brd (&p2pManager->lock, PTHREAD_MUTEX_NORMAL);

    return p2pManager;
}

extern void
cryptoClientP2PManagerRelease (BRCryptoClientP2PManager p2p) {
    p2p->handlers->release (p2p);
    memset (p2p, 0, p2p->sizeInBytes);
    free (p2p);
}

extern void
cryptoClientP2PManagerConnect (BRCryptoClientP2PManager p2p,
                               BRCryptoPeer peer) {
    p2p->handlers->connect (p2p, peer);
}

extern void
cryptoClientP2PManagerDisconnect (BRCryptoClientP2PManager p2p) {
    p2p->handlers->disconnect (p2p);
}

static void
cryptoClientP2PManagerSync (BRCryptoClientP2PManager p2p,
                            BRCryptoSyncDepth depth,
                            BRCryptoBlockNumber height) {
    p2p->handlers->sync (p2p, depth, height);
}

static void
cryptoClientP2PManagerSend (BRCryptoClientP2PManager p2p,
                            BRCryptoWallet   wallet,
                            BRCryptoTransfer transfer) {
    p2p->handlers->send (p2p, wallet, transfer);
}

extern void
cryptoClientP2PManagerSetNetworkReachable (BRCryptoClientP2PManager p2p,
                                           BRCryptoBoolean isNetworkReachable) {
    if (NULL != p2p->handlers->setNetworkReachable) {
        p2p->handlers->setNetworkReachable (p2p, CRYPTO_TRUE == isNetworkReachable);
    }
}

// MARK: Client QRY (QueRY)

static void cryptoClientQRYRequestBlockNumber  (BRCryptoClientQRYManager qry);
static bool cryptoClientQRYRequestTransactionsOrTransfers (BRCryptoClientQRYManager qry,
                                                           BRCryptoClientCallbackType type,
                                                           OwnershipKept  BRSetOf(BRCryptoAddress) oldAddresses,
                                                           OwnershipGiven BRSetOf(BRCryptoAddress) newAddresses,
                                                           size_t requestId);
static void cryptoClientQRYSubmitTransfer      (BRCryptoClientQRYManager qry,
                                                BRCryptoWallet   wallet,
                                                BRCryptoTransfer transfer);

extern BRCryptoClientQRYManager
cryptoClientQRYManagerCreate (BRCryptoClient client,
                              BRCryptoWalletManager manager,
                              BRCryptoClientQRYByType byType,
                              BRCryptoBlockNumber earliestBlockNumber,
                              BRCryptoBlockNumber currentBlockNumber) {
    BRCryptoClientQRYManager qry = calloc (1, sizeof (struct BRCryptoClientQRYManagerRecord));

    qry->client    = client;
    qry->manager   = manager;
    qry->requestId = 0;
    qry->byType    = byType;

    // For 'GET /transaction' we'll back up from the begBlockNumber by this offset.  Currently
    // about three days.  If the User has their App open continuously and if `GET /transactions`
    // fails for 2 days, then once it recovers, the App will get the 'missed' transactions back
    // from 3 days agao.
    qry->blockNumberOffset = OFFSET_BLOCKS_IN_SECONDS / manager->network->confirmationPeriodInSeconds;
    qry->blockNumberOffset = MAX (qry->blockNumberOffset, 100);

    // Initialize the `brdSync` struct
    qry->sync.rid = SIZE_MAX;
    qry->sync.begBlockNumber = earliestBlockNumber;
    qry->sync.endBlockNumber = MAX (earliestBlockNumber, currentBlockNumber);
    qry->sync.completed = true;
    qry->sync.success   = false;
    qry->sync.unbounded = CRYPTO_CLIENT_QRY_IS_UNBOUNDED;

    qry->connected = false;

    pthread_mutex_init_brd (&qry->lock, PTHREAD_MUTEX_NORMAL);
    return qry;
}

extern void
cryptoClientQRYManagerRelease (BRCryptoClientQRYManager qry) {
    // Try to ensure that `qry->lock` is unlocked prior to destroy.
    pthread_mutex_lock (&qry->lock);
    pthread_mutex_unlock (&qry->lock);
    // Tiny race
    pthread_mutex_destroy (&qry->lock);

    memset (qry, 0, sizeof(*qry));
    free (qry);
}

extern void
cryptoClientQRYManagerConnect (BRCryptoClientQRYManager qry) {
    pthread_mutex_lock (&qry->lock);
    qry->connected = true;
    cryptoWalletManagerSetState (qry->manager, cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_SYNCING));
    pthread_mutex_unlock (&qry->lock);

    // Start a sync immediately.
    cryptoClientQRYManagerTickTock (qry);
}

extern void
cryptoClientQRYManagerDisconnect (BRCryptoClientQRYManager qry) {
    pthread_mutex_lock (&qry->lock);
    cryptoWalletManagerSetState (qry->manager, cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_CONNECTED));
    qry->connected = false;
    pthread_mutex_unlock (&qry->lock);
}


static void
cryptoClientQRYManagerSync (BRCryptoClientQRYManager qry,
                            BRCryptoSyncDepth depth,
                            BRCryptoBlockNumber height) {

}

static BRCryptoBlockNumber
cryptoClientQRYGetNetworkBlockHeight (BRCryptoClientQRYManager qry){
    return cryptoNetworkGetHeight (qry->manager->network);
}

static void
cryptoClientQRYManagerSend (BRCryptoClientQRYManager qry,
                            BRCryptoWallet wallet,
                            BRCryptoTransfer transfer) {
    cryptoClientQRYSubmitTransfer (qry, wallet, transfer);
}

static void
cryptoClientQRYManagerUpdateSync (BRCryptoClientQRYManager qry,
                                  bool completed,
                                  bool success,
                                  bool needLock) {
    if (needLock) pthread_mutex_lock(&qry->lock);

    bool needBegEvent = !completed && qry->sync.completed;
    bool needEndEvent = completed && !qry->sync.completed;

    //
    // If the sync is an incremental sync - because the `begBlockNumber` is near to the current
    // network's block height, then we don't want events generated.  This makes the QRY events
    // similar to P2P events where as each block is announced NO sync event are generated.
    //
    // We are a little bit sloppy with avoiding the events.  Normally the `begBlockNumber` is
    // `blockNumberOffset` less than the current network's height - but we'll avoid events if
    // `begBlockNumber` is within `2 * blockNumberOffset`
    //
    if (qry->sync.begBlockNumber >= cryptoClientQRYGetNetworkBlockHeight (qry) - 2 * qry->blockNumberOffset) {
        needBegEvent = false;
        needEndEvent = false;
    }
    
    qry->sync.completed = completed;
    qry->sync.success   = success;

    if (needBegEvent) {
        cryptoWalletManagerSetState (qry->manager, (BRCryptoWalletManagerState) {
            CRYPTO_WALLET_MANAGER_STATE_SYNCING
        });

        cryptoWalletManagerGenerateEvent (qry->manager, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED
        });

        cryptoWalletManagerGenerateEvent (qry->manager, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
            { .syncContinues = { NO_CRYPTO_TIMESTAMP, 0 }}
        });
    }

    if (needEndEvent) {
        cryptoWalletManagerGenerateEvent (qry->manager, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
            { .syncContinues = { NO_CRYPTO_TIMESTAMP, 100 }}
        });

        cryptoWalletManagerGenerateEvent (qry->manager, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED,
            { .syncStopped = (success
                              ? cryptoSyncStoppedReasonComplete()
                              : cryptoSyncStoppedReasonUnknown()) }
        });

        cryptoWalletManagerSetState (qry->manager, (BRCryptoWalletManagerState) {
            CRYPTO_WALLET_MANAGER_STATE_CONNECTED
        });
    }

    if (needLock) pthread_mutex_unlock(&qry->lock);
}

extern void
cryptoClientQRYManagerTickTock (BRCryptoClientQRYManager qry) {
    pthread_mutex_lock (&qry->lock);

    // Only continue if connected
    if (qry->connected) {
        switch (qry->manager->syncMode) {
            case CRYPTO_SYNC_MODE_API_ONLY:
            case CRYPTO_SYNC_MODE_API_WITH_P2P_SEND:
                // Alwwys get the current block
                cryptoClientQRYRequestBlockNumber (qry);
                break;

            case CRYPTO_SYNC_MODE_P2P_WITH_API_SYNC:
            case CRYPTO_SYNC_MODE_P2P_ONLY:
                break;
        }
    }

    pthread_mutex_unlock (&qry->lock);
}

static void
cryptoClientQRYRequestSync (BRCryptoClientQRYManager qry, bool needLock) {
    if (needLock) pthread_mutex_lock (&qry->lock);

    // If we've successfully completed a sync then update `begBlockNumber` which will always be
    // `blockNumberOffset` prior to the `endBlockNumber`.  Using the offset, which is weeks prior,
    // ensures that we don't miss a range of blocks in the {transfer,transaction} query.

    if (qry->sync.completed && qry->sync.success) {

        // Be careful to ensure that the `begBlockNumber` is not negative.
        qry->sync.begBlockNumber = (qry->sync.endBlockNumber >= qry->blockNumberOffset
                                    ? qry->sync.endBlockNumber - qry->blockNumberOffset
                                    : 0);
    }

    // Whether or not we completed , update the `endBlockNumber` to the current block height.
    qry->sync.endBlockNumber = MAX (cryptoClientQRYGetNetworkBlockHeight (qry),
                                    qry->sync.begBlockNumber);

    // We'll update transactions if there are more blocks to examine and if the prior sync
    // completed (successfully or not).
    if (qry->sync.completed && qry->sync.begBlockNumber != qry->sync.endBlockNumber) {

        // Save the current requestId and mark the sync as completed successfully.
        qry->sync.rid = qry->requestId++;

        // Mark the sync as completed, unsucessfully (the initial state)
        cryptoClientQRYManagerUpdateSync (qry, false, false, false);

        // Get the addresses for the manager's wallet
        BRCryptoWallet wallet = cryptoWalletManagerGetWallet (qry->manager);
        BRSetOf(BRCryptoAddress) addresses = cryptoWalletGetAddressesForRecovery (wallet);
        assert (0 != BRSetCount(addresses));

        // We'll force the 'client' to return all transactions w/o regard to the `endBlockNumber`
        // Doing this ensures that the initial 'full-sync' returns everything.  Thus there is no
        // need to wait for a future 'tick tock' to get the recent and pending transactions'.  For
        // BTC the future 'tick tock' is minutes away; which is a burden on Users as they wait.

        cryptoClientQRYRequestTransactionsOrTransfers (qry,
                                                       (CRYPTO_CLIENT_REQUEST_USE_TRANSFERS == qry->byType
                                                        ? CLIENT_CALLBACK_REQUEST_TRANSFERS
                                                        : CLIENT_CALLBACK_REQUEST_TRANSACTIONS),
                                                       NULL,
                                                       addresses,
                                                       qry->sync.rid);

        cryptoWalletGive (wallet);
    }

    if (needLock) pthread_mutex_unlock (&qry->lock);
}

// MARK: - Client Callback State

static BRCryptoClientCallbackState
cryptoClientCallbackStateCreate (BRCryptoClientCallbackType type,
                                 size_t rid) {
    BRCryptoClientCallbackState state = calloc (1, sizeof (struct BRCryptoClientCallbackStateRecord));

    state->type = type;
    state->rid  = rid;

    return state;
}

static BRCryptoClientCallbackState
cryptoClientCallbackStateCreateGetTrans (BRCryptoClientCallbackType type,
                                         OwnershipGiven BRSetOf(BRCryptoAddress) addresses,
                                         size_t rid) {
    assert (CLIENT_CALLBACK_REQUEST_TRANSFERS    == type ||
            CLIENT_CALLBACK_REQUEST_TRANSACTIONS == type);
    BRCryptoClientCallbackState state = cryptoClientCallbackStateCreate (type, rid);

    switch (type) {
        case CLIENT_CALLBACK_REQUEST_TRANSFERS:
            state->u.getTransfers.addresses = addresses;
            break;
        case CLIENT_CALLBACK_REQUEST_TRANSACTIONS:
            state->u.getTransactions.addresses = addresses;
            break;
        default:
            assert (0);
            break;
    }

    return state;
}

static BRCryptoClientCallbackState
cryptoClientCallbackStateCreateSubmitTransaction (BRCryptoWallet wallet,
                                                  BRCryptoTransfer transfer,
                                                  size_t rid) {
    BRCryptoClientCallbackState state = cryptoClientCallbackStateCreate (CLIENT_CALLBACK_SUBMIT_TRANSACTION, rid);

    state->u.submitTransaction.wallet   = cryptoWalletTake   (wallet);
    state->u.submitTransaction.transfer = cryptoTransferTake (transfer);

    return state;
}

static BRCryptoClientCallbackState
cryptoClientCallbackStateCreateEstimateTransactionFee (BRCryptoHash hash,
                                                       BRCryptoCookie cookie,
                                                       OwnershipKept BRCryptoNetworkFee networkFee,
                                                       OwnershipKept BRCryptoFeeBasis initialFeeBasis,
                                                       size_t rid) {
    BRCryptoClientCallbackState state = cryptoClientCallbackStateCreate (CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE, rid);

    state->u.estimateTransactionFee.cookie = cookie;
    state->u.estimateTransactionFee.networkFee = cryptoNetworkFeeTake (networkFee);
    state->u.estimateTransactionFee.initialFeeBasis = cryptoFeeBasisTake (initialFeeBasis);

    return state;
}

static void
cryptoClientCallbackStateRelease (BRCryptoClientCallbackState state) {
    switch (state->type) {
        case CLIENT_CALLBACK_REQUEST_TRANSFERS:
            cryptoAddressSetRelease (state->u.getTransfers.addresses);
            break;

        case CLIENT_CALLBACK_REQUEST_TRANSACTIONS:
            cryptoAddressSetRelease(state->u.getTransactions.addresses);
            break;

        case CLIENT_CALLBACK_SUBMIT_TRANSACTION:
            cryptoWalletGive   (state->u.submitTransaction.wallet);
            cryptoTransferGive (state->u.submitTransaction.transfer);
            break;

        case CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE:
            cryptoHashGive (state->u.estimateTransactionFee.hash);
            cryptoNetworkFeeGive (state->u.estimateTransactionFee.networkFee);
            cryptoFeeBasisGive (state->u.estimateTransactionFee.initialFeeBasis);
            break;

        default:
            break;
    }

    memset (state, 0, sizeof (struct BRCryptoClientCallbackStateRecord));
    free (state);
}


// MARK: - Request/Announce Block Number

typedef struct {
    BREvent base;
    BRCryptoWalletManager manager;
    BRCryptoClientCallbackState callbackState;
    BRCryptoBoolean success;
    BRCryptoBlockNumber blockNumber;
    char * blockHashString;
} BRCryptoClientAnnounceBlockNumberEvent;

static void
cryptoClientHandleBlockNumber (OwnershipKept BRCryptoWalletManager cwm,
                               OwnershipGiven BRCryptoClientCallbackState callbackState,
                               BRCryptoBoolean success,
                               BRCryptoBlockNumber blockNumber,
                               char *blockHashString) {

    BRCryptoBlockNumber oldBlockNumber = cryptoNetworkGetHeight (cwm->network);

    if (oldBlockNumber != blockNumber) {
        cryptoNetworkSetHeight (cwm->network, blockNumber);

        if (NULL != blockHashString && '\0' != blockHashString[0]) {
            BRCryptoHash verifiedBlockHash = cryptoNetworkCreateHashFromString (cwm->network, blockHashString);
            cryptoNetworkSetVerifiedBlockHash (cwm->network, verifiedBlockHash);
            cryptoHashGive (verifiedBlockHash);
        }

        cryptoWalletManagerGenerateEvent (cwm, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED,
            { .blockHeight = blockNumber }
        });
    }

    cryptoClientCallbackStateRelease (callbackState);
    cryptoMemoryFree(blockHashString);

    // After getting any block number, whether successful or not, do a sync.  The sync will be
    // incremental or full - depending on where the last sync ended.
    cryptoClientQRYRequestSync (cwm->qryManager, true);
}

static void
cryptoClientAnnounceBlockNumberDispatcher (BREventHandler ignore,
                                           BRCryptoClientAnnounceBlockNumberEvent *event) {
    cryptoClientHandleBlockNumber (event->manager,
                                   event->callbackState,
                                   event->success,
                                   event->blockNumber,
                                   event->blockHashString);
}

static void
cryptoClientAnnounceBlockNumberDestroyer (BRCryptoClientAnnounceBlockNumberEvent *event) {
    cryptoWalletManagerGive (event->manager);
    cryptoClientCallbackStateRelease (event->callbackState);
    cryptoMemoryFree (event->blockHashString);
}

BREventType handleClientAnnounceBlockNumberEventType = {
    "CWM: Handle Client Announce Block Number Event",
    sizeof (BRCryptoClientAnnounceBlockNumberEvent),
    (BREventDispatcher) cryptoClientAnnounceBlockNumberDispatcher,
    (BREventDestroyer)  cryptoClientAnnounceBlockNumberDestroyer
};

extern void
cryptoClientAnnounceBlockNumber (OwnershipKept BRCryptoWalletManager cwm,
                               OwnershipGiven BRCryptoClientCallbackState callbackState,
                               BRCryptoBoolean success,
                               BRCryptoBlockNumber blockNumber,
                               const char *blockHashString) {
    BRCryptoClientAnnounceBlockNumberEvent event =
    { { NULL, &handleClientAnnounceBlockNumberEventType },
        cryptoWalletManagerTakeWeak(cwm),
        callbackState,
        success,
        blockNumber,
        (NULL == blockHashString ? NULL : strdup (blockHashString)) };

    eventHandlerSignalEvent (cwm->handler, (BREvent *) &event);
}

static void
cryptoClientQRYRequestBlockNumber (BRCryptoClientQRYManager qry) {
    // Extract CWM, checking to make sure it still lives
    BRCryptoWalletManager cwm = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == cwm) return;

    BRCryptoClientCallbackState callbackState = cryptoClientCallbackStateCreate (CLIENT_CALLBACK_REQUEST_BLOCK_NUMBER,
                                                                                 qry->requestId++);

    qry->client.funcGetBlockNumber (qry->client.context,
                                    cryptoWalletManagerTake (cwm),
                                    callbackState);

    cryptoWalletManagerGive (cwm);
}

// MARK: - Request/Announce Transactions

typedef struct {
    BREvent base;
    BRCryptoWalletManager manager;
    BRCryptoClientCallbackState callbackState;
    BRCryptoBoolean success;
    BRArrayOf (BRCryptoClientTransactionBundle) bundles;
} BRCryptoClientAnnounceTransactionsEvent;

extern void
cryptoClientHandleTransactions (OwnershipKept BRCryptoWalletManager manager,
                                OwnershipGiven BRCryptoClientCallbackState callbackState,
                                BRCryptoBoolean success,
                                BRArrayOf (BRCryptoClientTransactionBundle) bundles) {

    BRCryptoClientQRYManager qry = manager->qryManager;

    pthread_mutex_lock (&qry->lock);
    bool matchedRids = (callbackState->rid == qry->sync.rid);
    pthread_mutex_unlock (&qry->lock);

    bool syncCompleted = false;
    bool syncSuccess   = false;

    // Process the results if the bundles are for our rid; otherwise simply discard;
    if (matchedRids) {
        switch (success) {
            case CRYPTO_TRUE: {
                size_t bundlesCount = array_count(bundles);

                // Save the transaction bundles immediately
                for (size_t index = 0; index < bundlesCount; index++)
                    cryptoWalletManagerSaveTransactionBundle(manager, bundles[index]);

                // Sort bundles to have the lowest blocknumber first.  Use of `mergesort` is
                // appropriate given that the bundles are likely already ordered.  This minimizes
                // dependency resolution between later transactions depending on prior transactions.
                //
                // Seems that there may be duplicates in `bundles`; will be dealt with later

                mergesort_brd (bundles, bundlesCount, sizeof (BRCryptoClientTransactionBundle),
                               cryptoClientTransactionBundleCompareForSort);

                // Recover transfers from each bundle
                for (size_t index = 0; index < bundlesCount; index++)
                   cryptoWalletManagerRecoverTransfersFromTransactionBundle (manager, bundles[index]);

                // The following assumes `bundles` has produced transfers which may have
                // impacted the wallet's addresses.  Thus the recovery must be *serial w.r.t. the
                // subsequent call to `cryptoClientQRYRequestTransactionsOrTransfers()`.

                BRCryptoWallet wallet = cryptoWalletManagerGetWallet(manager);

                // We've completed a query for `oldAddresses`
                BRSetOf(BRCryptoAddress) oldAddresses = callbackState->u.getTransactions.addresses;

                // We'll need another query if `newAddresses` is now larger then `oldAddresses`
                BRSetOf(BRCryptoAddress) newAddresses = cryptoWalletGetAddressesForRecovery (wallet);

                // Make the actual request; if none is needed, then we are done
                if (!cryptoClientQRYRequestTransactionsOrTransfers (qry,
                                                                    CLIENT_CALLBACK_REQUEST_TRANSACTIONS,
                                                                    oldAddresses,
                                                                    newAddresses,
                                                                    callbackState->rid)) {
                    syncCompleted = true;
                    syncSuccess   = true;
                }

                cryptoWalletGive (wallet);
                break;

            case CRYPTO_FALSE:
                syncCompleted = true;
                syncSuccess   = false;
                break;
            }
        }
    }

    cryptoClientQRYManagerUpdateSync (qry, syncCompleted, syncSuccess, true);

    array_free_all (bundles, cryptoClientTransactionBundleRelease);
    cryptoClientCallbackStateRelease(callbackState);
}

static void
cryptoClientAnnounceTransactionsDispatcher (BREventHandler ignore,
                                            BRCryptoClientAnnounceTransactionsEvent *event) {
    cryptoClientHandleTransactions (event->manager,
                                    event->callbackState,
                                    event->success,
                                    event->bundles);
}

static void
cryptoClientAnnounceTransactionsDestroyer (BRCryptoClientAnnounceTransactionsEvent *event) {
    cryptoWalletManagerGive (event->manager);
    cryptoClientCallbackStateRelease (event->callbackState);
    array_free_all (event->bundles, cryptoClientTransactionBundleRelease);
}

BREventType handleClientAnnounceTransactionsEventType = {
    "CWM: Handle Client Announce Transactions Event",
    sizeof (BRCryptoClientAnnounceTransactionsEvent),
    (BREventDispatcher) cryptoClientAnnounceTransactionsDispatcher,
    (BREventDestroyer)  cryptoClientAnnounceTransactionsDestroyer
};

extern void
cryptoClientAnnounceTransactions (OwnershipKept BRCryptoWalletManager manager,
                                  OwnershipGiven BRCryptoClientCallbackState callbackState,
                                  BRCryptoBoolean success,
                                  BRCryptoClientTransactionBundle *bundles,  // given elements, not array
                                  size_t bundlesCount) {
    BRArrayOf (BRCryptoClientTransactionBundle) eventBundles;
    array_new (eventBundles, bundlesCount);
    array_add_array (eventBundles, bundles, bundlesCount);

    BRCryptoClientAnnounceTransactionsEvent event =
    { { NULL, &handleClientAnnounceTransactionsEventType },
        cryptoWalletManagerTakeWeak(manager),
        callbackState,
        success,
        eventBundles };

    eventHandlerSignalEvent (manager->handler, (BREvent *) &event);
}

// MARK: - Announce Transfer

typedef struct {
    BREvent base;
    BRCryptoWalletManager manager;
    BRCryptoClientCallbackState callbackState;
    BRCryptoBoolean success;
    BRArrayOf (BRCryptoClientTransferBundle) bundles;
} BRCryptoClientAnnounceTransfersEvent;

extern void
cryptoClientHandleTransfers (OwnershipKept BRCryptoWalletManager manager,
                                OwnershipGiven BRCryptoClientCallbackState callbackState,
                                BRCryptoBoolean success,
                                BRArrayOf (BRCryptoClientTransferBundle) bundles) {


    BRCryptoClientQRYManager qry = manager->qryManager;

    pthread_mutex_lock (&qry->lock);
    bool matchedRids = (callbackState->rid == qry->sync.rid);
    pthread_mutex_unlock (&qry->lock);

    bool syncCompleted = false;
    bool syncSuccess   = false;

    // Process the results if the bundles are for our rid; otherwise simply discard;
    if (matchedRids) {
        switch (success) {
            case CRYPTO_TRUE: {
                size_t bundlesCount = array_count(bundles);

                for (size_t index = 0; index < bundlesCount; index++)
                    cryptoWalletManagerSaveTransferBundle(manager, bundles[index]);

                // Sort bundles to have the lowest blocknumber first.  Use of `mergesort` is
                // appropriate given that the bundles are likely already ordered.  This minimizes
                // dependency resolution between later transfers depending on prior transfers.

                mergesort_brd (bundles, bundlesCount, sizeof (BRCryptoClientTransferBundle),
                               cryptoClientTransferBundleCompareForSort);

                // Recover transfers from each bundle
                for (size_t index = 0; index < bundlesCount; index++)
                   cryptoWalletManagerRecoverTransferFromTransferBundle (manager, bundles[index]);

                BRCryptoWallet wallet = cryptoWalletManagerGetWallet(manager);

                // We've completed a query for `oldAddresses`
                BRSetOf(BRCryptoAddress) oldAddresses = callbackState->u.getTransactions.addresses;

                // We'll need another query if `newAddresses` is now larger then `oldAddresses`
                BRSetOf(BRCryptoAddress) newAddresses = cryptoWalletGetAddressesForRecovery (wallet);

                // Make the actual request; if none is needed, then we are done.  Use the
                // same `rid` as we are in the same sync.
                if (!cryptoClientQRYRequestTransactionsOrTransfers (qry,
                                                                    CLIENT_CALLBACK_REQUEST_TRANSFERS,
                                                                    oldAddresses,
                                                                    newAddresses,
                                                                    callbackState->rid)) {
                    syncCompleted = true;
                    syncSuccess   = true;
                }

                cryptoWalletGive (wallet);
                break;

            case CRYPTO_FALSE:
                syncCompleted = true;
                syncSuccess   = false;
                break;
            }
        }
    }

    cryptoClientQRYManagerUpdateSync (qry, syncCompleted, syncSuccess, true);

    array_free_all (bundles, cryptoClientTransferBundleRelease);
    cryptoClientCallbackStateRelease(callbackState);
}

static void
cryptoClientAnnounceTransfersDispatcher (BREventHandler ignore,
                                            BRCryptoClientAnnounceTransfersEvent *event) {
    cryptoClientHandleTransfers (event->manager,
                                    event->callbackState,
                                    event->success,
                                    event->bundles);
}

static void
cryptoClientAnnounceTransfersDestroyer (BRCryptoClientAnnounceTransfersEvent *event) {
    cryptoWalletManagerGive (event->manager);
    cryptoClientCallbackStateRelease (event->callbackState);
    array_free_all (event->bundles, cryptoClientTransferBundleRelease);
}

BREventType handleClientAnnounceTransfersEventType = {
    "CWM: Handle Client Announce Transfers Event",
    sizeof (BRCryptoClientAnnounceTransfersEvent),
    (BREventDispatcher) cryptoClientAnnounceTransfersDispatcher,
    (BREventDestroyer)  cryptoClientAnnounceTransfersDestroyer
};


extern void
cryptoClientAnnounceTransfers (OwnershipKept BRCryptoWalletManager manager,
                               OwnershipGiven BRCryptoClientCallbackState callbackState,
                               BRCryptoBoolean success,
                               OwnershipGiven BRCryptoClientTransferBundle *bundles, // given elements, not array
                               size_t bundlesCount) {
    BRArrayOf (BRCryptoClientTransferBundle) eventBundles;
    array_new (eventBundles, bundlesCount);
    array_add_array (eventBundles, bundles, bundlesCount);

    BRCryptoClientAnnounceTransfersEvent event =
    { { NULL, &handleClientAnnounceTransfersEventType },
        cryptoWalletManagerTakeWeak(manager),
        callbackState,
        success,
        eventBundles };

    eventHandlerSignalEvent (manager->handler, (BREvent *) &event);
}

// MARK: - Request Transactions/Transfers

static BRArrayOf(char *)
cryptoClientQRYGetAddresses (BRCryptoClientQRYManager qry,
                             OwnershipKept BRSetOf(BRCryptoAddress) addresses) {
    BRArrayOf(char *) addressesEncoded;
    array_new (addressesEncoded, BRSetCount (addresses));

    FOR_SET (BRCryptoAddress, address, addresses) {
        array_add (addressesEncoded, cryptoAddressAsString (address));
    }

    return addressesEncoded;
}

static void
cryptoClientQRYReleaseAddresses (BRArrayOf(char *) addresses) {
    for (size_t index = 0; index < array_count(addresses); index++)
    free (addresses[index]);
    array_free (addresses);
}

static bool
cryptoClientQRYRequestTransactionsOrTransfers (BRCryptoClientQRYManager qry,
                                               BRCryptoClientCallbackType type,
                                               OwnershipKept  BRSetOf(BRCryptoAddress) oldAddresses,
                                               OwnershipGiven BRSetOf(BRCryptoAddress) newAddresses,
                                               size_t requestId) {

    BRCryptoWalletManager manager = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == manager) {
        cryptoAddressSetRelease(newAddresses);
        return false;
    }

    // Determine the set of addresses needed as `newAddresses - oldAddresses`.  The elements in
    // `addresses` ARE NOT owned by `addresses`; they remain owned by `newAddresses`
    BRSetOf(BRCryptoAddress) addresses = BRSetCopy (newAddresses, NULL);

    if (NULL != oldAddresses)
        BRSetMinus (addresses, oldAddresses);

    // If there are `addresses` then a reqeust is needed.
    bool needRequest = BRSetCount (addresses) > 0;

    if (needRequest) {
        // Get an array of the remaining, needed `addresses`
        BRArrayOf(char *) addressesEncoded = cryptoClientQRYGetAddresses (qry, addresses);

        // Create a `calllbackState`; importantly, report `newAddress` as the accumulated addresses
        // that have been requested.  Note, this specific request will be for `addresses` only.
        // The elements in `newAddresses` are now owned by `callbackState`.
        BRCryptoClientCallbackState callbackState = cryptoClientCallbackStateCreateGetTrans (type,
                                                                                             newAddresses,
                                                                                             requestId);

        switch (type) {
            case CLIENT_CALLBACK_REQUEST_TRANSFERS:
                qry->client.funcGetTransfers (qry->client.context,
                                              cryptoWalletManagerTake(manager),
                                              callbackState,
                                              (const char **) addressesEncoded,
                                              array_count(addressesEncoded),
                                              qry->sync.begBlockNumber,
                                              (qry->sync.unbounded
                                               ? BLOCK_HEIGHT_UNBOUND_VALUE
                                               : qry->sync.endBlockNumber));
                break;

            case CLIENT_CALLBACK_REQUEST_TRANSACTIONS:
                qry->client.funcGetTransactions (qry->client.context,
                                                 cryptoWalletManagerTake(manager),
                                                 callbackState,
                                                 (const char **) addressesEncoded,
                                                 array_count(addressesEncoded),
                                                 qry->sync.begBlockNumber,
                                                 (qry->sync.unbounded
                                                  ? BLOCK_HEIGHT_UNBOUND_VALUE
                                                  : qry->sync.endBlockNumber));
                break;

            default:
                assert (false);
        }

        cryptoClientQRYReleaseAddresses (addressesEncoded);
    }
    else {
        // If `newAddresses` ownership was not transfered to `callbackState`, then release everything.
        cryptoAddressSetRelease (newAddresses);
    }

    cryptoWalletManagerGive (manager);
    BRSetFree (addresses);

    return needRequest;
}


// MARK: Announce Submit Transfer

typedef struct {
    BREvent base;
    BRCryptoWalletManager manager;
    BRCryptoClientCallbackState callbackState;
    char *identifier;
    char *hash;
    BRCryptoBoolean success;
} BRCryptoClientAnnounceSubmitEvent;

static void
cryptoClientHandleSubmit (OwnershipKept BRCryptoWalletManager manager,
                          OwnershipGiven BRCryptoClientCallbackState callbackState,
                          OwnershipKept char *identifierStr,
                          OwnershipKept char *hashStr,
                          BRCryptoBoolean success) {
    assert (CLIENT_CALLBACK_SUBMIT_TRANSACTION == callbackState->type);

    BRCryptoWallet   wallet   = callbackState->u.submitTransaction.wallet;
    BRCryptoTransfer transfer = callbackState->u.submitTransaction.transfer;

    // Get the transfer state
    BRCryptoTransferState transferState = (CRYPTO_TRUE == success
                                           ? cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SUBMITTED)
                                           : cryptoTransferStateErroredInit (cryptoTransferSubmitErrorUnknown()));

    // Recover the `state` as either SUBMITTED or a UNKNOWN ERROR.  We have a slight issue, as
    // a possible race condition, whereby the transfer can already be INCLUDED by the time this
    // `announce` is called.  That has got to be impossible right?
    //
    // Assign the state; generate events in the process.
    cryptoTransferSetState (transfer, transferState);

    // On successful submit, the hash might be determined.  Yes, somewhat unfathomably (HBAR)
    BRCryptoHash hash = (NULL == hashStr ? NULL : cryptoNetworkCreateHashFromString (manager->network, hashStr));

    if (NULL != hash) {
        BRCryptoBoolean hashChanged = cryptoTransferSetHash (transfer, hash);

        if (CRYPTO_TRUE == hashChanged) {
            BRCryptoTransferState state = cryptoTransferGetState(transfer);

            cryptoTransferGenerateEvent (transfer, (BRCryptoTransferEvent) {
                CRYPTO_TRANSFER_EVENT_CHANGED,
                { .state = {
                    cryptoTransferStateTake (state),
                    cryptoTransferStateTake (state) }}
            });

            cryptoTransferStateGive (state);
        }

        cryptoHashGive (hash);
    }

    // If the manager's wallet (aka the primaryWallet) does not match the wallet, then the
    // transaction fee must be updated... on an ERROR - it was already updated upon submit, so
    // only on ERROR do we undo the fee.
    if (wallet != manager->wallet &&
        CRYPTO_TRANSFER_STATE_ERRORED == transferState->type &&
        CRYPTO_TRANSFER_RECEIVED      != transfer->direction) {
        cryptoWalletUpdBalance (manager->wallet, true);
    }

    cryptoTransferStateGive(transferState);

    cryptoWalletManagerGive (manager);
    cryptoClientCallbackStateRelease(callbackState);
    cryptoMemoryFree (identifierStr);
    cryptoMemoryFree (hashStr);
}

static void
cryptoClientAnnounceSubmitDispatcher (BREventHandler ignore,
                                      BRCryptoClientAnnounceSubmitEvent *event) {
    cryptoClientHandleSubmit (event->manager,
                              event->callbackState,
                              event->identifier,
                              event->hash,
                              event->success);
}

static void
cryptoClientAnnounceSubmitDestroyer (BRCryptoClientAnnounceSubmitEvent *event) {
    cryptoWalletManagerGive (event->manager);
    cryptoClientCallbackStateRelease (event->callbackState);
    cryptoMemoryFree (event->identifier);
    cryptoMemoryFree (event->hash);
}

BREventType handleClientAnnounceSubmitEventType = {
    "CWM: Handle Client Announce Submit Event",
    sizeof (BRCryptoClientAnnounceSubmitEvent),
    (BREventDispatcher) cryptoClientAnnounceSubmitDispatcher,
    (BREventDestroyer)  cryptoClientAnnounceSubmitDestroyer
};

extern void
cryptoClientAnnounceSubmitTransfer (OwnershipKept BRCryptoWalletManager manager,
                                    OwnershipGiven BRCryptoClientCallbackState callbackState,
                                    OwnershipKept const char *identifier,
                                    OwnershipKept const char *hash,
                                    BRCryptoBoolean success) {
    BRCryptoClientAnnounceSubmitEvent event =
    { { NULL, &handleClientAnnounceSubmitEventType },
        cryptoWalletManagerTakeWeak(manager),
        callbackState,
        (NULL == identifier ? NULL : strdup(identifier)),
        (NULL == hash ? NULL : strdup (hash)),
        success };

    eventHandlerSignalEvent (manager->handler, (BREvent *) &event);
}

static void
cryptoClientQRYSubmitTransfer (BRCryptoClientQRYManager qry,
                               BRCryptoWallet   wallet,
                               BRCryptoTransfer transfer) {
    BRCryptoWalletManager manager = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == manager) return;

    size_t   serializationCount;
    uint8_t *serialization = cryptoTransferSerializeForSubmission (transfer,
                                                                   manager->network,
                                                                   &serializationCount);

    BRCryptoClientCallbackState callbackState =
    cryptoClientCallbackStateCreateSubmitTransaction (wallet, transfer, qry->requestId++);

    qry->client.funcSubmitTransaction (qry->client.context,
                                       manager,
                                       callbackState,
                                       cryptoTransferGetIdentifier(transfer),
                                       serialization,
                                       serializationCount);

    free (serialization);
}

// MARK: - Announce Estimate Transaction Fee

typedef struct {
    BREvent base;
    BRCryptoWalletManager manager;
    BRCryptoClientCallbackState callbackState;
    BRCryptoBoolean success;
    uint64_t costUnits;
    BRArrayOf(char*) keys;
    BRArrayOf(char*) vals;
} BRCryptoClientAnnounceEstimateTransactionFeeEvent;

static void
cryptoClientHandleEstimateTransactionFee (OwnershipKept BRCryptoWalletManager manager,
                                          OwnershipGiven BRCryptoClientCallbackState callbackState,
                                          BRCryptoBoolean success,
                                          uint64_t costUnits,
                                          BRArrayOf(char*) attributeKeys,
                                          BRArrayOf(char*) attributeVals) {
    assert (CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE == callbackState->type);

    BRCryptoStatus status = (CRYPTO_TRUE == success ? CRYPTO_SUCCESS : CRYPTO_ERROR_FAILED);
    BRCryptoCookie cookie = callbackState->u.estimateTransactionFee.cookie;

    BRCryptoNetworkFee networkFee = callbackState->u.estimateTransactionFee.networkFee;
    BRCryptoFeeBasis initialFeeBasis = callbackState->u.estimateTransactionFee.initialFeeBasis;

    BRCryptoAmount pricePerCostFactor = cryptoNetworkFeeGetPricePerCostFactor (networkFee);
    double costFactor = (double) costUnits;
    BRCryptoFeeBasis feeBasis = NULL;
    if (CRYPTO_TRUE == success)
        feeBasis = cryptoWalletManagerRecoverFeeBasisFromFeeEstimate (manager,
                                                                      networkFee,
                                                                      initialFeeBasis,
                                                                      costFactor,
                                                                      array_count(attributeKeys),
                                                                      (const char**) attributeKeys,
                                                                      (const char**) attributeVals);

    cryptoWalletGenerateEvent (manager->wallet, cryptoWalletEventCreateFeeBasisEstimated(status, cookie, feeBasis));

    cryptoFeeBasisGive(feeBasis);
    cryptoAmountGive (pricePerCostFactor);
    cryptoClientCallbackStateRelease (callbackState);
}

static void
cryptoClientAnnounceEstimateTransactionFeeDispatcher (BREventHandler ignore,
                                      BRCryptoClientAnnounceEstimateTransactionFeeEvent *event) {
    cryptoClientHandleEstimateTransactionFee (event->manager,
                                              event->callbackState,
                                              event->success,
                                              event->costUnits,
                                              event->keys,
                                              event->vals);
}

static void
cryptoClientAnnounceEstimateTransactionFeeDestroyer (BRCryptoClientAnnounceEstimateTransactionFeeEvent *event) {
    cryptoWalletManagerGive (event->manager);
    cryptoClientCallbackStateRelease (event->callbackState);
    array_free_all (event->keys, cryptoMemoryFree);
    array_free_all (event->vals, cryptoMemoryFree);
}

BREventType handleClientAnnounceEstimateTransactionFeeEventType = {
    "CWM: Handle Client Announce EstimateTransactionFee Event",
    sizeof (BRCryptoClientAnnounceEstimateTransactionFeeEvent),
    (BREventDispatcher) cryptoClientAnnounceEstimateTransactionFeeDispatcher,
    (BREventDestroyer)  cryptoClientAnnounceEstimateTransactionFeeDestroyer
};

extern void
cryptoClientAnnounceEstimateTransactionFee (OwnershipKept BRCryptoWalletManager manager,
                                            OwnershipGiven BRCryptoClientCallbackState callbackState,
                                            BRCryptoBoolean success,
                                            uint64_t costUnits,
                                            size_t attributesCount,
                                            OwnershipKept const char **attributeKeys,
                                            OwnershipKept const char **attributeVals) {
    BRArrayOf(char *) keys;
    array_new (keys, attributesCount);
    array_add_array (keys, (char**) attributeKeys, attributesCount);

    BRArrayOf(char *) vals;
    array_new (vals, attributesCount);
    array_add_array (vals, (char**) attributeVals, attributesCount);

    for (size_t index = 0; index < attributesCount; index++) {
        keys[index] = strdup (keys[index]);
        vals[index] = strdup (vals[index]);
    }

    BRCryptoClientAnnounceEstimateTransactionFeeEvent event =
    { { NULL, &handleClientAnnounceEstimateTransactionFeeEventType },
        cryptoWalletManagerTakeWeak(manager),
        callbackState,
        success,
        costUnits,
        keys,
        vals };

    eventHandlerSignalEvent (manager->handler, (BREvent *) &event);

}

extern void
cryptoClientQRYEstimateTransferFee (BRCryptoClientQRYManager qry,
                                    BRCryptoCookie   cookie,
                                    OwnershipKept BRCryptoTransfer transfer,
                                    OwnershipKept BRCryptoNetworkFee networkFee,
                                    OwnershipKept BRCryptoFeeBasis initialFeeBasis) {
    BRCryptoWalletManager manager = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == manager) return;

    size_t   serializationCount;
    uint8_t *serialization = cryptoTransferSerializeForFeeEstimation (transfer, manager->network, &serializationCount);

    // There is no hash... transfer is not guaranteed to be signed; likely unsigned.
    BRCryptoHash hash      = NULL;
    const char  *hashAsHex = "";

    pthread_mutex_lock (&qry->lock);
    size_t rid = qry->requestId++;
    pthread_mutex_unlock (&qry->lock);

    BRCryptoClientCallbackState callbackState = cryptoClientCallbackStateCreateEstimateTransactionFee (hash,
                                                                                                       cookie,
                                                                                                       networkFee,
                                                                                                       initialFeeBasis,
                                                                                                       rid);

    qry->client.funcEstimateTransactionFee (qry->client.context,
                                            cryptoWalletManagerTake(manager),
                                            callbackState,
                                            serialization,
                                            serializationCount,
                                            hashAsHex);
}

// MARK: - Transfer Bundle

extern BRCryptoClientTransferBundle
cryptoClientTransferBundleCreate (BRCryptoTransferStateType status,
                                  OwnershipKept const char */* transaction */ hash,
                                  OwnershipKept const char */* transaction */ identifier,
                                  OwnershipKept const char */* transfer */ uids,
                                  OwnershipKept const char *from,
                                  OwnershipKept const char *to,
                                  OwnershipKept const char *amount,
                                  OwnershipKept const char *currency,
                                  OwnershipKept const char *fee,
                                  uint64_t transferIndex,
                                  uint64_t blockTimestamp,
                                  uint64_t blockNumber,
                                  uint64_t blockConfirmations,
                                  uint64_t blockTransactionIndex,
                                  OwnershipKept const char *blockHash,
                                  size_t attributesCount,
                                  OwnershipKept const char **attributeKeys,
                                  OwnershipKept const char **attributeVals) {
    BRCryptoClientTransferBundle bundle = calloc (1, sizeof (struct BRCryptoClientTransferBundleRecord));

    // In the case of an error, as indicated by `status`,  we've got no additional information
    // as to the error type.  The transfer/transaction is in the blockchain, presumably it has
    // resulted in a fee paid but no transfer of the desired asset.  A User would be expected to
    // recover in some blockchain specific way - and hopefully can recognize the cause of the error
    // so as to avoid simply creating a transaction to cause it again.

    bundle->status     = status;
    bundle->hash       = strdup (hash);
    bundle->identifier = strdup (identifier);
    bundle->uids       = strdup (uids);
    bundle->from     = strdup (from);
    bundle->to       = strdup (to);
    bundle->amount   = strdup (amount);
    bundle->currency = strdup (currency);
    bundle->fee      = NULL == fee ? NULL : strdup (fee);

    bundle->transferIndex = transferIndex;
    bundle->blockTimestamp = blockTimestamp;
    bundle->blockNumber    = blockNumber;
    bundle->blockConfirmations    = blockConfirmations;
    bundle->blockTransactionIndex = blockTransactionIndex;
    bundle->blockHash = strdup (blockHash);

    // attributes
    bundle->attributesCount = attributesCount;
    bundle->attributeKeys = bundle->attributeVals = NULL;

    if (bundle->attributesCount > 0) {
        bundle->attributeKeys = calloc (bundle->attributesCount, sizeof (char*));
        bundle->attributeVals = calloc (bundle->attributesCount, sizeof (char*));
        for (size_t index = 0; index < bundle->attributesCount; index++) {
            bundle->attributeKeys[index] = strdup (attributeKeys[index]);
            bundle->attributeVals[index] = strdup (attributeVals[index]);
        }
    }

    return bundle;
}

extern void
cryptoClientTransferBundleRelease (BRCryptoClientTransferBundle bundle) {
    //
    free (bundle->uids);
    free (bundle->hash);
    free (bundle->identifier);
    free (bundle->from);
    free (bundle->to);
    free (bundle->amount);
    free (bundle->currency);
    if (NULL != bundle->fee) free (bundle->fee);

    free (bundle->blockHash);

    if (bundle->attributesCount > 0) {
        for (size_t index = 0; index < bundle->attributesCount; index++) {
            free (bundle->attributeKeys[index]);
            free (bundle->attributeVals[index]);
        }
        free (bundle->attributeKeys);
        free (bundle->attributeVals);
    }
    
    memset (bundle, 0, sizeof (struct BRCryptoClientTransferBundleRecord));
    free (bundle);
}

static int
cryptoClientStringCompare (const char *s1, const char *s2) {
    int comparison = strcmp (s1, s2);

    return (comparison > 0
            ? +1
            : (comparison < 0
               ? -1
               :  0));
}
extern int
cryptoClientTransferBundleCompare (const BRCryptoClientTransferBundle b1,
                                   const BRCryptoClientTransferBundle b2) {
    return (b1->blockNumber < b2->blockNumber
            ? -1
            : (b1->blockNumber > b2->blockNumber
               ? +1
               :  (b1->blockTransactionIndex < b2->blockTransactionIndex
                   ? -1
                   : (b1->blockTransactionIndex > b2->blockTransactionIndex
                      ? +1
                      : (b1->transferIndex < b2->transferIndex
                         ? -1
                         : (b1->transferIndex > b2->transferIndex
                            ? +1
                            :  cryptoClientStringCompare (b1->uids, b2->uids)))))));
}

extern int
cryptoClientTransferBundleCompareByBlockheight (const BRCryptoClientTransferBundle b1,
                                                const BRCryptoClientTransferBundle b2) {
    return (b1->blockNumber < b2-> blockNumber
            ? -1
            : (b1->blockNumber > b2->blockNumber
               ? +1
               :  0));
}

extern BRCryptoTransferState
cryptoClientTransferBundleGetTransferState (const BRCryptoClientTransferBundle bundle,
                                            BRCryptoFeeBasis confirmedFeeBasis) {
    bool isIncluded = (CRYPTO_TRANSFER_STATE_INCLUDED == bundle->status ||    // success
                       (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status &&    // error
                        BLOCK_HEIGHT_UNBOUND != bundle->blockNumber &&
                        0 != bundle->blockTimestamp));

    return (isIncluded
            ? cryptoTransferStateIncludedInit (bundle->blockNumber,
                                               bundle->blockTransactionIndex,
                                               bundle->blockTimestamp,
                                               confirmedFeeBasis,
                                               AS_CRYPTO_BOOLEAN(CRYPTO_TRANSFER_STATE_INCLUDED == bundle->status),
                                               (isIncluded ? NULL : "unknown"))
            : (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status
               ? cryptoTransferStateErroredInit (cryptoTransferSubmitErrorUnknown())
               : cryptoTransferStateInit (bundle->status)));
}

static BRRlpItem
cryptoClientTransferBundleRlpEncodeAttributes (size_t count,
                                               const char **keys,
                                               const char **vals,
                                               BRRlpCoder coder) {
    BRRlpItem items[count];

    for (size_t index = 0; index < count; index++)
    items[index] = rlpEncodeList2 (coder,
                                   rlpEncodeString (coder, keys[index]),
                                   rlpEncodeString (coder, vals[index]));

    return rlpEncodeListItems (coder, items, count);
}

typedef struct {
    BRArrayOf(char*) keys;
    BRArrayOf(char*) vals;
} BRCryptoTransferBundleRlpDecodeAttributesResult;

static BRCryptoTransferBundleRlpDecodeAttributesResult
cryptoClientTransferBundleRlpDecodeAttributes (BRRlpItem item,
                                               BRRlpCoder coder) {
    size_t itemsCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);

    BRArrayOf(char*) keys;
    array_new (keys, itemsCount);

    BRArrayOf(char*) vals;
    array_new (vals, itemsCount);

    for (size_t index = 0; index < itemsCount; index++) {
        size_t count;
        const BRRlpItem *pair = rlpDecodeList (coder, items[index], &count);
        assert (2 == count);

        array_add (keys, rlpDecodeString (coder, pair[0]));
        array_add (vals, rlpDecodeString (coder, pair[1]));
    }

    return (BRCryptoTransferBundleRlpDecodeAttributesResult) { keys, vals };
}

private_extern BRRlpItem
cryptoClientTransferBundleRlpEncode (BRCryptoClientTransferBundle bundle,
                                     BRRlpCoder coder) {

    return rlpEncodeList (coder, 16,
                          rlpEncodeUInt64 (coder, bundle->status, 0),
                          rlpEncodeString (coder, bundle->uids),
                          rlpEncodeString (coder, bundle->hash),
                          rlpEncodeString (coder, bundle->identifier),
                          rlpEncodeString (coder, bundle->from),
                          rlpEncodeString (coder, bundle->to),
                          rlpEncodeString (coder, bundle->amount),
                          rlpEncodeString (coder, bundle->currency),
                          rlpEncodeString (coder, bundle->fee),
                          rlpEncodeUInt64 (coder, bundle->blockTimestamp,        0),
                          rlpEncodeUInt64 (coder, bundle->blockNumber,           0),
                          rlpEncodeUInt64 (coder, bundle->blockConfirmations,    0),
                          rlpEncodeUInt64 (coder, bundle->blockTransactionIndex, 0),
                          rlpEncodeString (coder, bundle->blockHash),
                          cryptoClientTransferBundleRlpEncodeAttributes (bundle->attributesCount,
                                                                         (const char **) bundle->attributeKeys,
                                                                         (const char **) bundle->attributeVals,
                                                                         coder),
                          rlpEncodeUInt64 (coder, bundle->transferIndex,         0));
}

private_extern BRCryptoClientTransferBundle
cryptoClientTransferBundleRlpDecode (BRRlpItem item,
                                     BRRlpCoder coder,
                                     BRCryptoFileServiceTransferVersion version) {
    size_t itemsCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);

    switch (version) {
        case CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_1:
            assert (15 == itemsCount);
            break;
        case CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_2:
            assert (16 == itemsCount);
            break;
        default:
            break;
    }

    char *uids     = rlpDecodeString (coder, items[ 1]);
    char *hash     = rlpDecodeString (coder, items[ 2]);
    char *ident    = rlpDecodeString (coder, items[ 3]);
    char *from     = rlpDecodeString (coder, items[ 4]);
    char *to       = rlpDecodeString (coder, items[ 5]);
    char *amount   = rlpDecodeString (coder, items[ 6]);
    char *currency = rlpDecodeString (coder, items[ 7]);
    char *fee      = rlpDecodeString (coder, items[ 8]);

    uint64_t blockTimestamp        = rlpDecodeUInt64 (coder, items[ 9], 0);
    uint64_t blockNumber           = rlpDecodeUInt64 (coder, items[10], 0);
    uint64_t blockConfirmations    = rlpDecodeUInt64 (coder, items[11], 0);
    uint64_t blockTransactionIndex = rlpDecodeUInt64 (coder, items[12], 0);

    char *blockHash  = rlpDecodeString (coder, items[13]);

    BRCryptoTransferBundleRlpDecodeAttributesResult attributesResult =
    cryptoClientTransferBundleRlpDecodeAttributes (items[14], coder);

    // Set the transferIndex to a default value.
    uint64_t transferIndex = 0;

    switch (version) {
        case CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_1:
            // derive the transferIndex from the UIDS
            if (NULL != uids) {
                char *sepPtr = strrchr (uids, ':');    // "<network>:<hash>:<index>" for Blockset only!

                if (NULL != sepPtr) {
                    char *endPtr = NULL;
                    unsigned long value = strtoul (sepPtr + 1, &endPtr, 10);

                    if ('\0' == endPtr[0])
                        transferIndex = (uint64_t) value;
                }
            }
            break;
        case CRYPTO_FILE_SERVICE_TYPE_TRANSFER_VERSION_2:
            transferIndex = rlpDecodeUInt64 (coder, items[15], 0);
            break;
        default:
            break;
    }

    BRCryptoClientTransferBundle bundle =
    cryptoClientTransferBundleCreate ((BRCryptoTransferStateType) rlpDecodeUInt64 (coder, items[ 0], 0),
                                      hash,
                                      ident,
                                      uids,
                                      from,
                                      to,
                                      amount,
                                      currency,
                                      (0 == strcmp(fee,"") ? NULL : fee),
                                      transferIndex,
                                      blockTimestamp,
                                      blockNumber,
                                      blockConfirmations,
                                      blockTransactionIndex,
                                      blockHash,
                                      array_count(attributesResult.keys),
                                      (const char **) attributesResult.keys,
                                      (const char **) attributesResult.vals);

    free (blockHash);
    free (fee);
    free (currency);
    free (amount);
    free (to);
    free (from);
    free (uids);
    free (hash);

    array_free_all (attributesResult.keys, free);
    array_free_all (attributesResult.vals, free);

    return bundle;
}

private_extern size_t
cryptoClientTransferBundleGetHashValue (BRCryptoClientTransferBundle bundle) {
    uint8_t md16[16];
    BRMD5 (md16, bundle->uids, strlen(bundle->uids));
    return *((size_t *) md16);
}

private_extern int
cryptoClientTransferBundleIsEqual (BRCryptoClientTransferBundle bundle1,
                                   BRCryptoClientTransferBundle bundle2) {
    return 0 == strcmp (bundle1->uids, bundle2->uids);
}

// MARK: - Transaction Bundle

extern BRCryptoClientTransactionBundle
cryptoClientTransactionBundleCreate (BRCryptoTransferStateType status,
                                     OwnershipKept uint8_t *transaction,
                                     size_t transactionLength,
                                     BRCryptoTimestamp timestamp,
                                     BRCryptoBlockNumber blockHeight) {
    BRCryptoClientTransactionBundle bundle = calloc (1, sizeof (struct BRCryptoClientTransactionBundleRecord));

    bundle->status = status;

    bundle->serialization = malloc(transactionLength);
    memcpy (bundle->serialization, transaction, transactionLength);
    bundle->serializationCount = transactionLength;

    bundle->timestamp   = timestamp;
    bundle->blockHeight = (BRCryptoBlockNumber) blockHeight;

    return bundle;
}

extern void
cryptoClientTransactionBundleRelease (BRCryptoClientTransactionBundle bundle) {
    free (bundle->serialization);
    memset (bundle, 0, sizeof (struct BRCryptoClientTransactionBundleRecord));
    free (bundle);
}

extern int
cryptoClientTransactionBundleCompare (const BRCryptoClientTransactionBundle b1,
                                      const BRCryptoClientTransactionBundle b2) {
    return cryptoClientTransactionBundleCompareByBlockheight (b1, b2);
}

extern int
cryptoClientTransactionBundleCompareByBlockheight (const BRCryptoClientTransactionBundle b1,
                                                   const BRCryptoClientTransactionBundle b2) {
    return (b1->blockHeight < b2-> blockHeight
            ? -1
            : (b1->blockHeight > b2->blockHeight
               ? +1
               :  0));
}

private_extern OwnershipKept uint8_t *
cryptoClientTransactionBundleGetSerialization (BRCryptoClientTransactionBundle bundle,
                                               size_t *serializationCount) {
    *serializationCount = bundle->serializationCount;
    return bundle->serialization;
}

private_extern BRRlpItem
cryptoClientTransactionBundleRlpEncode (BRCryptoClientTransactionBundle bundle,
                                        BRRlpCoder coder) {
    return rlpEncodeList (coder, 4,
                          rlpEncodeUInt64 (coder, bundle->status,      0),
                          rlpEncodeBytes  (coder, bundle->serialization, bundle->serializationCount),
                          rlpEncodeUInt64 (coder, bundle->timestamp,   0),
                          rlpEncodeUInt64 (coder, bundle->blockHeight, 0));
}

private_extern BRCryptoClientTransactionBundle
cryptoClientTransactionBundleRlpDecode (BRRlpItem item,
                                        BRRlpCoder coder) {
    size_t itemsCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);
    assert (4 == itemsCount);

    BRRlpData serializationData = rlpDecodeBytesSharedDontRelease (coder, items[1]);

    return cryptoClientTransactionBundleCreate ((BRCryptoTransferStateType) rlpDecodeUInt64 (coder, items[0], 0),
                                                serializationData.bytes,
                                                serializationData.bytesCount,
                                                rlpDecodeUInt64(coder, items[2], 0),
                                                rlpDecodeUInt64(coder, items[3], 0));
}

private_extern size_t
cryptoClientTransactionBundleGetHashValue (BRCryptoClientTransactionBundle bundle) {
    uint8_t md16[16];
    BRMD5 (md16, bundle->serialization, bundle->serializationCount);
    return *((size_t *) md16);
}

private_extern int
cryptoClientTransactionBundleIsEqual (BRCryptoClientTransactionBundle bundle1,
                                      BRCryptoClientTransactionBundle bundle2) {
    return (bundle1->status             == bundle2->status             &&
            bundle1->timestamp          == bundle2->timestamp          &&
            bundle1->blockHeight        == bundle2->blockHeight        &&
            bundle1->serializationCount == bundle2->serializationCount &&
            0 == memcmp (bundle1->serialization, bundle2->serialization, bundle1->serializationCount));
}

// MARK: - Currency, CurrencyDenomination Bundle

static BRCryptoClientCurrencyDenominationBundle
cryptoClientCurrencyDenominationBundleCreateInternal (OwnershipGiven char *name,
                                                      OwnershipGiven char *code,
                                                      OwnershipGiven char *symbol,
                                                      uint8_t     decimals) {
    BRCryptoClientCurrencyDenominationBundle bundle = malloc (sizeof (struct BRCryptoCliehtCurrencyDenominationBundleRecord));

    bundle->name = name,
    bundle->code = code;
    bundle->symbol = symbol;
    bundle->decimals = decimals;

    return bundle;
}

extern BRCryptoClientCurrencyDenominationBundle
cryptoClientCurrencyDenominationBundleCreate (const char *name,
                                              const char *code,
                                              const char *symbol,
                                              uint8_t     decimals) {
    return cryptoClientCurrencyDenominationBundleCreateInternal (strdup (name),
                                                                 strdup (code),
                                                                 strdup (symbol),
                                                                 decimals);
}

static void
cryptoClientCurrencyDenominationBundleRelease (BRCryptoClientCurrencyDenominationBundle bundle) {
    free (bundle->symbol);
    free (bundle->code);
    free (bundle->name);

    memset (bundle, 0, sizeof (struct BRCryptoCliehtCurrencyDenominationBundleRecord));
    free (bundle);
}

private_extern BRRlpItem
cryptoClientCurrencyDenominationBundleRlpEncode (BRCryptoClientCurrencyDenominationBundle bundle,
                                                 BRRlpCoder coder) {
    return rlpEncodeList (coder, 4,
                          rlpEncodeString (coder, bundle->name),
                          rlpEncodeString (coder, bundle->code),
                          rlpEncodeString (coder, bundle->symbol),
                          rlpEncodeUInt64 (coder, bundle->decimals, 0));
}

static BRRlpItem
cryptoClientCurrencyDenominationBundlesRlpEncode (BRArrayOf (BRCryptoClientCurrencyDenominationBundle) bundles,
                                                 BRRlpCoder coder) {
    size_t itemsCount = array_count(bundles);
    BRRlpItem items[itemsCount];

    for (size_t index = 0; index < itemsCount; index++)
        items[index] = cryptoClientCurrencyDenominationBundleRlpEncode (bundles[index], coder);

    return rlpEncodeListItems (coder, items, itemsCount);
}

private_extern BRCryptoClientCurrencyDenominationBundle
cryptoClientCurrencyDenominationBundleRlpDecode (BRRlpItem item,
                                                 BRRlpCoder coder) {
    size_t itemsCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);
    assert (4 == itemsCount);

    return cryptoClientCurrencyDenominationBundleCreateInternal (rlpDecodeString (coder, items[0]),
                                                                 rlpDecodeString (coder, items[1]),
                                                                 rlpDecodeString (coder, items[2]),
                                                                 rlpDecodeUInt64 (coder, items[3], 0));
}

static BRArrayOf (BRCryptoClientCurrencyDenominationBundle)
cryptoClientCurrencyDenominationBundlesRlpDecode (BRRlpItem item,
                                                 BRRlpCoder coder) {
    size_t itemsCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);

    BRArrayOf (BRCryptoClientCurrencyDenominationBundle) bundles;
    array_new (bundles, itemsCount);

    for (size_t index = 0; index < itemsCount; index++)
        array_add (bundles, cryptoClientCurrencyDenominationBundleRlpDecode (items[index], coder));

    return bundles;
}


static BRCryptoClientCurrencyBundle
cryptoClientCurrencyBundleCreateInternal (OwnershipGiven char *id,
                                          OwnershipGiven char *name,
                                          OwnershipGiven char *code,
                                          OwnershipGiven char *type,
                                          OwnershipGiven char *blockchainId,
                                          OwnershipGiven char *address,
                                          bool verified,
                                          OwnershipGiven BRArrayOf(BRCryptoClientCurrencyDenominationBundle) denominations) {
    BRCryptoClientCurrencyBundle bundle = malloc (sizeof (struct BRCryptoClientCurrencyBundleRecord));

    bundle->id = id;
    bundle->name = name;
    bundle->code = code;
    bundle->type = type;
    bundle->bid  = blockchainId;
    bundle->address   = address;
    bundle->verfified = verified;
    bundle->denominations = denominations;

    return bundle;
}

extern BRCryptoClientCurrencyBundle
cryptoClientCurrencyBundleCreate (const char *id,
                                  const char *name,
                                  const char *code,
                                  const char *type,
                                  const char *blockchainId,
                                  const char *address,
                                  bool verified,
                                  size_t denominationsCount,
                                  OwnershipGiven BRCryptoClientCurrencyDenominationBundle *denominations) {

    BRArrayOf(BRCryptoClientCurrencyDenominationBundle) arrayOfDenominations;
    array_new (arrayOfDenominations, denominationsCount);
    array_add_array (arrayOfDenominations, denominations, denominationsCount);

    return cryptoClientCurrencyBundleCreateInternal (strdup (id),
                                                     strdup(name),
                                                     strdup(code),
                                                     strdup(type),
                                                     strdup(blockchainId),
                                                     (NULL == address ? NULL : strdup (address)),
                                                     verified,
                                                     arrayOfDenominations);
}

extern void
cryptoClientCurrencyBundleRelease (BRCryptoClientCurrencyBundle bundle) {
    array_free_all (bundle->denominations, cryptoClientCurrencyDenominationBundleRelease);

    if (bundle->address) free (bundle->address);
    free (bundle->bid);
    free (bundle->type);
    free (bundle->code);
    free (bundle->name);
    free (bundle->id);

    memset (bundle, 0, sizeof (struct BRCryptoClientCurrencyBundleRecord));
    free (bundle);
}

static size_t
cryptoClientCurrencyBundleGetHashValue (BRCryptoClientCurrencyBundle bundle) {
    UInt256 identifier;
    BRSHA256(identifier.u8, bundle->id, strlen(bundle->id));
    return (8 == sizeof(size_t)
            ? identifier.u64[0]
            : identifier.u32[0]);
}

// For BRSet
static int
cryptoClientCurrencyBundleIsEqual (BRCryptoClientCurrencyBundle bundle1,
                                    BRCryptoClientCurrencyBundle bundle2) {
    return 0 == strcmp (bundle1->id, bundle2->id);
}

extern OwnershipGiven BRSetOf(BRCryptoClientCurrencyBundle)
cryptoClientCurrencyBundleSetCreate (size_t capacity) {
    return BRSetNew ((size_t (*) (const void *)) cryptoClientCurrencyBundleGetHashValue,
                     (int (*) (const void *, const void *)) cryptoClientCurrencyBundleIsEqual,
                     capacity);
}

extern void
cryptoClientCurrencyBundleSetRelease (OwnershipGiven BRSetOf(BRCryptoClientCurrencyBundle) bundles) {
    BRSetFreeAll (bundles, (void (*) (void *)) cryptoClientCurrencyBundleRelease);
}

private_extern BRRlpItem
cryptoClientCurrencyBundleRlpEncode (BRCryptoClientCurrencyBundle bundle,
                                     BRRlpCoder coder) {
    return rlpEncodeList (coder, 8,
                          rlpEncodeString (coder, bundle->id),
                          rlpEncodeString (coder, bundle->name),
                          rlpEncodeString (coder, bundle->code),
                          rlpEncodeString (coder, bundle->type),
                          rlpEncodeString (coder, bundle->bid),
                          rlpEncodeString (coder, bundle->address),
                          rlpEncodeUInt64 (coder, bundle->verfified, 0),
                          cryptoClientCurrencyDenominationBundlesRlpEncode (bundle->denominations, coder));
}

private_extern BRCryptoClientCurrencyBundle
cryptoClientCurrencyBundleRlpDecode (BRRlpItem item,
                                     BRRlpCoder coder) {
    size_t itemsCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);
    assert (8 == itemsCount);

    return cryptoClientCurrencyBundleCreateInternal (rlpDecodeString (coder, items[0]),
                                                     rlpDecodeString (coder, items[1]),
                                                     rlpDecodeString (coder, items[2]),
                                                     rlpDecodeString (coder, items[3]),
                                                     rlpDecodeString (coder, items[4]),
                                                     rlpDecodeString (coder, items[5]),
                                                     rlpDecodeUInt64 (coder, items[6], 0),
                                                     cryptoClientCurrencyDenominationBundlesRlpDecode (items[7], coder));
}

extern void
cryptoClientAnnounceCurrencies (BRCryptoSystem system,
                                OwnershipGiven BRCryptoClientCurrencyBundle *bundles,
                                size_t bundlesCount) {
    BRArrayOf(BRCryptoClientCurrencyBundle) bundlesAsArray;
    array_new (bundlesAsArray, bundlesCount);
    array_add_array(bundlesAsArray, bundles, bundlesCount);

    cryptoSystemHandleCurrencyBundles (system, bundlesAsArray);
    array_free (bundlesAsArray);
}
