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

    qry->connected = true;

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
    pthread_mutex_unlock (&qry->lock);
}

extern void
cryptoClientQRYManagerDisconnect (BRCryptoClientQRYManager qry) {
    pthread_mutex_lock (&qry->lock);
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

extern void
cryptoClientQRYManagerTickTock (BRCryptoClientQRYManager qry) {
    pthread_mutex_lock (&qry->lock);

    // Skip out if not connected.
    if (!qry->connected) {
        pthread_mutex_unlock (&qry->lock);
        return;
    }

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

    // 3) we'll update transactions if there are more blocks to examine and if the
    //    prior sync completed successfully or not.
    if (qry->sync.completed && qry->sync.begBlockNumber != qry->sync.endBlockNumber) {

        // 3a) Save the current requestId and mark as not completed.
        qry->sync.rid = qry->requestId++;
        qry->sync.completed = false;
        qry->sync.success   = false;

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
    pthread_mutex_unlock (&qry->lock);
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
                                                  BRCryptoHash hash,
                                                  size_t rid) {
    BRCryptoClientCallbackState state = cryptoClientCallbackStateCreate (CLIENT_CALLBACK_SUBMIT_TRANSACTION, rid);

    state->u.submitTransaction.hash     = cryptoHashTake (hash);
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

    state->u.estimateTransactionFee.hash   = cryptoHashTake (hash);
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
            cryptoHashGive     (state->u.submitTransaction.hash);
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
                        uint64_t blockNumber,
                        const char *blockHashString) {

    BRCryptoBlockNumber oldBlockNumber = cryptoNetworkGetHeight (cwm->network);

    if (oldBlockNumber != blockNumber) {
        cryptoNetworkSetHeight (cwm->network, blockNumber);
        
        if (NULL != blockHashString) {
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

static int
cryptoClientTransactionBundleCompareForSort (const void *v1, const void *v2) {
    const BRCryptoClientTransactionBundle *b1 = v1;
    const BRCryptoClientTransactionBundle *b2 = v2;
    return cryptoClientTransactionBundleCompare (*b1, *b2);
}

extern void
cwmAnnounceTransactions (OwnershipKept BRCryptoWalletManager manager,
                         OwnershipGiven BRCryptoClientCallbackState callbackState,
                         BRCryptoBoolean success,
                         BRCryptoClientTransactionBundle *bundles,  // given elements, not array
                         size_t bundlesCount) {

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
                // Sort bundles to have the lowest blocknumber first.  Use of `mergesort` is
                // appropriate given that the bundles are likely already ordered.  This minimizes
                // dependency resolution between later transactions depending on prior transactions.
                //
                // Seems that there may be duplicates in `bundles`; will be dealt with later

                mergesort (bundles, bundlesCount, sizeof (BRCryptoClientTransactionBundle),
                           cryptoClientTransactionBundleCompareForSort);

                // Recover transfers from each bundle
                for (size_t index = 0; index < bundlesCount; index++) {
                    cryptoWalletManagerSaveTransactionBundle(manager, bundles[index]);
                    cryptoWalletManagerRecoverTransfersFromTransactionBundle (manager, bundles[index]);
                }

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

    pthread_mutex_lock (&qry->lock);
    qry->sync.completed = syncCompleted;
    qry->sync.success   = syncSuccess;
    pthread_mutex_unlock (&qry->lock);

    for (size_t index = 0; index < bundlesCount; index++)
        cryptoClientTransactionBundleRelease (bundles[index]);

    cryptoClientCallbackStateRelease(callbackState);
}

// MARK: - Announce Transfer

static int
cryptoClientTransferBundleCompareForSort (const void *v1, const void *v2) {
    const BRCryptoClientTransferBundle *b1 = v1;
    const BRCryptoClientTransferBundle *b2 = v2;
    return cryptoClientTransferBundleCompare (*b1, *b2);
}

extern void
cwmAnnounceTransfers (OwnershipKept BRCryptoWalletManager manager,
                      OwnershipGiven BRCryptoClientCallbackState callbackState,
                      BRCryptoBoolean success,
                      OwnershipGiven BRCryptoClientTransferBundle *bundles, // given elements, not array
                      size_t bundlesCount) {
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
                // Sort bundles to have the lowest blocknumber first.  Use of `mergesort` is
                // appropriate given that the bundles are likely already ordered.  This minimizes
                // dependency resolution between later transfers depending on prior transfers.

                mergesort (bundles, bundlesCount, sizeof (BRCryptoClientTransferBundle),
                           cryptoClientTransferBundleCompareForSort);

                // Recover transfers from each bundle
                for (size_t index = 0; index < bundlesCount; index++) {
                    cryptoWalletManagerSaveTransferBundle(manager, bundles[index]);
                    cryptoWalletManagerRecoverTransferFromTransferBundle (manager, bundles[index]);
                }

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

    pthread_mutex_lock (&qry->lock);
    qry->sync.completed = syncCompleted;
    qry->sync.success   = syncSuccess;
    pthread_mutex_unlock (&qry->lock);

    for (size_t index = 0; index < bundlesCount; index++)
        cryptoClientTransferBundleRelease (bundles[index]);

    cryptoClientCallbackStateRelease(callbackState);
}

// MARK: Announce Submit Transfer

static void
cryptoClientQRYSubmitTransfer (BRCryptoClientQRYManager qry,
                               BRCryptoWallet   wallet,
                               BRCryptoTransfer transfer) {
    BRCryptoWalletManager cwm = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == cwm) return;

    size_t   serializationCount;
    uint8_t *serialization = cryptoTransferSerializeForSubmission (transfer,
                                                                   cwm->network,
                                                                   &serializationCount);

    BRCryptoHash hash = cryptoTransferGetHash (transfer);
    char *hashAsString = cryptoHashEncodeString (hash);

    BRCryptoClientCallbackState callbackState =
    cryptoClientCallbackStateCreateSubmitTransaction (wallet, transfer, hash, qry->requestId++);

    qry->client.funcSubmitTransaction (qry->client.context,
                                       cwm,
                                       callbackState,
                                       serialization,
                                       serializationCount,
                                       hashAsString);

    cryptoHashGive (hash);
    free (serialization);
    free (hashAsString);
}

extern void
cwmAnnounceSubmitTransfer (OwnershipKept BRCryptoWalletManager cwm,
                           OwnershipGiven BRCryptoClientCallbackState callbackState,
                           BRCryptoBoolean success) {
    assert (CLIENT_CALLBACK_SUBMIT_TRANSACTION == callbackState->type);

    BRCryptoWallet   wallet   = callbackState->u.submitTransaction.wallet;
    BRCryptoTransfer transfer = callbackState->u.submitTransaction.transfer;

    // Must be the case... 'belt and suspenders'
    if (CRYPTO_TRUE == cryptoWalletHasTransfer (wallet, transfer)) {
        // Recover the `state` as either SUBMITTED or a UNKNOWN ERROR.  We have a slight issue, as
        // a possible race condition, whereby the transfer can already be INCLUDED by the time this
        // `announce` is called.  That has got to be impossible right?
        //
        // Assign the state; generate events in the process.
        cryptoTransferSetState (transfer, (CRYPTO_TRUE == success
                                           ? cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SUBMITTED)
                                           : cryptoTransferStateErroredInit (cryptoTransferSubmitErrorUnknown())));
    }

    cryptoClientCallbackStateRelease(callbackState);
}

// MARK: - Announce Estimate Transaction Fee

extern void
cryptoClientQRYEstimateTransferFee (BRCryptoClientQRYManager qry,
                                    BRCryptoCookie   cookie,
                                    OwnershipKept BRCryptoTransfer transfer,
                                    OwnershipKept BRCryptoNetworkFee networkFee,
                                    OwnershipKept BRCryptoFeeBasis initialFeeBasis) {
    BRCryptoWalletManager cwm = cryptoWalletManagerTakeWeak(qry->manager);
    if (NULL == cwm) return;

    size_t   serializationCount;
    uint8_t *serialization = cryptoTransferSerializeForFeeEstimation (transfer, cwm->network, &serializationCount);

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
                                   uint64_t costUnits,
                                   size_t attributesCount,
                                   OwnershipKept const char **attributeKeys,
                                   OwnershipKept const char **attributeVals) {
    assert (CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE == callbackState->type);

    BRCryptoStatus status = (CRYPTO_TRUE == success ? CRYPTO_SUCCESS : CRYPTO_ERROR_FAILED);
    BRCryptoCookie cookie = callbackState->u.estimateTransactionFee.cookie;

    BRCryptoNetworkFee networkFee = callbackState->u.estimateTransactionFee.networkFee;
    BRCryptoFeeBasis initialFeeBasis = callbackState->u.estimateTransactionFee.initialFeeBasis;

    BRCryptoAmount pricePerCostFactor = cryptoNetworkFeeGetPricePerCostFactor (networkFee);
    double costFactor = (double) costUnits;
    BRCryptoFeeBasis feeBasis = NULL;
    if (CRYPTO_TRUE == success)
        feeBasis = cryptoWalletManagerRecoverFeeBasisFromFeeEstimate (cwm,
                                                                      networkFee,
                                                                      initialFeeBasis,
                                                                      costFactor,
                                                                      attributesCount,
                                                                      attributeKeys,
                                                                      attributeVals);

    cryptoWalletGenerateEvent (cwm->wallet, (BRCryptoWalletEvent) {
        CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED,
        { .feeBasisEstimated = { status, cookie, feeBasis }}
    });

    cryptoAmountGive (pricePerCostFactor);
    cryptoClientCallbackStateRelease (callbackState);
}

// MARK: - Transfer Bundle

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
                      :  0))));
}

extern BRCryptoTransferState
cryptoClientTransferBundleGetTransferState (const BRCryptoClientTransferBundle bundle,
                                            BRCryptoFeeBasis confirmedFeeBasis) {
    bool isIncluded = (CRYPTO_TRANSFER_STATE_INCLUDED == bundle->status ||
                       (CRYPTO_TRANSFER_STATE_ERRORED == bundle->status &&
                        0 != bundle->blockNumber &&
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

    return rlpEncodeList (coder, 14,
                          rlpEncodeUInt64 (coder, bundle->status, 0),
                          rlpEncodeString (coder, bundle->hash),
                          rlpEncodeString (coder, bundle->uids),
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
                                                                         coder));
}

private_extern BRCryptoClientTransferBundle
cryptoClientTransferBundleRlpDecode (BRRlpItem item,
                                     BRRlpCoder coder) {
    size_t itemsCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);
    assert (14 == itemsCount);

    char *hash     = rlpDecodeString (coder, items[ 1]);
    char *uids     = rlpDecodeString (coder, items[ 2]);
    char *from     = rlpDecodeString (coder, items[ 3]);
    char *to       = rlpDecodeString (coder, items[ 4]);
    char *amount   = rlpDecodeString (coder, items[ 5]);
    char *currency = rlpDecodeString (coder, items[ 6]);
    char *fee      = rlpDecodeString (coder, items[ 7]);
    char *blkHash  = rlpDecodeString (coder, items[12]);

    BRCryptoTransferBundleRlpDecodeAttributesResult attributesResult =
    cryptoClientTransferBundleRlpDecodeAttributes (items[13], coder);

    BRCryptoClientTransferBundle bundle =
    cryptoClientTransferBundleCreate ((BRCryptoTransferStateType) rlpDecodeUInt64 (coder, items[ 0], 0),
                                      hash,
                                      uids,
                                      from,
                                      to,
                                      amount,
                                      currency,
                                      (0 == strcmp(fee,"") ? NULL : fee),
                                      rlpDecodeUInt64 (coder, items[ 8], 0),
                                      rlpDecodeUInt64 (coder, items[ 9], 0),
                                      rlpDecodeUInt64 (coder, items[10], 0),
                                      rlpDecodeUInt64 (coder, items[11], 0),
                                      blkHash,
                                      array_count(attributesResult.keys),
                                      (const char **) attributesResult.keys,
                                      (const char **) attributesResult.vals);

    free (blkHash);
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

private_extern bool
cryptoClientTransferBundleIsEqual (BRCryptoClientTransferBundle bundle1,
                                   BRCryptoClientTransferBundle bundle2) {
    return 0 == strcmp (bundle1->uids, bundle2->uids);
}


// MARK: - Transaction Bundle

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

extern int
cryptoClientTransactionBundleCompare (const BRCryptoClientTransactionBundle b1,
                                      const BRCryptoClientTransactionBundle b2) {
    return (b1->blockHeight < b2->blockHeight
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

private_extern bool
cryptoClientTransactionBundleIsEqual (BRCryptoClientTransactionBundle bundle1,
                                      BRCryptoClientTransactionBundle bundle2) {
    return (bundle1->status             == bundle2->status             &&
            bundle1->timestamp          == bundle2->timestamp          &&
            bundle1->blockHeight        == bundle2->blockHeight        &&
            bundle1->serializationCount == bundle2->serializationCount &&
            0 == memcmp (bundle1->serialization, bundle2->serialization, bundle1->serializationCount));
}

