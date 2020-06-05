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
#include "BRCryptoAddressP.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoWalletP.h"
#include "BRCryptoWalletManagerP.h"

#define OFFSET_BLOCKS_IN_SECONDS       (3 * 24 * 60 * 60)  // 3 days

// MARK: Client Sync/Send Forward Declarations

static void
cryptoClientP2PManagerSync (BRCryptoClientP2PManager p2p, BRCryptoSyncDepth depth, BRCryptoBlockNumber height);

static void
cryptoClientP2PManagerSend (BRCryptoClientP2PManager p2p, BRCryptoTransfer transfer);

static void
cryptoClientQRYManagerSync (BRCryptoClientQRYManager qry, BRCryptoSyncDepth depth, BRCryptoBlockNumber height);

static void
cryptoClientQRYManagerSend (BRCryptoClientQRYManager qry, BRCryptoTransfer transfer);


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
cryptoClientP2PManagerSend (BRCryptoClientP2PManager p2p, BRCryptoTransfer transfer) {
    p2p->handlers->send (p2p, transfer);
}

// MARK: Client QRY (QueRY)

static void cryptoClientQRYRequestBlockNumber  (BRCryptoClientQRYManager qry);
static void cryptoClientQRYRequestTransfers    (BRCryptoClientQRYManager qry,
                                                OwnershipGiven BRSetOf(BRCryptoAddress) addresses);
static void cryptoClientQRYRequestTransactions (BRCryptoClientQRYManager qry,
                                                OwnershipGiven BRSetOf(BRCryptoAddress) addresses);
static void cryptoClientQRYSubmitTransfer      (BRCryptoClientQRYManager qry, BRCryptoTransfer transfer);

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
    qry->sync.success = false;

    // gwm->syncContext  = syncContext;
    // gwm->syncCallback = syncCallback;
    return qry;
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
cryptoClientQRYManagerSend (BRCryptoClientQRYManager qry, BRCryptoTransfer transfer) {
    cryptoClientQRYSubmitTransfer (qry, transfer);
}

extern void
cryptoClientQRYManagerTickTock (BRCryptoClientQRYManager qry) {
    // Skip out if not an API sync.
    if (CRYPTO_SYNC_MODE_API_ONLY          != qry->manager->syncMode &&
        CRYPTO_SYNC_MODE_API_WITH_P2P_SEND != qry->manager->syncMode) return;

    cryptoClientQRYRequestBlockNumber (qry);

    if (qry->sync.completed && qry->sync.success) {
        // 1a) if so, advance the sync range by updating `begBlockNumber`
        qry->sync.begBlockNumber = (qry->sync.endBlockNumber >= qry->blockNumberOffset
                                    ? qry->sync.endBlockNumber - qry->blockNumberOffset
                                    : 0);
    }

    // 2) completed or not, update the `endBlockNumber` to the current block height.
    qry->sync.endBlockNumber = MAX (cryptoClientQRYGetNetworkBlockHeight (qry),
                                    qry->sync.begBlockNumber);

    // 3) we'll update transactions if there are more blocks to examine
    if (qry->sync.begBlockNumber != qry->sync.endBlockNumber) {

        // 3a) Save the current requestId and mark as not completed.
        qry->sync.rid = qry->requestId;
        qry->sync.completed = false;
        qry->sync.success   = false;

        BRCryptoWallet wallet = cryptoWalletManagerGetWallet (qry->manager);
        BRSetOf(BRCryptoAddress) addresses = cryptoWalletGetAddressesForRecovery (wallet);

        switch (qry->byType) {
            case CRYPTO_CLIENT_REQUEST_USE_TRANSFERS:
                cryptoClientQRYRequestTransfers (qry, addresses);
                break;
            case CRYPTO_CLIENT_REQUEST_USE_TRANSACTIONS:
                cryptoClientQRYRequestTransactions (qry, addresses);
                break;
        }

        cryptoWalletGive (wallet);
    }

//    if (NULL != gwm->syncCallback)
//        gwm->syncCallback (gwm->syncContext,
//                           gwm,
//                           qry->sync.begBlockNumber,
//                           qry->sync.endBlockNumber,
//                           // lots of incremental sync 'slop'
//                           2 * GWM_BRD_SYNC_START_BLOCK_OFFSET);

}

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
{
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
         }
#endif

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
cryptoClientCallbackStateCreateSubmitTransaction (BRCryptoHash hash,
                                                  size_t rid) {
    BRCryptoClientCallbackState state = cryptoClientCallbackStateCreate (CLIENT_CALLBACK_SUBMIT_TRANSACTION, rid);

    state->u.submitTransaction.hash = cryptoHashTake (hash);

    return state;
}

static BRCryptoClientCallbackState
cryptoClientCallbackStateCreateEstimateTransactionFee (BRCryptoHash hash,
                                                       BRCryptoCookie cookie,
                                                       OwnershipKept BRCryptoNetworkFee networkFee,
                                                       size_t rid) {
    BRCryptoClientCallbackState state = cryptoClientCallbackStateCreate (CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE, rid);

    state->u.estimateTransactionFee.hash   = cryptoHashTake (hash);
    state->u.estimateTransactionFee.cookie = cookie;
    state->u.estimateTransactionFee.networkFee = cryptoNetworkFeeTake(networkFee);

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
            cryptoHashGive (state->u.submitTransaction.hash);
            break;

        case CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE:
            cryptoHashGive (state->u.estimateTransactionFee.hash);
            cryptoNetworkFeeGive (state->u.estimateTransactionFee.networkFee);
            break;

        default:
            break;
    }

    memset (state, 0, sizeof (struct BRCryptoClientCallbackStateRecord));
    free (state);
}


// MARK: - Reqeust/Announce Block Number

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

extern void
cwmAnnounceBlockNumber (OwnershipKept BRCryptoWalletManager cwm,
                        OwnershipGiven BRCryptoClientCallbackState callbackState,
                        BRCryptoBoolean success,
                        uint64_t blockNumber) {

    BRCryptoBlockNumber oldBlockNumber = cryptoNetworkGetHeight (cwm->network);

    if (oldBlockNumber != blockNumber) {
        cryptoNetworkSetHeight (cwm->network, blockNumber);

        cryptoWalletManagerGenerateManagerEvent (cwm, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED,
            { .blockHeight = { blockNumber } }
        });
    }

    cryptoClientCallbackStateRelease (callbackState);
}

// MARK: - Request/Announce Transaction

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

static void
cryptoClientQRYRequestTransactions (BRCryptoClientQRYManager qry,
                                    OwnershipGiven BRSetOf(BRCryptoAddress) addresses) {
    BRCryptoWalletManager cwm = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == cwm) return;

    BRArrayOf(char *) addressesEncoded = cryptoClientQRYGetAddresses (qry, addresses);

    BRCryptoClientCallbackState callbackState = cryptoClientCallbackStateCreateGetTrans (CLIENT_CALLBACK_REQUEST_TRANSACTIONS,
                                                                                         addresses,
                                                                                         qry->requestId++);

    qry->client.funcGetTransactions (qry->client.context,
                                     cryptoWalletManagerTake(cwm),
                                     callbackState,
                                     (const char **) addressesEncoded,
                                     array_count(addressesEncoded),
                                     qry->sync.begBlockNumber,
                                     qry->sync.endBlockNumber);

    cryptoClientQRYReleaseAddresses (addressesEncoded);
}

extern void
cwmAnnounceTransactions (OwnershipKept BRCryptoWalletManager manager,
                         OwnershipGiven BRCryptoClientCallbackState callbackState,
                         BRCryptoBoolean success,
                         BRCryptoClientTransactionBundle *bundles,
                         size_t bundlesCount) {

    BRCryptoClientQRYManager qry = manager->qryManager;

    // Process the results if the bundles are for our rid; otherwise simply discard;
    if (callbackState->rid == qry->sync.rid) {
        qry->sync.completed = true;
        qry->sync.success = CRYPTO_TRUE == success;

        switch (success) {
            case CRYPTO_TRUE: {
                for (size_t index = 0; index < bundlesCount; index++)
                    cryptoWalletManagerRecoverTransfersFromTransactionBundle (manager, bundles[index]);

                // With transfers/transactions recovered, see if there is another set of
                // addresses to recover.  If so, simply make another request with the new set.
                BRCryptoWallet wallet = cryptoWalletManagerGetWallet(manager);
                BRSetOf(BRCryptoAddress) addresses = cryptoWalletGetAddressesForRecovery (wallet);

                BRSetMinus (addresses, callbackState->u.getTransactions.addresses);
                if (BRSetCount(addresses) > 0)
                    cryptoClientQRYRequestTransactions (qry, addresses);

                cryptoWalletGive (wallet);
                break;
            case CRYPTO_FALSE:
                break;
            }
        }
    }

    for (size_t index = 0; index < bundlesCount; index++)
        cryptoClientTransactionBundleRelease (bundles[index]);

    cryptoClientCallbackStateRelease(callbackState);
}

// MARK: - Announce Transfer

static void
cryptoClientQRYRequestTransfers (BRCryptoClientQRYManager qry,
                                 OwnershipGiven BRSetOf(BRCryptoAddress) addresses) {
    BRCryptoWalletManager cwm = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == cwm) return;

    BRArrayOf(char *) addressesEncoded = cryptoClientQRYGetAddresses (qry, addresses);

    BRCryptoClientCallbackState callbackState = cryptoClientCallbackStateCreateGetTrans (CLIENT_CALLBACK_REQUEST_TRANSACTIONS,
                                                                                         addresses,
                                                                                         qry->requestId++);

    qry->client.funcGetTransfers (qry->client.context,
                                     cryptoWalletManagerTake(cwm),
                                     callbackState,
                                     (const char **) addressesEncoded,
                                     array_count(addressesEncoded),
                                     qry->sync.begBlockNumber,
                                     qry->sync.endBlockNumber);

    cryptoClientQRYReleaseAddresses (addressesEncoded);
}

extern void
cwmAnnounceTransfers (OwnershipKept BRCryptoWalletManager manager,
                      OwnershipGiven BRCryptoClientCallbackState callbackState,
                      BRCryptoBoolean success,
                      OwnershipGiven BRCryptoClientTransferBundle *bundles,
                      size_t bundlesCount) {
    BRCryptoClientQRYManager qry = manager->qryManager;

    // Process the results if the bundles are for our rid; otherwise simply discard;
    if (callbackState->rid == qry->sync.rid) {
        qry->sync.completed = true;
        qry->sync.success = CRYPTO_TRUE == success;

        switch (success) {
            case CRYPTO_TRUE: {
                for (size_t index = 0; index < bundlesCount; index++)
                    cryptoWalletManagerRecoverTransferFromTransferBundle (manager, bundles[index]);
                break;
            case CRYPTO_FALSE:
                break;
            }
        }
    }

    for (size_t index = 0; index < bundlesCount; index++)
        cryptoClientTransferBundleRelease (bundles[index]);

    cryptoClientCallbackStateRelease(callbackState);
}

// MARK: Announce Submit Transfer

static void
cryptoClientQRYSubmitTransfer (BRCryptoClientQRYManager qry, BRCryptoTransfer transfer) {
    BRCryptoWalletManager cwm = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == cwm) return;

    size_t   serializationCount;
    uint8_t *serialization = cryptoTransferSerializeForSubmission (transfer,
                                                                   cwm->network,
                                                                   &serializationCount);


    BRCryptoHash hash = cryptoTransferGetHash (transfer);
    char *hashAsHex = cryptoHashString(hash);

    BRCryptoClientCallbackState callbackState = cryptoClientCallbackStateCreateSubmitTransaction (hash, qry->requestId++);

    qry->client.funcSubmitTransaction (qry->client.context,
                                       cryptoWalletManagerTake(cwm),
                                       callbackState,
                                       serialization,
                                       serializationCount,
                                       hashAsHex);

    cryptoHashGive (hash);
    free (serialization);
    free (hashAsHex);
}

extern void
cwmAnnounceSubmitTransfer (OwnershipKept BRCryptoWalletManager cwm,
                           OwnershipGiven BRCryptoClientCallbackState callbackState,
                           BRCryptoBoolean success,
                           OwnershipKept const char *hash) {
    assert (CLIENT_CALLBACK_SUBMIT_TRANSACTION == callbackState->type);

    cryptoClientCallbackStateRelease(callbackState);
}


// MARK: - Announce Estimate Transaction Fee

extern void
cryptoClientQRYEstimateTransferFee (BRCryptoClientQRYManager qry,
                                    BRCryptoCookie   cookie,
                                    OwnershipKept BRCryptoTransfer transfer,
                                    OwnershipKept BRCryptoNetworkFee networkFee) {
    BRCryptoWalletManager cwm = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == cwm) return;

    size_t   serializationCount;
    uint8_t *serialization = cryptoTransferSerializeForFeeEstimation (transfer, cwm->network, &serializationCount);

    // There is no hash... transfer is not guaranteed to be signed; likely unsigned.
    BRCryptoHash hash      = NULL;
    const char  *hashAsHex = "";

    BRCryptoClientCallbackState callbackState = cryptoClientCallbackStateCreateEstimateTransactionFee (hash,
                                                                                                       cookie,
                                                                                                       networkFee,
                                                                                                       qry->requestId++);

    qry->client.funcEstimateTransactionFee (qry->client.context,
                                            cryptoWalletManagerTake(cwm),
                                            callbackState,
                                            serialization,
                                            serializationCount,
                                            hashAsHex);
}

extern void
cwmAnnounceEstimateTransactionFee (OwnershipKept BRCryptoWalletManager cwm,
                                   OwnershipGiven BRCryptoClientCallbackState callbackState,
                                   BRCryptoBoolean success,
                                   OwnershipKept const char *hash,
                                   uint64_t costUnits) {
    assert (CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE == callbackState->type);

    BRCryptoStatus status = (CRYPTO_TRUE == success ? CRYPTO_SUCCESS : CRYPTO_ERROR_FAILED);
    BRCryptoCookie cookie = callbackState->u.estimateTransactionFee.cookie;

    BRCryptoNetworkFee networkFee = callbackState->u.estimateTransactionFee.networkFee;

    BRCryptoAmount pricePerCostFactor = cryptoNetworkFeeGetPricePerCostFactor (networkFee);
    double costFactor = (double) costUnits;
    BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreate (pricePerCostFactor, costFactor);

    cryptoWalletManagerGenerateWalletEvent (cwm, cwm->wallet, (BRCryptoWalletEvent) {
        CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED,
        { .feeBasisEstimated = { status, cookie, feeBasis }}
    });

    cryptoAmountGive (pricePerCostFactor);
    cryptoFeeBasisGive (feeBasis);
    cryptoClientCallbackStateRelease (callbackState);
}

#if 0
    const char *addresses[] = {
        "35qSFN1ktQBsrbK4bFUJfvFtzgrHDTSY4M"
    };
    size_t addressesCount = sizeof (addresses) / sizeof (char *);

    uint64_t begBlockNumber = 629535;
    uint64_t endBlockNumber = 629543; // BLOCK_HEIGHT_UNBOUND;

    switch (qry->byType) {
        case CRYPTO_CLIENT_REQUEST_USE_TRANSFERS:
            qry->client.funcGetTransfers (qry->client.context,
                                          cryptoWalletManagerTake (qry->manager),
                                          qryState,
                                          addresses,
                                          addressesCount,
                                          begBlockNumber,
                                          endBlockNumber);
            break;

        case CRYPTO_CLIENT_REQUEST_USE_TRANSACTIONS:
            qry->client.funcGetTransactions (qry->client.context,
                                             cryptoWalletManagerTake (qry->manager),
                                             qryState,
                                             addresses,
                                             addressesCount,
                                             begBlockNumber,
                                             endBlockNumber);

            break;
    }
#endif

// MARK: - Transfer/Transaction Bundles

extern BRCryptoClientTransferBundle
cryptoClientTransferBundleCreate (BRCryptoTransferStateType status,
                                  OwnershipKept const char *hash,
                                  OwnershipKept const char *uids,
                                  OwnershipKept const char *from,
                                  OwnershipKept const char *to,
                                  OwnershipKept const char *amount,
                                  OwnershipKept const char *currency,
                                  OwnershipKept const char *fee,
                                  uint64_t blockTimestamp,
                                  uint64_t blockNumber,
                                  uint64_t blockConfirmations,
                                  uint64_t blockTransactionIndex,
                                  OwnershipKept const char *blockHash,
                                  size_t attributesCount,
                                  OwnershipKept const char **attributeKeys,
                                  OwnershipKept const char **attributeVals) {
    BRCryptoClientTransferBundle bundle = calloc (1, sizeof (struct BRCryptoClientTransferBundleRecord));

    bundle->status   = status;
    bundle->hash     = strdup (hash);
    bundle->uids     = strdup (uids);
    bundle->from     = strdup (from);
    bundle->to       = strdup (to);
    bundle->amount   = strdup (amount);
    bundle->currency = strdup (currency);
    bundle->fee      = NULL == fee ? NULL : strdup (fee);

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
    free (bundle->hash);
    free (bundle->uids);
    free (bundle->from);
    free (bundle->to);
    free (bundle->amount);
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

extern BRCryptoClientTransactionBundle
cryptoClientTransactionBundleCreate (BRCryptoTransferStateType status,
                                     OwnershipKept uint8_t *transaction,
                                     size_t transactionLength,
                                     uint64_t timestamp,
                                     uint64_t blockHeight) {
    BRCryptoClientTransactionBundle bundle = calloc (1, sizeof (struct BRCryptoClientTransactionBundleRecord));

    bundle->status = status;

    bundle->serialization = malloc(transactionLength);
    memcpy (bundle->serialization, transaction, transactionLength);
    bundle->serializationCount = transactionLength;

    bundle->timestamp   = AS_CRYPTO_TIMESTAMP(timestamp);
    bundle->blockHeight = (BRCryptoBlockNumber) blockHeight;

    return bundle;
}

extern void
cryptoClientTransactionBundleRelease (BRCryptoClientTransactionBundle bundle) {
    free (bundle->serialization);
    memset (bundle, 0, sizeof (struct BRCryptoClientTransactionBundleRecord));
    free (bundle);
}

