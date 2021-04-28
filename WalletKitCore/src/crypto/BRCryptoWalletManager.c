//
//  BRCryptoWalletManager.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <arpa/inet.h>      // struct in_addr
#include <ctype.h>          // toupper()

#include "BRCryptoBase.h"

#include "BRCryptoKeyP.h"
#include "BRCryptoAccountP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoAddressP.h"
#include "BRCryptoAmountP.h"
#include "BRCryptoFeeBasisP.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoWalletP.h"
#include "BRCryptoPaymentP.h"
#include "BRCryptoClientP.h"
#include "BRCryptoFileService.h"

#include "BRCryptoWalletManager.h"
#include "BRCryptoWalletManagerP.h"

#include "BRCryptoHandlersP.h"

#include "bitcoin/BRMerkleBlock.h"
#include "bitcoin/BRPeer.h"
#include "support/event/BREventAlarm.h"

// We'll do a period QRY 'tick-tock' CWM_CONFIRMATION_PERIOD_FACTOR times in
// each network's ConfirmationPeriod.  Thus, for example, the Bitcoin confirmation period is
// targeted for every 10 minutes; we'll check every 2.5 minutes.
#define CWM_CONFIRMATION_PERIOD_FACTOR  (4)

// We don't want to sample too frequently or too infrequently.  As a base, we'll sample at:
//     block-chain-period / CWM_CONFIRMATION_PERIOD_FACTOR
// which is 2.5 minutes for BTC and 1/4 second for Ripple.
//
// From a User perspective waiting 10 seconds to learn of a transaction, even if it could be
// confirmed in 1 second, isn't burdensome.  This would ba a case like Ripple where blocks arrive
// every 1 second and only 1 block-ing is needed for confirmation.  Other blockchains have
// confirmationCounts beyond 1, like 6, and thus waiting 10 seconds isn't bad when 6 blocks are
// needed anyway.
//
// Additionally sampling at 1 minutes for a blockchain like BTC, where blocks average 10 minutes,
// is not burdensome on the network data rates.
//
#define CWM_MAXIMUM_SAMPLING_PERIOD_IN_MILLISECONDS   (1 * 60 * 1000)    //  1 minute
#define CWM_MINIMUM_SAMPLING_PERIOD_IN_MILLISECONDS   (    10 * 1000)    // 10 seconds

static unsigned int
cryptoWalletManagerBoundSamplingPeriod (unsigned int milliseconds) {
    return (milliseconds > CWM_MAXIMUM_SAMPLING_PERIOD_IN_MILLISECONDS
            ? CWM_MAXIMUM_SAMPLING_PERIOD_IN_MILLISECONDS
            : (milliseconds < CWM_MINIMUM_SAMPLING_PERIOD_IN_MILLISECONDS
               ? CWM_MINIMUM_SAMPLING_PERIOD_IN_MILLISECONDS
               : milliseconds));
}

uint64_t BLOCK_HEIGHT_UNBOUND_VALUE = UINT64_MAX;

static void
cryptoWalletManagerPeriodicDispatcher (BREventHandler handler,
                                       BREventTimeout *event);

static void
cryptoWalletManagerFileServiceErrorHandler (BRFileServiceContext context,
                                            BRFileService fs,
                                            BRFileServiceError error);

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoWalletManager, cryptoWalletManager)

/// =============================================================================================
///
/// MARK: - Wallet Manager State
///
///
private_extern BRCryptoWalletManagerState
cryptoWalletManagerStateInit(BRCryptoWalletManagerStateType type) {
    switch (type) {
        case CRYPTO_WALLET_MANAGER_STATE_CREATED:
        case CRYPTO_WALLET_MANAGER_STATE_CONNECTED:
        case CRYPTO_WALLET_MANAGER_STATE_SYNCING:
        case CRYPTO_WALLET_MANAGER_STATE_DELETED:
            return (BRCryptoWalletManagerState) { type };
        case CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED:
            assert (0); // if you are hitting this, use cryptoWalletManagerStateDisconnectedInit!
            return (BRCryptoWalletManagerState) {
                CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED,
                { .disconnected = { cryptoWalletManagerDisconnectReasonUnknown() } }
            };
    }
}

private_extern BRCryptoWalletManagerState
cryptoWalletManagerStateDisconnectedInit(BRCryptoWalletManagerDisconnectReason reason) {
    return (BRCryptoWalletManagerState) {
        CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED,
        { .disconnected = { reason } }
    };
}

/// =============================================================================================
///
/// MARK: - Wallet Manager
///
///

#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-function"
static BRArrayOf(BRCryptoCurrency)
cryptoWalletManagerGetCurrenciesOfInterest (BRCryptoWalletManager cwm) {
    BRArrayOf(BRCryptoCurrency) currencies;

    array_new (currencies, 3);
    return currencies;
}

static void
cryptoWalletManagerReleaseCurrenciesOfInterest (BRCryptoWalletManager cwm,
                                                BRArrayOf(BRCryptoCurrency) currencies) {
    for (size_t index = 0; index < array_count(currencies); index++)
        cryptoCurrencyGive (currencies[index]);
    array_free (currencies);
}
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

static int
cryptoClientTransferBundleCompareByBlockheight (const void *tb1, const void *tb2) {
    BRCryptoClientTransferBundle b1 = * (BRCryptoClientTransferBundle *) tb1;
    BRCryptoClientTransferBundle b2 = * (BRCryptoClientTransferBundle *) tb2;
    return (b1->blockNumber < b2-> blockNumber
            ? -1
            : (b1->blockNumber > b2->blockNumber
               ? +1
               :  0));
}

static void // not locked; called during manager init
cryptoWalletManagerInitialTransferBundlesLoad (BRCryptoWalletManager manager) {
    assert (NULL == manager->bundleTransfers);

    BRSetOf(BRCryptoClientTransferBundle) bundles = cryptoClientTransferBundleSetCreate (25);

    if (fileServiceHasType (manager->fileService, CRYPTO_FILE_SERVICE_TYPE_TRANSFER) &&
        1 != fileServiceLoad (manager->fileService, bundles, CRYPTO_FILE_SERVICE_TYPE_TRANSFER, 1)) {
        cryptoClientTransferBundleSetRelease (bundles);
        printf ("CRY: %4s: failed to load transfer bundles",
                cryptoBlockChainTypeGetCurrencyCode (manager->type));
        cryptoClientTransferBundleSetRelease(bundles);
        return;
    }
    size_t sortedBundlesCount = BRSetCount(bundles);

    printf ("CRY: %4s: loaded %4zu transfer bundles\n",
            cryptoBlockChainTypeGetCurrencyCode (manager->type),
            sortedBundlesCount);

    if (0 != sortedBundlesCount) {
        array_new (manager->bundleTransfers, sortedBundlesCount);
        BRSetAll (bundles, (void**) manager->bundleTransfers, sortedBundlesCount);
        array_set_count (manager->bundleTransfers, sortedBundlesCount);

        qsort (manager->bundleTransfers, sortedBundlesCount, sizeof (BRCryptoClientTransferBundle), cryptoClientTransferBundleCompareByBlockheight);
    }

    // Don't release the set's bundles
    BRSetFree(bundles);
}

static void // called wtih manager->lock
cryptoWalletManagerInitialTransferBundlesRecover (BRCryptoWalletManager manager) {
    if (NULL != manager->bundleTransfers) {
        for (size_t index = 0; index < array_count(manager->bundleTransfers); index++) {
            cryptoWalletManagerRecoverTransferFromTransferBundle (manager, manager->bundleTransfers[index]);
        }

        array_free_all (manager->bundleTransfers, cryptoClientTransferBundleRelease);
        manager->bundleTransfers = NULL;
    }
}

static int
cryptoClientTransactionBundleCompareByBlockheight (const void *tb1, const void *tb2) {
    BRCryptoClientTransactionBundle b1 = * (BRCryptoClientTransactionBundle *) tb1;
    BRCryptoClientTransactionBundle b2 = * (BRCryptoClientTransactionBundle *) tb2;
    return (b1->blockHeight < b2-> blockHeight
            ? -1
            : (b1->blockHeight > b2->blockHeight
               ? +1
               :  0));
}

static void // not locked; called during manager init
cryptoWalletManagerInitialTransactionBundlesLoad (BRCryptoWalletManager manager) {
    assert (NULL == manager->bundleTransactions);

    BRSetOf(BRCryptoClientTransactionBundle) bundles = cryptoClientTransactionBundleSetCreate (25);
    if (fileServiceHasType (manager->fileService, CRYPTO_FILE_SERVICE_TYPE_TRANSACTION) &&
        1 != fileServiceLoad (manager->fileService, bundles, CRYPTO_FILE_SERVICE_TYPE_TRANSACTION, 1)) {
        cryptoClientTransactionBundleSetRelease (bundles);
        printf ("CRY: %4s: failed to load transaction bundles",
                cryptoBlockChainTypeGetCurrencyCode (manager->type));
        return;
    }
    size_t sortedBundlesCount = BRSetCount(bundles);

    printf ("CRY: %4s: loaded %4zu transaction bundles\n",
            cryptoBlockChainTypeGetCurrencyCode (manager->type),
            sortedBundlesCount);

    if (0 != sortedBundlesCount) {
        BRArrayOf(BRCryptoClientTransactionBundle) sortedBundles;
        array_new (sortedBundles, sortedBundlesCount);
        BRSetAll (bundles, (void**) sortedBundles, sortedBundlesCount);
        array_set_count(sortedBundles, sortedBundlesCount);

        qsort (sortedBundles, sortedBundlesCount, sizeof (BRCryptoClientTransactionBundle), cryptoClientTransactionBundleCompareByBlockheight);

        manager->bundleTransactions = sortedBundles;
    }

    // Don't release the set's bundles
    BRSetFree(bundles);
}

static void // called wtih manager->lock
cryptoWalletManagerInitialTransactionBundlesRecover (BRCryptoWalletManager manager) {
    if (NULL != manager->bundleTransactions) {
        for (size_t index = 0; index < array_count(manager->bundleTransactions); index++) {
            cryptoWalletManagerRecoverTransfersFromTransactionBundle (manager, manager->bundleTransactions[index]);
        }

        array_free_all (manager->bundleTransactions, cryptoClientTransactionBundleRelease);
        manager->bundleTransactions = NULL;
    }
}

extern BRCryptoWalletManager
cryptoWalletManagerAllocAndInit (size_t sizeInBytes,
                                 BRCryptoBlockChainType type,
                                 BRCryptoWalletManagerListener listener,
                                 BRCryptoClient client,
                                 BRCryptoAccount account,
                                 BRCryptoNetwork network,
                                 BRCryptoAddressScheme scheme,
                                 const char *path,
                                 BRCryptoClientQRYByType byType,
                                 BRCryptoWalletManagerCreateContext createContext,
                                 BRCryptoWalletManagerCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct BRCryptoWalletManagerRecord));
    assert (type == cryptoNetworkGetType(network));

    BRCryptoWalletManager manager = calloc (1, sizeInBytes);
    if (NULL == manager) return NULL;

    manager->type = type;
    manager->handlers = cryptoHandlersLookup(type)->manager;
    network->sizeInBytes = sizeInBytes;
    manager->listener = listener;

    manager->client  = client;
    manager->network = cryptoNetworkTake (network);
    manager->account = cryptoAccountTake (account);
    manager->state   = cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_CREATED);
    manager->addressScheme = scheme;
    manager->path = strdup (path);

    manager->byType = byType;

    // Hold this early
    manager->ref = CRYPTO_REF_ASSIGN (cryptoWalletManagerRelease);

    // Initialize to NULL, for now.
    manager->qryManager = NULL;
    manager->p2pManager = NULL;
    manager->wallet     = NULL;
    array_new (manager->wallets, 1);

    // File Service
    const char *currencyName = cryptoBlockChainTypeGetCurrencyCode (manager->type);
    const char *networkName  = cryptoNetworkGetDesc(network);

    // TODO: Replace `createFileService` with `getFileServiceSpecifications`
    manager->fileService = manager->handlers->createFileService (manager,
                                                                 manager->path,
                                                                 currencyName,
                                                                 networkName,
                                                                 manager,
                                                                 cryptoWalletManagerFileServiceErrorHandler);

    // TODO: This causes an Android (only - Core Demo App) crash.  Understand, then restore
    // fileServicePurge (manager->fileService);

    // Create the alarm clock, but don't start it.
    alarmClockCreateIfNecessary(0);

    // Create the event handler name (useful for debugging).
    char handlerName[5 + strlen(currencyName) + 1];
    sprintf(handlerName, "Core %s", currencyName);
    for (char *s = &handlerName[5]; *s; s++) *s = toupper (*s);

    // Get the event handler types.
    size_t eventTypesCount;
    const BREventType **eventTypes = manager->handlers->getEventTypes (manager, &eventTypesCount);

    // Create the event handler
    manager->handler = eventHandlerCreate (handlerName,
                                           eventTypes,
                                           eventTypesCount,
                                           &manager->lock);

    eventHandlerSetTimeoutDispatcher (manager->handler,
                                      cryptoWalletManagerBoundSamplingPeriod ((1000 * cryptoNetworkGetConfirmationPeriodInSeconds(network)) / CWM_CONFIRMATION_PERIOD_FACTOR),
                                      (BREventDispatcher) cryptoWalletManagerPeriodicDispatcher,
                                      (void*) manager);

    manager->listenerWallet = cryptoListenerCreateWalletListener (&manager->listener, manager);

    pthread_mutex_init_brd (&manager->lock, PTHREAD_MUTEX_RECURSIVE);

    BRCryptoTimestamp   earliestAccountTime = cryptoAccountGetTimestamp (account);
    BRCryptoBlockNumber earliestBlockNumber = cryptoNetworkGetBlockNumberAtOrBeforeTimestamp(network, earliestAccountTime);
    BRCryptoBlockNumber latestBlockNumber   = cryptoNetworkGetHeight (network);

    // Setup the QRY Manager
    manager->qryManager = cryptoClientQRYManagerCreate (client,
                                                        manager,
                                                        manager->byType,
                                                        earliestBlockNumber,
                                                        latestBlockNumber);

    if (createCallback) createCallback (createContext, manager);
    
    // Announce the created manager; this must preceed any wallet created/added events
    cryptoWalletManagerGenerateEvent (manager, (BRCryptoWalletManagerEvent) {
        CRYPTO_WALLET_MANAGER_EVENT_CREATED
    });

    cryptoWalletManagerInitialTransferBundlesLoad (manager);
    cryptoWalletManagerInitialTransactionBundlesLoad (manager);

    // Create the primary wallet
    manager->wallet = cryptoWalletManagerCreateWalletInitialized (manager,
                                                                  network->currency,
                                                                  manager->bundleTransactions,
                                                                  manager->bundleTransfers);

    // Create the P2P manager
    manager->p2pManager = manager->handlers->createP2PManager (manager);

    // Allow callers to be in an atomic region.
    pthread_mutex_lock (&manager->lock);
    return manager;
}

extern BRCryptoWalletManager
cryptoWalletManagerCreate (BRCryptoWalletManagerListener listener,
                           BRCryptoClient client,
                           BRCryptoAccount account,
                           BRCryptoNetwork network,
                           BRCryptoSyncMode mode,
                           BRCryptoAddressScheme scheme,
                           const char *path) {
    // Only create a wallet manager for accounts that are initialized on network.
    if (CRYPTO_FALSE == cryptoNetworkIsAccountInitialized (network, account))
        return NULL;

    // Lookup the handler for the network's type.
    BRCryptoBlockChainType type = cryptoNetworkGetType(network);
    const BRCryptoWalletManagerHandlers *handlers = cryptoHandlersLookup(type)->manager;

    // Create the manager
    BRCryptoWalletManager manager = handlers->create (listener,
                                                      client,
                                                      account,
                                                      network,
                                                      mode,
                                                      scheme,
                                                      path);
    if (NULL == manager) return NULL;

    // Recover transfers and transactions
    cryptoWalletManagerInitialTransferBundlesRecover (manager);
    cryptoWalletManagerInitialTransactionBundlesRecover (manager);

    pthread_mutex_unlock (&manager->lock);

    // Set the mode for QRY or P2P syncing
    cryptoWalletManagerSetMode (manager, mode);

    return manager;
}

#define _peer_log_x printf
static void
cryptoWalletManagerFileServiceErrorHandler (BRFileServiceContext context,
                                            BRFileService fs,
                                            BRFileServiceError error) {
    switch (error.type) {
        case FILE_SERVICE_IMPL:
            // This actually a FATAL - an unresolvable coding error.
            _peer_log_x ("CRY: FileService Error: IMPL: %s\n", error.u.impl.reason);
            break;
        case FILE_SERVICE_UNIX:
            _peer_log_x ("CRY: FileService Error: UNIX: %s\n", strerror(error.u.unx.error));
            break;
        case FILE_SERVICE_ENTITY:
            // This is likely a coding error too.
            _peer_log_x ("CRY: FileService Error: ENTITY (%s): %s\n",
                     error.u.entity.type,
                     error.u.entity.reason);
            break;
        case FILE_SERVICE_SDB:
            _peer_log_x ("CRY: FileService Error: SDB: (%d): %s\n",
                       error.u.sdb.code,
                       error.u.sdb.reason);
            break;
    }
    _peer_log_x ("CRY: FileService Error: FORCED SYNC%s\n", "");

    // BRWalletManager bwm = (BRWalletManager) context;
    // TODO(fix): What do we actually want to happen here?
    // if (NULL != bwm->peerManager)
    //     BRPeerManagerRescan (bwm->peerManager);
}
#undef _peer_log_x


static void
cryptoWalletManagerRelease (BRCryptoWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);

    // Ensure CWM is stopped...
    cryptoWalletManagerStop (cwm);

    // ... then release any type-specific resources
    cwm->handlers->release (cwm);

    // ... then release memory.
    cryptoAccountGive (cwm->account);
    cryptoNetworkGive (cwm->network);
    if (NULL != cwm->wallet) cryptoWalletGive (cwm->wallet);

    // .. then give all the wallets
    for (size_t index = 0; index < array_count(cwm->wallets); index++)
        cryptoWalletGive (cwm->wallets[index]);
    array_free (cwm->wallets);

    // ... then the p2p and qry managers
    if (NULL != cwm->p2pManager) cryptoClientP2PManagerRelease(cwm->p2pManager);
    cryptoClientQRYManagerRelease (cwm->qryManager);

    // ... then the fileService
    if (NULL != cwm->fileService) fileServiceRelease (cwm->fileService);

    // ... then the eventHandler
    eventHandlerDestroy (cwm->handler);
//    eventHandlerDestroy (cwm->listenerHandler);

    // ... and finally individual memory allocations
    free (cwm->path);

    pthread_mutex_unlock  (&cwm->lock);
    pthread_mutex_destroy (&cwm->lock);

    memset (cwm, 0, sizeof(*cwm));
    free (cwm);
}

extern BRCryptoNetwork
cryptoWalletManagerGetNetwork (BRCryptoWalletManager cwm) {
    return cryptoNetworkTake (cwm->network);
}

extern BRCryptoBoolean
cryptoWalletManagerHasNetwork (BRCryptoWalletManager cwm,
                               BRCryptoNetwork network) {
    return AS_CRYPTO_BOOLEAN (cwm->network == network);
}

extern BRCryptoAccount
cryptoWalletManagerGetAccount (BRCryptoWalletManager cwm) {
    return cryptoAccountTake (cwm->account);
}

extern BRCryptoBoolean
cryptoWalletManagerHasAccount (BRCryptoWalletManager cwm,
                               BRCryptoAccount account) {
    return AS_CRYPTO_BOOLEAN (cwm->account == account);
}

extern void
cryptoWalletManagerSetMode (BRCryptoWalletManager cwm, BRCryptoSyncMode mode) {
    pthread_mutex_lock (&cwm->lock);

    // Get default p2p{Sync,Send} Managers
    BRCryptoClientSync p2pSync = (NULL != cwm->p2pManager
                                  ? cryptoClientP2PManagerAsSync (cwm->p2pManager)
                                  : cryptoClientQRYManagerAsSync (cwm->qryManager));
    BRCryptoClientSend p2pSend = (NULL != cwm->p2pManager
                                  ? cryptoClientP2PManagerAsSend (cwm->p2pManager)
                                  : cryptoClientQRYManagerAsSend (cwm->qryManager));

    BRCryptoClientSync qrySync = cryptoClientQRYManagerAsSync (cwm->qryManager);
    BRCryptoClientSend qrySend = cryptoClientQRYManagerAsSend (cwm->qryManager);

    // TODO: Manager Memory?

    // Set cwm->can{Sync,Send} based on mode.
    switch (mode) {
        case CRYPTO_SYNC_MODE_API_ONLY:
            cwm->canSync = qrySync;
            cwm->canSend = qrySend;
            break;
        case CRYPTO_SYNC_MODE_API_WITH_P2P_SEND:
            cwm->canSync = qrySync;
            cwm->canSend = p2pSend;
            break;
        case CRYPTO_SYNC_MODE_P2P_WITH_API_SYNC:
             // Initial sync w/ QRY, thereafter w/ P2P
            cwm->canSync = qrySync;
            cwm->canSend = p2pSend;
            break;
        case CRYPTO_SYNC_MODE_P2P_ONLY:
            cwm->canSync = p2pSync;
            cwm->canSend = p2pSend;
            break;
    }
    cwm->syncMode = mode;
    pthread_mutex_unlock (&cwm->lock);
}

extern BRCryptoSyncMode
cryptoWalletManagerGetMode (BRCryptoWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    BRCryptoSyncMode mode = cwm->syncMode;
    pthread_mutex_unlock (&cwm->lock);
    return mode;
}

extern BRCryptoWalletManagerState
cryptoWalletManagerGetState (BRCryptoWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    BRCryptoWalletManagerState state = cwm->state;
    pthread_mutex_unlock (&cwm->lock);
    return state;
}

extern BRCryptoWalletManagerStateType
cryptoWalletManagerGetStateType (BRCryptoWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    BRCryptoWalletManagerStateType type = cwm->state.type;
    pthread_mutex_unlock (&cwm->lock);

    return type;
}

private_extern void
cryptoWalletManagerSetState (BRCryptoWalletManager cwm,
                             BRCryptoWalletManagerState newState) {
    pthread_mutex_lock (&cwm->lock);
    BRCryptoWalletManagerState oldState = cwm->state;
    cwm->state = newState;
    pthread_mutex_unlock (&cwm->lock);

    if (oldState.type != newState.type)
        cryptoWalletManagerGenerateEvent (cwm, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_CHANGED,
            { .state = { oldState, newState }}
        });
}

extern BRCryptoAddressScheme
cryptoWalletManagerGetAddressScheme (BRCryptoWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    BRCryptoAddressScheme scheme = cwm->addressScheme;
    pthread_mutex_unlock (&cwm->lock);
    return scheme;
}

extern void
cryptoWalletManagerSetAddressScheme (BRCryptoWalletManager cwm,
                                     BRCryptoAddressScheme scheme) {
    pthread_mutex_lock (&cwm->lock);
    cwm->addressScheme = scheme;
    pthread_mutex_unlock (&cwm->lock);
}

extern const char *
cryptoWalletManagerGetPath (BRCryptoWalletManager cwm) {
    return cwm->path;
}

extern void
cryptoWalletManagerSetNetworkReachable (BRCryptoWalletManager cwm,
                                        BRCryptoBoolean isNetworkReachable) {
    if (NULL != cwm->p2pManager) {
        cryptoClientP2PManagerSetNetworkReachable (cwm->p2pManager, isNetworkReachable);
    }
}

extern BRCryptoWallet
cryptoWalletManagerCreateWalletInitialized (BRCryptoWalletManager cwm,
                                            BRCryptoCurrency currency,
                                            Nullable OwnershipKept BRArrayOf(BRCryptoClientTransactionBundle) transactions,
                                            Nullable OwnershipKept BRArrayOf(BRCryptoClientTransferBundle) transfers) {
    BRCryptoWallet wallet = cryptoWalletManagerGetWalletForCurrency (cwm, currency);
    return (NULL == wallet
            ? cwm->handlers->createWallet (cwm, currency, transactions, transfers)
            : wallet);
}


extern BRCryptoWallet
cryptoWalletManagerCreateWallet (BRCryptoWalletManager cwm,
                                 BRCryptoCurrency currency) {
    return cryptoWalletManagerCreateWalletInitialized (cwm, currency, NULL, NULL);
}


extern BRCryptoWallet
cryptoWalletManagerGetWallet (BRCryptoWalletManager cwm) {
    return cryptoWalletTake (cwm->wallet);
}

extern BRCryptoWallet *
cryptoWalletManagerGetWallets (BRCryptoWalletManager cwm, size_t *count) {
    pthread_mutex_lock (&cwm->lock);
    *count = array_count (cwm->wallets);
    BRCryptoWallet *wallets = NULL;
    if (0 != *count) {
        wallets = calloc (*count, sizeof(BRCryptoWallet));
        for (size_t index = 0; index < *count; index++) {
            wallets[index] = cryptoWalletTake(cwm->wallets[index]);
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallets;
}


extern BRCryptoWallet
cryptoWalletManagerGetWalletForCurrency (BRCryptoWalletManager cwm,
                                         BRCryptoCurrency currency) {
    BRCryptoWallet wallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count(cwm->wallets); index++) {
        if (CRYPTO_TRUE == cryptoWalletHasCurrency (cwm->wallets[index], currency)) {
            wallet = cryptoWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}

static BRCryptoBoolean
cryptoWalletManagerHasWalletLock (BRCryptoWalletManager cwm,
                                  BRCryptoWallet wallet,
                                  bool needLock) {
    BRCryptoBoolean r = CRYPTO_FALSE;
    if (needLock) pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets) && CRYPTO_FALSE == r; index++) {
        r = cryptoWalletEqual(cwm->wallets[index], wallet);
    }
    if (needLock) pthread_mutex_unlock (&cwm->lock);
    return r;
}

extern BRCryptoBoolean
cryptoWalletManagerHasWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet) {
    return cryptoWalletManagerHasWalletLock (cwm, wallet, true);
}

extern void
cryptoWalletManagerAddWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet) {
    pthread_mutex_lock (&cwm->lock);
    if (CRYPTO_FALSE == cryptoWalletManagerHasWalletLock (cwm, wallet, false)) {
        array_add (cwm->wallets, cryptoWalletTake (wallet));
        cryptoWalletManagerGenerateEvent (cwm, (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED,
            { .wallet = cryptoWalletTake (wallet) }
        });
    }
    pthread_mutex_unlock (&cwm->lock);
}

extern void
cryptoWalletManagerRemWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet) {

    BRCryptoWallet managerWallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets); index++) {
        if (CRYPTO_TRUE == cryptoWalletEqual(cwm->wallets[index], wallet)) {
            managerWallet = cwm->wallets[index];
            array_rm (cwm->wallets, index);
            cryptoWalletManagerGenerateEvent (cwm, (BRCryptoWalletManagerEvent) {
                CRYPTO_WALLET_MANAGER_EVENT_WALLET_DELETED,
                { .wallet = cryptoWalletTake (wallet) }
            });
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);

    // drop reference outside of lock to avoid potential case where release function runs
    if (NULL != managerWallet) cryptoWalletGive (managerWallet);
}

// MARK: - Start/Stop

extern void
cryptoWalletManagerStart (BRCryptoWalletManager cwm) {
    // Start the CWM 'Event Handler'
    eventHandlerStart (cwm->handler);

    // {P2P,QRY} Manager - on connect
}

extern void
cryptoWalletManagerStop (BRCryptoWalletManager cwm) {
    // Stop the CWM 'Event Handler'
    eventHandlerStop (cwm->handler);

    // {P2P,QRY} Manager - on disconnect
}

/// MARK: - Connect/Disconnect/Sync

extern void
cryptoWalletManagerConnect (BRCryptoWalletManager cwm,
                            BRCryptoPeer peer) {
    switch (cryptoWalletManagerGetStateType (cwm)) {
        case CRYPTO_WALLET_MANAGER_STATE_CREATED:
        case CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED: {

            // Go to the connected state.
            if (CRYPTO_CLIENT_P2P_MANAGER_TYPE == cwm->canSend.type ||
                CRYPTO_CLIENT_P2P_MANAGER_TYPE == cwm->canSync.type)
                cryptoClientP2PManagerConnect (cwm->p2pManager, peer);
            else
                // TODO: CORE-1059 - Do we require cryptoClientP2PManagerConnect to set BRCryptoWalletManager state?
                cryptoWalletManagerSetState (cwm, cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_CONNECTED));

            // Start the QRY Manager
            cryptoClientQRYManagerConnect (cwm->qryManager);
            break;
        }
            
        case CRYPTO_WALLET_MANAGER_STATE_CONNECTED:
            break;

        case CRYPTO_WALLET_MANAGER_STATE_SYNCING:
            if (CRYPTO_CLIENT_P2P_MANAGER_TYPE == cwm->canSend.type ||
                CRYPTO_CLIENT_P2P_MANAGER_TYPE == cwm->canSync.type)
                cryptoClientP2PManagerConnect (cwm->p2pManager, peer);
            else
                // TODO: CORE-1059 - Do we require cryptoClientP2PManagerConnect to set BRCryptoWalletManager state?
                cryptoWalletManagerSetState (cwm, cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_CONNECTED));

            break;

        case CRYPTO_WALLET_MANAGER_STATE_DELETED:
            break;
    }
}

extern void
cryptoWalletManagerDisconnect (BRCryptoWalletManager cwm) {
    switch (cryptoWalletManagerGetStateType (cwm)) {
        case CRYPTO_WALLET_MANAGER_STATE_CREATED:
        case CRYPTO_WALLET_MANAGER_STATE_CONNECTED:
        case CRYPTO_WALLET_MANAGER_STATE_SYNCING:
            // TODO: CORE-1059 - De we require cryptoClientP2PManagerDisconnect to set BRCryptoWalletManager state?
            if (NULL != cwm->p2pManager) cryptoClientP2PManagerDisconnect (cwm->p2pManager);
            cryptoClientQRYManagerDisconnect (cwm->qryManager);

            cryptoWalletManagerSetState (cwm, cryptoWalletManagerStateDisconnectedInit (cryptoWalletManagerDisconnectReasonRequested()));
            break;

        case CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED:
            break;

        case CRYPTO_WALLET_MANAGER_STATE_DELETED:
            break;
    }
}

extern void
cryptoWalletManagerSync (BRCryptoWalletManager cwm) {
    cryptoWalletManagerSyncToDepth (cwm, CRYPTO_SYNC_DEPTH_FROM_CREATION);
}

extern void
cryptoWalletManagerSyncToDepth (BRCryptoWalletManager cwm,
                                BRCryptoSyncDepth depth) {
    if (CRYPTO_WALLET_MANAGER_STATE_CONNECTED == cryptoWalletManagerGetStateType (cwm))
        cryptoClientSync (cwm->canSync, depth, cryptoNetworkGetHeight(cwm->network));
}

// MARK: - Wipe


extern void
cryptoWalletManagerWipe (BRCryptoNetwork network,
                         const char *path) {

    const char *currencyName = cryptoBlockChainTypeGetCurrencyCode (cryptoNetworkGetType(network));
    const char *networkName  = cryptoNetworkGetDesc(network);

    pthread_mutex_lock (&network->lock);
    fileServiceWipe (path, currencyName, networkName);
    pthread_mutex_unlock (&network->lock);
}

// MARK: - Transfer Sign/Submit

extern BRCryptoBoolean
cryptoWalletManagerSign (BRCryptoWalletManager manager,
                         BRCryptoWallet wallet,
                         BRCryptoTransfer transfer,
                         const char *paperKey) {
    // Derived the seed used for signing.
    UInt512 seed = cryptoAccountDeriveSeed(paperKey);

    BRCryptoBoolean success = manager->handlers->signTransactionWithSeed (manager,
                                                                          wallet,
                                                                          transfer,
                                                                          seed);
    if (CRYPTO_TRUE == success)
        cryptoTransferSetState (transfer, cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SIGNED));

    // Zero-out the seed.
    seed = UINT512_ZERO;

    return success;
}

static BRCryptoBoolean
cryptoWalletManagerSignWithKey (BRCryptoWalletManager manager,
                                BRCryptoWallet wallet,
                                BRCryptoTransfer transfer,
                                BRCryptoKey key) {
    BRCryptoBoolean success =  manager->handlers->signTransactionWithKey (manager,
                                                                          wallet,
                                                                          transfer,
                                                                          key);
    if (CRYPTO_TRUE == success)
        cryptoTransferSetState (transfer, cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SIGNED));

    return success;
}

extern void
cryptoWalletManagerSubmitSigned (BRCryptoWalletManager cwm,
                                 BRCryptoWallet wallet,
                                 BRCryptoTransfer transfer) {
    BRCryptoUnit unit       = transfer->unit;
    BRCryptoUnit unitForFee = transfer->unitForFee;

    BRCryptoCurrency currencyForFee = cryptoUnitGetCurrency(unitForFee);

    // See if `transfer` belongs in another wallet too.  It will if it is not RECEIVED and
    // has a unitForFee that differs (in currency) from unit.
    BRCryptoWallet walletForFee = ((CRYPTO_TRANSFER_RECEIVED != cryptoTransferGetDirection(transfer) &&
                                    CRYPTO_FALSE == cryptoUnitIsCompatible (unit, unitForFee))
                                   ? cryptoWalletManagerGetWalletForCurrency(cwm, currencyForFee)
                                   : NULL);

    cryptoWalletAddTransfer (wallet, transfer);
    if (NULL != walletForFee) cryptoWalletAddTransfer (walletForFee, transfer);

    cryptoClientSend (cwm->canSend, wallet, transfer);

    BRCryptoWalletEvent event = cryptoWalletEventCreateTransferSubmitted (transfer);

    cryptoWalletGenerateEvent (wallet, cryptoWalletEventTake (event));
    if (NULL != walletForFee) {
        cryptoWalletGenerateEvent (walletForFee, cryptoWalletEventTake (event));
        cryptoWalletGive (walletForFee);
    }

    cryptoWalletEventGive(event);
    cryptoCurrencyGive(currencyForFee);
}

extern void
cryptoWalletManagerSubmit (BRCryptoWalletManager manager,
                           BRCryptoWallet wallet,
                           BRCryptoTransfer transfer,
                           const char *paperKey) {

    if (CRYPTO_TRUE == cryptoWalletManagerSign (manager, wallet, transfer, paperKey))
        cryptoWalletManagerSubmitSigned (manager, wallet, transfer);
}

extern void
cryptoWalletManagerSubmitForKey (BRCryptoWalletManager manager,
                                 BRCryptoWallet wallet,
                                 BRCryptoTransfer transfer,
                                 BRCryptoKey key) {
    // Signing requires `key` to have a secret (that is, be a private key).
    if (!cryptoKeyHasSecret(key)) return;

    if (CRYPTO_TRUE == cryptoWalletManagerSignWithKey(manager, wallet, transfer, key))
        cryptoWalletManagerSubmitSigned (manager, wallet, transfer);
}

// MARK: - Estimate Limit/Fee

extern BRCryptoAmount
cryptoWalletManagerEstimateLimit (BRCryptoWalletManager manager,
                                  BRCryptoWallet  wallet,
                                  BRCryptoBoolean asMaximum,
                                  BRCryptoAddress target,
                                  BRCryptoNetworkFee fee,
                                  BRCryptoBoolean *needEstimate,
                                  BRCryptoBoolean *isZeroIfInsuffientFunds) {
    assert (NULL != needEstimate && NULL != isZeroIfInsuffientFunds);

    BRCryptoUnit unit = cryptoUnitGetBaseUnit (wallet->unit);

    // By default, we don't need an estimate
    *needEstimate = CRYPTO_FALSE;

    // By default, zero does not indicate insufficient funds
    *isZeroIfInsuffientFunds = CRYPTO_FALSE;

    BRCryptoAmount limit = manager->handlers->estimateLimit (manager,
                                                             wallet,
                                                             asMaximum,
                                                             target,
                                                             fee,
                                                             needEstimate,
                                                             isZeroIfInsuffientFunds,
                                                             unit);

    cryptoUnitGive (unit);
    return limit;
}


extern void
cryptoWalletManagerEstimateFeeBasis (BRCryptoWalletManager manager,
                                     BRCryptoWallet  wallet,
                                     BRCryptoCookie cookie,
                                     BRCryptoAddress target,
                                     BRCryptoAmount  amount,
                                     BRCryptoNetworkFee fee,
                                     size_t attributesCount,
                                     OwnershipKept BRCryptoTransferAttribute *attributes) {
    BRCryptoFeeBasis feeBasis = manager->handlers->estimateFeeBasis (manager,
                                                                     wallet,
                                                                     cookie,
                                                                     target,
                                                                     amount,
                                                                     fee,
                                                                     attributesCount,
                                                                     attributes);
    if (NULL != feeBasis)
        cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateFeeBasisEstimated (CRYPTO_SUCCESS, cookie, feeBasis));

    cryptoFeeBasisGive (feeBasis);
}

extern void
cryptoWalletManagerEstimateFeeBasisForPaymentProtocolRequest (BRCryptoWalletManager cwm,
                                                              BRCryptoWallet wallet,
                                                              BRCryptoCookie cookie,
                                                              BRCryptoPaymentProtocolRequest request,
                                                              BRCryptoNetworkFee fee) {
    const BRCryptoPaymentProtocolHandlers * paymentHandlers = cryptoHandlersLookup(cryptoWalletGetType(wallet))->payment;

    assert (NULL != paymentHandlers);

    BRCryptoFeeBasis feeBasis = paymentHandlers->estimateFeeBasis (request,
                                                                   cwm,
                                                                   wallet,
                                                                   cookie,
                                                                   fee);
    if (NULL != feeBasis)
        cryptoWalletGenerateEvent (wallet, cryptoWalletEventCreateFeeBasisEstimated (CRYPTO_SUCCESS, cookie, feeBasis));

    cryptoFeeBasisGive (feeBasis);
}

// MARK: - Sweeper

extern BRCryptoWalletSweeperStatus
cryptoWalletManagerWalletSweeperValidateSupported (BRCryptoWalletManager cwm,
                                                   BRCryptoWallet wallet,
                                                   BRCryptoKey key) {
    if (cryptoNetworkGetType (cwm->network) != cryptoWalletGetType (wallet)) {
        return CRYPTO_WALLET_SWEEPER_INVALID_ARGUMENTS;
    }
    
    if (CRYPTO_FALSE == cryptoKeyHasSecret (key)) {
        return CRYPTO_WALLET_SWEEPER_INVALID_KEY;
    }
    
    return cwm->handlers->validateSweeperSupported (cwm, wallet, key);
}

extern BRCryptoWalletSweeper
cryptoWalletManagerCreateWalletSweeper (BRCryptoWalletManager cwm,
                                        BRCryptoWallet wallet,
                                        BRCryptoKey key) {
    assert (cryptoKeyHasSecret (key));
    
    return cwm->handlers->createSweeper (cwm,
                                         wallet,
                                         key);
}

extern const char *
cryptoWalletManagerEventTypeString (BRCryptoWalletManagerEventType t) {
    switch (t) {
        case CRYPTO_WALLET_MANAGER_EVENT_CREATED:
        return "CRYPTO_WALLET_MANAGER_EVENT_CREATED";

        case CRYPTO_WALLET_MANAGER_EVENT_CHANGED:
        return "CRYPTO_WALLET_MANAGER_EVENT_CHANGED";

        case CRYPTO_WALLET_MANAGER_EVENT_DELETED:
        return "CRYPTO_WALLET_MANAGER_EVENT_DELETED";

        case CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED:
        return "CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED";

        case CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED:
        return "CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED";

        case CRYPTO_WALLET_MANAGER_EVENT_WALLET_DELETED:
        return "CRYPTO_WALLET_MANAGER_EVENT_WALLET_DELETED";

        case CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED:
        return "CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED";

        case CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES:
        return "CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES";

        case CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED:
        return "CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED";

        case CRYPTO_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED:
        return "CRYPTO_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED";

        case CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED:
        return "CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED";
    }
    return "<CRYPTO_WALLET_MANAGER_EVENT_TYPE_UNKNOWN>";
}

/// MARK: Disconnect Reason

extern BRCryptoWalletManagerDisconnectReason
cryptoWalletManagerDisconnectReasonRequested(void) {
    return (BRCryptoWalletManagerDisconnectReason) {
        CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_REQUESTED
    };
}

extern BRCryptoWalletManagerDisconnectReason
cryptoWalletManagerDisconnectReasonUnknown(void) {
    return (BRCryptoWalletManagerDisconnectReason) {
        CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_UNKNOWN
    };
}

extern BRCryptoWalletManagerDisconnectReason
cryptoWalletManagerDisconnectReasonPosix(int errnum) {
    return (BRCryptoWalletManagerDisconnectReason) {
        CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_POSIX,
        { .posix = { errnum } }
    };
}

extern char *
cryptoWalletManagerDisconnectReasonGetMessage(BRCryptoWalletManagerDisconnectReason *reason) {
    char *message = NULL;

    switch (reason->type) {
        case CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_POSIX: {
            if (NULL != (message = strerror (reason->u.posix.errnum))) {
                message = strdup (message);
            }
            break;
        }
        default: {
            break;
        }
    }

    return message;
}

/// MARK: Sync Stopped Reason

extern BRCryptoSyncStoppedReason
cryptoSyncStoppedReasonComplete(void) {
    return (BRCryptoSyncStoppedReason) {
        CRYPTO_SYNC_STOPPED_REASON_COMPLETE
    };
}

extern BRCryptoSyncStoppedReason
cryptoSyncStoppedReasonRequested(void) {
    return (BRCryptoSyncStoppedReason) {
        CRYPTO_SYNC_STOPPED_REASON_REQUESTED
    };
}

extern BRCryptoSyncStoppedReason
cryptoSyncStoppedReasonUnknown(void) {
    return (BRCryptoSyncStoppedReason) {
        CRYPTO_SYNC_STOPPED_REASON_UNKNOWN
    };
}

extern BRCryptoSyncStoppedReason
cryptoSyncStoppedReasonPosix(int errnum) {
    return (BRCryptoSyncStoppedReason) {
        CRYPTO_SYNC_STOPPED_REASON_POSIX,
        { .posix = { errnum } }
    };
}

extern char *
cryptoSyncStoppedReasonGetMessage(BRCryptoSyncStoppedReason *reason) {
    char *message = NULL;

    switch (reason->type) {
        case CRYPTO_SYNC_STOPPED_REASON_POSIX: {
            if (NULL != (message = strerror (reason->u.posix.errnum))) {
                message = strdup (message);
            }
            break;
        }
        default: {
            break;
        }
    }

    return message;
}

/// MARK: Sync Mode

extern const char *
cryptoSyncModeString (BRCryptoSyncMode m) {
    switch (m) {
        case CRYPTO_SYNC_MODE_API_ONLY:
        return "CRYPTO_SYNC_MODE_API_ONLY";
        case CRYPTO_SYNC_MODE_API_WITH_P2P_SEND:
        return "CRYPTO_SYNC_MODE_API_WITH_P2P_SEND";
        case CRYPTO_SYNC_MODE_P2P_WITH_API_SYNC:
        return "CRYPTO_SYNC_MODE_P2P_WITH_API_SYNC";
        case CRYPTO_SYNC_MODE_P2P_ONLY:
        return "CRYPTO_SYNC_MODE_P2P_ONLY";
    }
}

// MARK: - Periodic Dispatcher

static void
cryptoWalletManagerPeriodicDispatcher (BREventHandler handler,
                                       BREventTimeout *event) {
    BRCryptoWalletManager cwm = (BRCryptoWalletManager) event->context;
    cryptoClientSyncPeriodic (cwm->canSync);
}

// MARK: - Transaction/Transfer Bundle

private_extern void
cryptoWalletManagerSaveTransactionBundle (BRCryptoWalletManager manager,
                                          OwnershipKept BRCryptoClientTransactionBundle bundle) {
    if (NULL != manager->handlers->saveTransactionBundle)
        manager->handlers->saveTransactionBundle (manager, bundle);
    else if (fileServiceHasType (manager->fileService, CRYPTO_FILE_SERVICE_TYPE_TRANSACTION))
        fileServiceSave (manager->fileService, CRYPTO_FILE_SERVICE_TYPE_TRANSACTION, bundle);
}

private_extern void
cryptoWalletManagerSaveTransferBundle (BRCryptoWalletManager manager,
                                       OwnershipKept BRCryptoClientTransferBundle bundle) {
    if (NULL != manager->handlers->saveTransferBundle)
        manager->handlers->saveTransferBundle (manager, bundle);
    else if (fileServiceHasType (manager->fileService, CRYPTO_FILE_SERVICE_TYPE_TRANSFER))
        fileServiceSave (manager->fileService, CRYPTO_FILE_SERVICE_TYPE_TRANSFER, bundle);
}

private_extern void
cryptoWalletManagerRecoverTransfersFromTransactionBundle (BRCryptoWalletManager cwm,
                                                          OwnershipKept BRCryptoClientTransactionBundle bundle) {
    cwm->handlers->recoverTransfersFromTransactionBundle (cwm, bundle);
}

private_extern void
cryptoWalletManagerRecoverTransferFromTransferBundle (BRCryptoWalletManager cwm,
                                                      OwnershipKept BRCryptoClientTransferBundle bundle) {
    cwm->handlers->recoverTransferFromTransferBundle (cwm, bundle);
}

private_extern void
cryptoWalletManagerRecoverTransferAttributesFromTransferBundle (BRCryptoWallet wallet,
                                                                BRCryptoTransfer transfer,
                                                                OwnershipKept BRCryptoClientTransferBundle bundle) {
    // If we are passed in attribues, they will replace any attribute already held
    // in `genTransfer`.  Specifically, for example, if we created an XRP transfer, then
    // we might have a 'DestinationTag'.  If the attributes provided do not include
    // 'DestinatinTag' then that attribute will be lost.  Losing such an attribute would
    // indicate a BlockSet error in processing transfers.
    if (bundle->attributesCount > 0) {
        BRCryptoAddress target = cryptoTransferGetTargetAddress (transfer);

        // Build the transfer attributes
        BRArrayOf(BRCryptoTransferAttribute) attributes;
        array_new(attributes, bundle->attributesCount);
        for (size_t index = 0; index < bundle->attributesCount; index++) {
            const char *key = bundle->attributeKeys[index];
            BRCryptoBoolean isRequiredAttribute;
            BRCryptoBoolean isAttribute = cryptoWalletHasTransferAttributeForKey (wallet,
                                                                                  target,
                                                                                  key,
                                                                                  &isRequiredAttribute);
            if (CRYPTO_TRUE == isAttribute)
                array_add (attributes,
                           cryptoTransferAttributeCreate(key,
                                                         bundle->attributeVals[index],
                                                         isRequiredAttribute));
        }
        
        cryptoTransferSetAttributes (transfer, array_count(attributes), attributes);
        cryptoTransferAttributeArrayRelease (attributes);
        cryptoAddressGive (target);
    }
}

private_extern BRCryptoFeeBasis
cryptoWalletManagerRecoverFeeBasisFromFeeEstimate (BRCryptoWalletManager cwm,
                                                   BRCryptoNetworkFee networkFee,
                                                   BRCryptoFeeBasis initialFeeBasis,
                                                   double costUnits,
                                                   size_t attributesCount,
                                                   OwnershipKept const char **attributeKeys,
                                                   OwnershipKept const char **attributeVals) {
    assert (NULL != cwm->handlers->recoverFeeBasisFromFeeEstimate); // not supported by chain
    return cwm->handlers->recoverFeeBasisFromFeeEstimate (cwm,
                                                          networkFee,
                                                          initialFeeBasis,
                                                          costUnits,
                                                          attributesCount,
                                                          attributeKeys,
                                                          attributeVals);
}
