//
//  WKWalletManager.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <assert.h>
#include <arpa/inet.h>      // struct in_addr
#include <ctype.h>          // toupper()

#include "WKBase.h"

#include "WKKeyP.h"
#include "WKAccountP.h"
#include "WKNetworkP.h"
#include "WKAddressP.h"
#include "WKAmountP.h"
#include "WKFeeBasisP.h"
#include "WKTransferP.h"
#include "WKWalletP.h"
#include "WKPaymentP.h"
#include "WKClientP.h"
#include "WKFileService.h"

#include "WKWalletManager.h"
#include "WKWalletManagerP.h"

#include "WKHandlersP.h"

#include "bitcoin/BRBitcoinMerkleBlock.h"
#include "bitcoin/BRBitcoinPeer.h"
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
wkWalletManagerBoundSamplingPeriod (unsigned int milliseconds) {
    return (milliseconds > CWM_MAXIMUM_SAMPLING_PERIOD_IN_MILLISECONDS
            ? CWM_MAXIMUM_SAMPLING_PERIOD_IN_MILLISECONDS
            : (milliseconds < CWM_MINIMUM_SAMPLING_PERIOD_IN_MILLISECONDS
               ? CWM_MINIMUM_SAMPLING_PERIOD_IN_MILLISECONDS
               : milliseconds));
}

uint64_t BLOCK_HEIGHT_UNBOUND_VALUE = UINT64_MAX;

static void
wkWalletManagerPeriodicDispatcher (BREventHandler handler,
                                       BREventTimeout *event);

static void
wkWalletManagerFileServiceErrorHandler (BRFileServiceContext context,
                                            BRFileService fs,
                                            BRFileServiceError error);

IMPLEMENT_WK_GIVE_TAKE (WKWalletManager, wkWalletManager)

/// =============================================================================================
///
/// MARK: - Wallet Manager State
///
///
private_extern WKWalletManagerState
wkWalletManagerStateInit(WKWalletManagerStateType type) {
    switch (type) {
        case WK_WALLET_MANAGER_STATE_CREATED:
        case WK_WALLET_MANAGER_STATE_CONNECTED:
        case WK_WALLET_MANAGER_STATE_SYNCING:
        case WK_WALLET_MANAGER_STATE_DELETED:
            return (WKWalletManagerState) { type };
        case WK_WALLET_MANAGER_STATE_DISCONNECTED:
            assert (0); // if you are hitting this, use wkWalletManagerStateDisconnectedInit!
            return (WKWalletManagerState) {
                WK_WALLET_MANAGER_STATE_DISCONNECTED,
                { .disconnected = { wkWalletManagerDisconnectReasonUnknown() } }
            };
    }
}

private_extern WKWalletManagerState
wkWalletManagerStateDisconnectedInit(WKWalletManagerDisconnectReason reason) {
    return (WKWalletManagerState) {
        WK_WALLET_MANAGER_STATE_DISCONNECTED,
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
static BRArrayOf(WKCurrency)
wkWalletManagerGetCurrenciesOfInterest (WKWalletManager cwm) {
    BRArrayOf(WKCurrency) currencies;

    array_new (currencies, 3);
    return currencies;
}

static void
wkWalletManagerReleaseCurrenciesOfInterest (WKWalletManager cwm,
                                                BRArrayOf(WKCurrency) currencies) {
    for (size_t index = 0; index < array_count(currencies); index++)
        wkCurrencyGive (currencies[index]);
    array_free (currencies);
}
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

static void // not locked; called during manager init
wkWalletManagerInitialTransferBundlesLoad (WKWalletManager manager) {
    assert (NULL == manager->bundleTransfers);

    BRSetOf(WKClientTransferBundle) bundles = wkClientTransferBundleSetCreate (25);

    if (fileServiceHasType (manager->fileService, WK_FILE_SERVICE_TYPE_TRANSFER) &&
        1 != fileServiceLoad (manager->fileService, bundles, WK_FILE_SERVICE_TYPE_TRANSFER, 1)) {
        printf ("CRY: %4s: failed to load transfer bundles",
                wkNetworkTypeGetCurrencyCode (manager->type));
        wkClientTransferBundleSetRelease(bundles);
        return;
    }
    size_t sortedBundlesCount = BRSetCount(bundles);

    printf ("CRY: %4s: loaded %4zu transfer bundles\n",
            wkNetworkTypeGetCurrencyCode (manager->type),
            sortedBundlesCount);

    if (0 != sortedBundlesCount) {
        array_new (manager->bundleTransfers, sortedBundlesCount);
        BRSetAll (bundles, (void**) manager->bundleTransfers, sortedBundlesCount);
        array_set_count (manager->bundleTransfers, sortedBundlesCount);

        mergesort_brd (manager->bundleTransfers, sortedBundlesCount, sizeof (WKClientTransferBundle),
                       wkClientTransferBundleCompareByBlockheightForSort);
    }

    // Don't release the set's bundles
    BRSetFree(bundles);
}

static void // called wtih manager->lock
wkWalletManagerInitialTransferBundlesRecover (WKWalletManager manager) {
    if (NULL != manager->bundleTransfers) {
        for (size_t index = 0; index < array_count(manager->bundleTransfers); index++) {
            wkWalletManagerRecoverTransferFromTransferBundle (manager, manager->bundleTransfers[index]);
        }

        array_free_all (manager->bundleTransfers, wkClientTransferBundleRelease);
        manager->bundleTransfers = NULL;
    }
}

static void // not locked; called during manager init
wkWalletManagerInitialTransactionBundlesLoad (WKWalletManager manager) {
    assert (NULL == manager->bundleTransactions);

    BRSetOf(WKClientTransactionBundle) bundles = wkClientTransactionBundleSetCreate (25);
    if (fileServiceHasType (manager->fileService, WK_FILE_SERVICE_TYPE_TRANSACTION) &&
        1 != fileServiceLoad (manager->fileService, bundles, WK_FILE_SERVICE_TYPE_TRANSACTION, 1)) {
        wkClientTransactionBundleSetRelease (bundles);
        printf ("CRY: %4s: failed to load transaction bundles",
                wkNetworkTypeGetCurrencyCode (manager->type));
        return;
    }
    size_t sortedBundlesCount = BRSetCount(bundles);

    printf ("CRY: %4s: loaded %4zu transaction bundles\n",
            wkNetworkTypeGetCurrencyCode (manager->type),
            sortedBundlesCount);

    if (0 != sortedBundlesCount) {
        BRArrayOf(WKClientTransactionBundle) sortedBundles;
        array_new (sortedBundles, sortedBundlesCount);
        BRSetAll (bundles, (void**) sortedBundles, sortedBundlesCount);
        array_set_count(sortedBundles, sortedBundlesCount);

        mergesort_brd (sortedBundles, sortedBundlesCount, sizeof (WKClientTransactionBundle),
                       wkClientTransactionBundleCompareByBlockheightForSort);

        manager->bundleTransactions = sortedBundles;
    }

    // Don't release the set's bundles
    BRSetFree(bundles);
}

static void // called wtih manager->lock
wkWalletManagerInitialTransactionBundlesRecover (WKWalletManager manager) {
    if (NULL != manager->bundleTransactions) {
        for (size_t index = 0; index < array_count(manager->bundleTransactions); index++) {
            wkWalletManagerRecoverTransfersFromTransactionBundle (manager, manager->bundleTransactions[index]);
        }

        array_free_all (manager->bundleTransactions, wkClientTransactionBundleRelease);
        manager->bundleTransactions = NULL;
    }
}

#define MACRO_GENERATION
#include "WKLog.h"

static pthread_once_t initWKLogsOnce = PTHREAD_ONCE_INIT;
static void initializeWalletKitLogs() {
    LOG_REGISTER_MODULE(WK);
    LOG_ADD_SUBMODULE(WK,CRY);
}


extern WKWalletManager
wkWalletManagerAllocAndInit (size_t sizeInBytes,
                                 WKNetworkType type,
                                 WKWalletManagerListener listener,
                                 WKClient client,
                                 WKAccount account,
                                 WKNetwork network,
                                 WKAddressScheme scheme,
                                 const char *path,
                                 WKClientQRYByType byType,
                                 WKWalletManagerCreateContext createContext,
                                 WKWalletManagerCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct WKWalletManagerRecord));
    assert (type == wkNetworkGetType(network));

    pthread_once (&initWKLogsOnce, initializeWalletKitLogs);
    
    WKWalletManager manager = calloc (1, sizeInBytes);
    if (NULL == manager) return NULL;

    manager->type = type;
    manager->handlers = wkHandlersLookup(type)->manager;
    network->sizeInBytes = sizeInBytes;
    manager->listener = listener;

    manager->client  = client;
    manager->network = wkNetworkTake (network);
    manager->account = wkAccountTake (account);
    manager->state   = wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CREATED);
    manager->addressScheme = scheme;
    manager->path = strdup (path);

    manager->byType = byType;

    // Hold this early
    manager->ref = WK_REF_ASSIGN (wkWalletManagerRelease);

    // Initialize to NULL, for now.
    manager->qryManager = NULL;
    manager->p2pManager = NULL;
    manager->wallet     = NULL;
    array_new (manager->wallets, 1);

    // File Service
    const char *currencyName = wkNetworkTypeGetCurrencyCode (manager->type);
    const char *networkName  = wkNetworkGetDesc(network);

    // TODO: Replace `createFileService` with `getFileServiceSpecifications`
    manager->fileService = manager->handlers->createFileService (manager,
                                                                 manager->path,
                                                                 currencyName,
                                                                 networkName,
                                                                 manager,
                                                                 wkWalletManagerFileServiceErrorHandler);

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
                                      wkWalletManagerBoundSamplingPeriod ((1000 * wkNetworkGetConfirmationPeriodInSeconds(network)) / CWM_CONFIRMATION_PERIOD_FACTOR),
                                      (BREventDispatcher) wkWalletManagerPeriodicDispatcher,
                                      (void*) manager);

    manager->listenerWallet = wkListenerCreateWalletListener (&manager->listener, manager);

    pthread_mutex_init_brd (&manager->lock, PTHREAD_MUTEX_RECURSIVE);

    WKTimestamp   earliestAccountTime = wkAccountGetTimestamp (account);
    WKBlockNumber earliestBlockNumber = wkNetworkGetBlockNumberAtOrBeforeTimestamp(network, earliestAccountTime);
    WKBlockNumber latestBlockNumber   = wkNetworkGetHeight (network);

    // Setup the QRY Manager
    manager->qryManager = wkClientQRYManagerCreate (client,
                                                        manager,
                                                        manager->byType,
                                                        earliestBlockNumber,
                                                        latestBlockNumber);

    if (createCallback) createCallback (createContext, manager);
    
    // Announce the created manager; this must preceed any wallet created/added events
    wkWalletManagerGenerateEvent (manager, (WKWalletManagerEvent) {
        WK_WALLET_MANAGER_EVENT_CREATED
    });

    wkWalletManagerInitialTransferBundlesLoad (manager);
    wkWalletManagerInitialTransactionBundlesLoad (manager);

    // Create the primary wallet
    manager->wallet = wkWalletManagerCreateWalletInitialized (manager,
                                                                  network->currency,
                                                                  manager->bundleTransactions,
                                                                  manager->bundleTransfers);

    // Create the P2P manager
    manager->p2pManager = manager->handlers->createP2PManager (manager);

    // Allow callers to be in an atomic region.
    pthread_mutex_lock (&manager->lock);
    return manager;
}

extern WKWalletManager
wkWalletManagerCreate (WKWalletManagerListener listener,
                           WKClient client,
                           WKAccount account,
                           WKNetwork network,
                           WKSyncMode mode,
                           WKAddressScheme scheme,
                           const char *path) {
    // Only create a wallet manager for accounts that are initialized on network.
    if (WK_FALSE == wkNetworkIsAccountInitialized (network, account))
        return NULL;

    // Lookup the handler for the network's type.
    WKNetworkType type = wkNetworkGetType(network);
    const WKWalletManagerHandlers *handlers = wkHandlersLookup(type)->manager;

    // Create the manager
    WKWalletManager manager = handlers->create (listener,
                                                      client,
                                                      account,
                                                      network,
                                                      mode,
                                                      scheme,
                                                      path);
    if (NULL == manager) return NULL;

    // Recover transfers and transactions
    wkWalletManagerInitialTransferBundlesRecover (manager);
    wkWalletManagerInitialTransactionBundlesRecover (manager);

    pthread_mutex_unlock (&manager->lock);

    // Set the mode for QRY or P2P syncing
    wkWalletManagerSetMode (manager, mode);

    return manager;
}

static void
wkWalletManagerFileServiceErrorHandler (BRFileServiceContext context,
                                            BRFileService fs,
                                            BRFileServiceError error) {
    switch (error.type) {
        case FILE_SERVICE_IMPL:
            // This actually a FATAL - an unresolvable coding error.
            LOG (LL_FATAL, WK_CRY, "FileService Error: IMPL: %s\n", error.u.impl.reason);
            break;
        case FILE_SERVICE_UNIX:
            LOG (LL_ERROR, WK_CRY, "FileService Error: UNIX: %s", strerror(error.u.unx.error));
            break;
        case FILE_SERVICE_ENTITY:
            // This is likely a coding error too.
            LOG (LL_ERROR, WK_CRY, "FileService Error: ENTITY (%s): %s",
                 error.u.entity.type,
                 error.u.entity.reason);
            break;
        case FILE_SERVICE_SDB:
            LOG (LL_ERROR, WK_CRY, "FileService Error: SDB: (%d): %s",
                 error.u.sdb.code,
                 error.u.sdb.reason);
            break;
    }
    LOG (LL_ERROR, WK_CRY, "FileService Error: FORCED SYNC");

    // BRWalletManager bwm = (BRWalletManager) context;
    // TODO(fix): What do we actually want to happen here?
    // if (NULL != bwm->peerManager)
    //     btcPeerManagerRescan (bwm->peerManager);
}


static void
wkWalletManagerRelease (WKWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);

    // Ensure CWM is stopped...
    wkWalletManagerStop (cwm);

    // ... then release any type-specific resources
    cwm->handlers->release (cwm);

    // ... then release memory.
    wkAccountGive (cwm->account);
    wkNetworkGive (cwm->network);
    if (NULL != cwm->wallet) wkWalletGive (cwm->wallet);

    // .. then give all the wallets
    for (size_t index = 0; index < array_count(cwm->wallets); index++)
        wkWalletGive (cwm->wallets[index]);
    array_free (cwm->wallets);

    // ... then the p2p and qry managers
    if (NULL != cwm->p2pManager) wkClientP2PManagerRelease(cwm->p2pManager);
    wkClientQRYManagerRelease (cwm->qryManager);

    // ... then the fileService
    fileServiceRelease (cwm->fileService);

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

extern WKNetwork
wkWalletManagerGetNetwork (WKWalletManager cwm) {
    return wkNetworkTake (cwm->network);
}

extern WKBoolean
wkWalletManagerHasNetwork (WKWalletManager cwm,
                               WKNetwork network) {
    return AS_WK_BOOLEAN (cwm->network == network);
}

extern WKAccount
wkWalletManagerGetAccount (WKWalletManager cwm) {
    return wkAccountTake (cwm->account);
}

extern WKBoolean
wkWalletManagerHasAccount (WKWalletManager cwm,
                               WKAccount account) {
    return AS_WK_BOOLEAN (cwm->account == account);
}

extern void
wkWalletManagerSetMode (WKWalletManager cwm, WKSyncMode mode) {
    pthread_mutex_lock (&cwm->lock);

    // Get default p2p{Sync,Send} Managers
    WKClientSync p2pSync = (NULL != cwm->p2pManager
                                  ? wkClientP2PManagerAsSync (cwm->p2pManager)
                                  : wkClientQRYManagerAsSync (cwm->qryManager));
    WKClientSend p2pSend = (NULL != cwm->p2pManager
                                  ? wkClientP2PManagerAsSend (cwm->p2pManager)
                                  : wkClientQRYManagerAsSend (cwm->qryManager));

    WKClientSync qrySync = wkClientQRYManagerAsSync (cwm->qryManager);
    WKClientSend qrySend = wkClientQRYManagerAsSend (cwm->qryManager);

    // TODO: Manager Memory?

    // Set cwm->can{Sync,Send} based on mode.
    switch (mode) {
        case WK_SYNC_MODE_API_ONLY:
            cwm->canSync = qrySync;
            cwm->canSend = qrySend;
            break;
        case WK_SYNC_MODE_API_WITH_P2P_SEND:
            cwm->canSync = qrySync;
            cwm->canSend = p2pSend;
            break;
        case WK_SYNC_MODE_P2P_WITH_API_SYNC:
             // Initial sync w/ QRY, thereafter w/ P2P
            cwm->canSync = qrySync;
            cwm->canSend = p2pSend;
            break;
        case WK_SYNC_MODE_P2P_ONLY:
            cwm->canSync = p2pSync;
            cwm->canSend = p2pSend;
            break;
    }
    cwm->syncMode = mode;
    pthread_mutex_unlock (&cwm->lock);
}

extern WKSyncMode
wkWalletManagerGetMode (WKWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    WKSyncMode mode = cwm->syncMode;
    pthread_mutex_unlock (&cwm->lock);
    return mode;
}

extern WKWalletManagerState
wkWalletManagerGetState (WKWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    WKWalletManagerState state = cwm->state;
    pthread_mutex_unlock (&cwm->lock);
    return state;
}

extern WKWalletManagerStateType
wkWalletManagerGetStateType (WKWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    WKWalletManagerStateType type = cwm->state.type;
    pthread_mutex_unlock (&cwm->lock);

    return type;
}

private_extern void
wkWalletManagerSetState (WKWalletManager cwm,
                             WKWalletManagerState newState) {
    pthread_mutex_lock (&cwm->lock);
    WKWalletManagerState oldState = cwm->state;
    cwm->state = newState;
    pthread_mutex_unlock (&cwm->lock);

    if (oldState.type != newState.type)
        wkWalletManagerGenerateEvent (cwm, (WKWalletManagerEvent) {
            WK_WALLET_MANAGER_EVENT_CHANGED,
            { .state = { oldState, newState }}
        });
}

extern WKAddressScheme
wkWalletManagerGetAddressScheme (WKWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    WKAddressScheme scheme = cwm->addressScheme;
    pthread_mutex_unlock (&cwm->lock);
    return scheme;
}

extern void
wkWalletManagerSetAddressScheme (WKWalletManager cwm,
                                     WKAddressScheme scheme) {
    pthread_mutex_lock (&cwm->lock);
    cwm->addressScheme = scheme;
    pthread_mutex_unlock (&cwm->lock);
}

extern const char *
wkWalletManagerGetPath (WKWalletManager cwm) {
    return cwm->path;
}

extern void
wkWalletManagerSetNetworkReachable (WKWalletManager cwm,
                                        WKBoolean isNetworkReachable) {
    if (NULL != cwm->p2pManager) {
        wkClientP2PManagerSetNetworkReachable (cwm->p2pManager, isNetworkReachable);
    }
}

extern WKWallet
wkWalletManagerCreateWalletInitialized (WKWalletManager cwm,
                                            WKCurrency currency,
                                            Nullable OwnershipKept BRArrayOf(WKClientTransactionBundle) transactions,
                                            Nullable OwnershipKept BRArrayOf(WKClientTransferBundle) transfers) {
    WKWallet wallet = wkWalletManagerGetWalletForCurrency (cwm, currency);
    return (NULL == wallet
            ? cwm->handlers->createWallet (cwm, currency, transactions, transfers)
            : wallet);
}


extern WKWallet
wkWalletManagerCreateWallet (WKWalletManager cwm,
                                 WKCurrency currency) {
    return wkWalletManagerCreateWalletInitialized (cwm, currency, NULL, NULL);
}


extern WKWallet
wkWalletManagerGetWallet (WKWalletManager cwm) {
    return wkWalletTake (cwm->wallet);
}

extern WKWallet *
wkWalletManagerGetWallets (WKWalletManager cwm, size_t *count) {
    pthread_mutex_lock (&cwm->lock);
    *count = array_count (cwm->wallets);
    WKWallet *wallets = NULL;
    if (0 != *count) {
        wallets = calloc (*count, sizeof(WKWallet));
        for (size_t index = 0; index < *count; index++) {
            wallets[index] = wkWalletTake(cwm->wallets[index]);
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallets;
}


extern WKWallet
wkWalletManagerGetWalletForCurrency (WKWalletManager cwm,
                                         WKCurrency currency) {
    WKWallet wallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count(cwm->wallets); index++) {
        if (WK_TRUE == wkWalletHasCurrency (cwm->wallets[index], currency)) {
            wallet = wkWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}

static WKBoolean
wkWalletManagerHasWalletLock (WKWalletManager cwm,
                                  WKWallet wallet,
                                  bool needLock) {
    WKBoolean r = WK_FALSE;
    if (needLock) pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets) && WK_FALSE == r; index++) {
        r = wkWalletEqual(cwm->wallets[index], wallet);
    }
    if (needLock) pthread_mutex_unlock (&cwm->lock);
    return r;
}

extern WKBoolean
wkWalletManagerHasWallet (WKWalletManager cwm,
                              WKWallet wallet) {
    return wkWalletManagerHasWalletLock (cwm, wallet, true);
}

private_extern void
wkWalletManagerAddWallet (WKWalletManager cwm,
                              WKWallet wallet) {
    pthread_mutex_lock (&cwm->lock);
    if (WK_FALSE == wkWalletManagerHasWalletLock (cwm, wallet, false)) {
        array_add (cwm->wallets, wkWalletTake (wallet));
        wkWalletManagerGenerateEvent (cwm, (WKWalletManagerEvent) {
            WK_WALLET_MANAGER_EVENT_WALLET_ADDED,
            { .wallet = wkWalletTake (wallet) }
        });
    }
    pthread_mutex_unlock (&cwm->lock);
}

private_extern void
wkWalletManagerRemWallet (WKWalletManager cwm,
                              WKWallet wallet) {

    WKWallet managerWallet = NULL;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets); index++) {
        if (WK_TRUE == wkWalletEqual(cwm->wallets[index], wallet)) {
            managerWallet = cwm->wallets[index];
            array_rm (cwm->wallets, index);
            wkWalletManagerGenerateEvent (cwm, (WKWalletManagerEvent) {
                WK_WALLET_MANAGER_EVENT_WALLET_DELETED,
                { .wallet = wkWalletTake (wallet) }
            });
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);

    // drop reference outside of lock to avoid potential case where release function runs
    if (NULL != managerWallet) wkWalletGive (managerWallet);
}

// MARK: - Start/Stop

extern void
wkWalletManagerStart (WKWalletManager cwm) {
    // Start the CWM 'Event Handler'
    eventHandlerStart (cwm->handler);

    // {P2P,QRY} Manager - on connect
}

extern void
wkWalletManagerStop (WKWalletManager cwm) {
    // Stop the CWM 'Event Handler'
    eventHandlerStop (cwm->handler);

    // {P2P,QRY} Manager - on disconnect
}

/// MARK: - Connect/Disconnect/Sync

extern void
wkWalletManagerConnect (WKWalletManager cwm,
                            WKPeer peer) {
    switch (wkWalletManagerGetStateType (cwm)) {
        case WK_WALLET_MANAGER_STATE_CREATED:
        case WK_WALLET_MANAGER_STATE_DISCONNECTED: {

            // Go to the connected state.
            if (WK_CLIENT_P2P_MANAGER_TYPE == cwm->canSend.type ||
                WK_CLIENT_P2P_MANAGER_TYPE == cwm->canSync.type)
                wkClientP2PManagerConnect (cwm->p2pManager, peer);
            else
                // TODO: CORE-1059 - Do we require wkClientP2PManagerConnect to set WKWalletManager state?
                wkWalletManagerSetState (cwm, wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED));

            // Start the QRY Manager
            wkClientQRYManagerConnect (cwm->qryManager);
            break;
        }
            
        case WK_WALLET_MANAGER_STATE_CONNECTED:
            break;

        case WK_WALLET_MANAGER_STATE_SYNCING:
            if (WK_CLIENT_P2P_MANAGER_TYPE == cwm->canSend.type ||
                WK_CLIENT_P2P_MANAGER_TYPE == cwm->canSync.type)
                wkClientP2PManagerConnect (cwm->p2pManager, peer);
            else
                // TODO: CORE-1059 - Do we require wkClientP2PManagerConnect to set WKWalletManager state?
                wkWalletManagerSetState (cwm, wkWalletManagerStateInit (WK_WALLET_MANAGER_STATE_CONNECTED));

            break;

        case WK_WALLET_MANAGER_STATE_DELETED:
            break;
    }
}

extern void
wkWalletManagerDisconnect (WKWalletManager cwm) {
    switch (wkWalletManagerGetStateType (cwm)) {
        case WK_WALLET_MANAGER_STATE_CREATED:
        case WK_WALLET_MANAGER_STATE_CONNECTED:
        case WK_WALLET_MANAGER_STATE_SYNCING:
            // TODO: CORE-1059 - De we require wkClientP2PManagerDisconnect to set WKWalletManager state?
            if (NULL != cwm->p2pManager) wkClientP2PManagerDisconnect (cwm->p2pManager);
            wkClientQRYManagerDisconnect (cwm->qryManager);

            wkWalletManagerSetState (cwm, wkWalletManagerStateDisconnectedInit (wkWalletManagerDisconnectReasonRequested()));
            break;

        case WK_WALLET_MANAGER_STATE_DISCONNECTED:
            break;

        case WK_WALLET_MANAGER_STATE_DELETED:
            break;
    }
}

extern void
wkWalletManagerSync (WKWalletManager cwm) {
    wkWalletManagerSyncToDepth (cwm, WK_SYNC_DEPTH_FROM_CREATION);
}

extern void
wkWalletManagerSyncToDepth (WKWalletManager cwm,
                                WKSyncDepth depth) {
    if (WK_WALLET_MANAGER_STATE_CONNECTED == wkWalletManagerGetStateType (cwm))
        wkClientSync (cwm->canSync, depth, wkNetworkGetHeight(cwm->network));
}

// MARK: - Wipe


extern void
wkWalletManagerWipe (WKNetwork network,
                         const char *path) {

    const char *currencyName = wkNetworkTypeGetCurrencyCode (wkNetworkGetType(network));
    const char *networkName  = wkNetworkGetDesc(network);

    pthread_mutex_lock (&network->lock);
    fileServiceWipe (path, currencyName, networkName);
    pthread_mutex_unlock (&network->lock);
}

// MARK: - Transfer Sign/Submit

extern WKBoolean
wkWalletManagerSign (WKWalletManager manager,
                         WKWallet wallet,
                         WKTransfer transfer,
                         const char *paperKey) {
    // Derived the seed used for signing.
    UInt512 seed = wkAccountDeriveSeed(paperKey);

    WKBoolean success = manager->handlers->signTransactionWithSeed (manager,
                                                                          wallet,
                                                                          transfer,
                                                                          seed);
    if (WK_TRUE == success)
        wkTransferSetState (transfer, wkTransferStateInit (WK_TRANSFER_STATE_SIGNED));

    // Zero-out the seed.
    seed = UINT512_ZERO;

    return success;
}

static WKBoolean
wkWalletManagerSignWithKey (WKWalletManager manager,
                                WKWallet wallet,
                                WKTransfer transfer,
                                WKKey key) {
    WKBoolean success =  manager->handlers->signTransactionWithKey (manager,
                                                                          wallet,
                                                                          transfer,
                                                                          key);
    if (WK_TRUE == success)
        wkTransferSetState (transfer, wkTransferStateInit (WK_TRANSFER_STATE_SIGNED));

    return success;
}

extern void
wkWalletManagerSubmitSigned (WKWalletManager cwm,
                                 WKWallet wallet,
                                 WKTransfer transfer) {
    WKUnit unit       = transfer->unit;
    WKUnit unitForFee = transfer->unitForFee;

    WKCurrency currencyForFee = wkUnitGetCurrency(unitForFee);

    // See if `transfer` belongs in another wallet too.  It will if it is not RECEIVED and
    // has a unitForFee that differs (in currency) from unit.
    WKWallet walletForFee = ((WK_TRANSFER_RECEIVED != wkTransferGetDirection(transfer) &&
                                    WK_FALSE == wkUnitIsCompatible (unit, unitForFee))
                                   ? wkWalletManagerGetWalletForCurrency(cwm, currencyForFee)
                                   : NULL);

    wkWalletAddTransfer (wallet, transfer);
    if (NULL != walletForFee) wkWalletAddTransfer (walletForFee, transfer);

    wkClientSend (cwm->canSend, wallet, transfer);

    WKWalletEvent event = wkWalletEventCreateTransferSubmitted (transfer);

    wkWalletGenerateEvent (wallet, wkWalletEventTake (event));
    if (NULL != walletForFee) {
        wkWalletGenerateEvent (walletForFee, wkWalletEventTake (event));
        wkWalletGive (walletForFee);
    }

    wkWalletEventGive(event);
    wkCurrencyGive(currencyForFee);
}

extern void
wkWalletManagerSubmit (WKWalletManager manager,
                           WKWallet wallet,
                           WKTransfer transfer,
                           const char *paperKey) {

    if (WK_TRUE == wkWalletManagerSign (manager, wallet, transfer, paperKey))
        wkWalletManagerSubmitSigned (manager, wallet, transfer);
}

extern void
wkWalletManagerSubmitForKey (WKWalletManager manager,
                                 WKWallet wallet,
                                 WKTransfer transfer,
                                 WKKey key) {
    // Signing requires `key` to have a secret (that is, be a private key).
    if (!wkKeyHasSecret(key)) return;

    if (WK_TRUE == wkWalletManagerSignWithKey(manager, wallet, transfer, key))
        wkWalletManagerSubmitSigned (manager, wallet, transfer);
}

// MARK: - Estimate Limit/Fee

extern WKAmount
wkWalletManagerEstimateLimit (WKWalletManager manager,
                                  WKWallet  wallet,
                                  WKBoolean asMaximum,
                                  WKAddress target,
                                  WKNetworkFee fee,
                                  WKBoolean *needEstimate,
                                  WKBoolean *isZeroIfInsuffientFunds) {
    assert (NULL != needEstimate && NULL != isZeroIfInsuffientFunds);

    WKUnit unit = wkUnitGetBaseUnit (wallet->unit);

    // By default, we don't need an estimate
    *needEstimate = WK_FALSE;

    // By default, zero does not indicate insufficient funds
    *isZeroIfInsuffientFunds = WK_FALSE;

    WKAmount limit = manager->handlers->estimateLimit (manager,
                                                             wallet,
                                                             asMaximum,
                                                             target,
                                                             fee,
                                                             needEstimate,
                                                             isZeroIfInsuffientFunds,
                                                             unit);

    wkUnitGive (unit);
    return limit;
}


extern void
wkWalletManagerEstimateFeeBasis (WKWalletManager manager,
                                     WKWallet  wallet,
                                     WKCookie cookie,
                                     WKAddress target,
                                     WKAmount  amount,
                                     WKNetworkFee fee,
                                     size_t attributesCount,
                                     OwnershipKept WKTransferAttribute *attributes) {

    // Margin will be added, if appropriate for `manager`
    WKFeeBasis feeBasis = manager->handlers->estimateFeeBasis (manager,
                                                                     wallet,
                                                                     cookie,
                                                                     target,
                                                                     amount,
                                                                     fee,
                                                                     attributesCount,
                                                                     attributes);
    if (NULL != feeBasis)
        wkWalletGenerateEvent (wallet, wkWalletEventCreateFeeBasisEstimated (WK_SUCCESS, cookie, feeBasis));

    wkFeeBasisGive (feeBasis);
}

extern void
wkWalletManagerEstimateFeeBasisForPaymentProtocolRequest (WKWalletManager cwm,
                                                              WKWallet wallet,
                                                              WKCookie cookie,
                                                              WKPaymentProtocolRequest request,
                                                              WKNetworkFee fee) {
    const WKPaymentProtocolHandlers * paymentHandlers = wkHandlersLookup(wkWalletGetType(wallet))->payment;

    assert (NULL != paymentHandlers);

    WKFeeBasis feeBasis = paymentHandlers->estimateFeeBasis (request,
                                                                   cwm,
                                                                   wallet,
                                                                   cookie,
                                                                   fee);
    if (NULL != feeBasis)
        wkWalletGenerateEvent (wallet, wkWalletEventCreateFeeBasisEstimated (WK_SUCCESS, cookie, feeBasis));

    wkFeeBasisGive (feeBasis);
}

// MARK: - Sweeper

extern WKWalletSweeperStatus
wkWalletManagerWalletSweeperValidateSupported (WKWalletManager cwm,
                                                   WKWallet wallet,
                                                   WKKey key) {
    if (wkNetworkGetType (cwm->network) != wkWalletGetType (wallet)) {
        return WK_WALLET_SWEEPER_INVALID_ARGUMENTS;
    }
    
    if (WK_FALSE == wkKeyHasSecret (key)) {
        return WK_WALLET_SWEEPER_INVALID_KEY;
    }
    
    return cwm->handlers->validateSweeperSupported (cwm, wallet, key);
}

extern WKWalletSweeper
wkWalletManagerCreateWalletSweeper (WKWalletManager cwm,
                                        WKWallet wallet,
                                        WKKey key) {
    assert (wkKeyHasSecret (key));
    
    return cwm->handlers->createSweeper (cwm,
                                         wallet,
                                         key);
}

extern const char *
wkWalletManagerEventTypeString (WKWalletManagerEventType t) {
    switch (t) {
        case WK_WALLET_MANAGER_EVENT_CREATED:
        return "WK_WALLET_MANAGER_EVENT_CREATED";

        case WK_WALLET_MANAGER_EVENT_CHANGED:
        return "WK_WALLET_MANAGER_EVENT_CHANGED";

        case WK_WALLET_MANAGER_EVENT_DELETED:
        return "WK_WALLET_MANAGER_EVENT_DELETED";

        case WK_WALLET_MANAGER_EVENT_WALLET_ADDED:
        return "WK_WALLET_MANAGER_EVENT_WALLET_ADDED";

        case WK_WALLET_MANAGER_EVENT_WALLET_CHANGED:
        return "WK_WALLET_MANAGER_EVENT_WALLET_CHANGED";

        case WK_WALLET_MANAGER_EVENT_WALLET_DELETED:
        return "WK_WALLET_MANAGER_EVENT_WALLET_DELETED";

        case WK_WALLET_MANAGER_EVENT_SYNC_STARTED:
        return "WK_WALLET_MANAGER_EVENT_SYNC_STARTED";

        case WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES:
        return "WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES";

        case WK_WALLET_MANAGER_EVENT_SYNC_STOPPED:
        return "WK_WALLET_MANAGER_EVENT_SYNC_STOPPED";

        case WK_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED:
        return "WK_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED";

        case WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED:
        return "WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED";
    }
    return "<WK_WALLET_MANAGER_EVENT_TYPE_UNKNOWN>";
}

/// MARK: Disconnect Reason

extern WKWalletManagerDisconnectReason
wkWalletManagerDisconnectReasonRequested(void) {
    return (WKWalletManagerDisconnectReason) {
        WK_WALLET_MANAGER_DISCONNECT_REASON_REQUESTED
    };
}

extern WKWalletManagerDisconnectReason
wkWalletManagerDisconnectReasonUnknown(void) {
    return (WKWalletManagerDisconnectReason) {
        WK_WALLET_MANAGER_DISCONNECT_REASON_UNKNOWN
    };
}

extern WKWalletManagerDisconnectReason
wkWalletManagerDisconnectReasonPosix(int errnum) {
    return (WKWalletManagerDisconnectReason) {
        WK_WALLET_MANAGER_DISCONNECT_REASON_POSIX,
        { .posix = { errnum } }
    };
}

extern char *
wkWalletManagerDisconnectReasonGetMessage(WKWalletManagerDisconnectReason *reason) {
    char *message = NULL;

    switch (reason->type) {
        case WK_WALLET_MANAGER_DISCONNECT_REASON_POSIX: {
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

extern WKSyncStoppedReason
wkSyncStoppedReasonComplete(void) {
    return (WKSyncStoppedReason) {
        WK_SYNC_STOPPED_REASON_COMPLETE
    };
}

extern WKSyncStoppedReason
wkSyncStoppedReasonRequested(void) {
    return (WKSyncStoppedReason) {
        WK_SYNC_STOPPED_REASON_REQUESTED
    };
}

extern WKSyncStoppedReason
wkSyncStoppedReasonUnknown(void) {
    return (WKSyncStoppedReason) {
        WK_SYNC_STOPPED_REASON_UNKNOWN
    };
}

extern WKSyncStoppedReason
wkSyncStoppedReasonPosix(int errnum) {
    return (WKSyncStoppedReason) {
        WK_SYNC_STOPPED_REASON_POSIX,
        { .posix = { errnum } }
    };
}

extern char *
wkSyncStoppedReasonGetMessage(WKSyncStoppedReason *reason) {
    char *message = NULL;

    switch (reason->type) {
        case WK_SYNC_STOPPED_REASON_POSIX: {
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
wkSyncModeString (WKSyncMode m) {
    switch (m) {
        case WK_SYNC_MODE_API_ONLY:
        return "WK_SYNC_MODE_API_ONLY";
        case WK_SYNC_MODE_API_WITH_P2P_SEND:
        return "WK_SYNC_MODE_API_WITH_P2P_SEND";
        case WK_SYNC_MODE_P2P_WITH_API_SYNC:
        return "WK_SYNC_MODE_P2P_WITH_API_SYNC";
        case WK_SYNC_MODE_P2P_ONLY:
        return "WK_SYNC_MODE_P2P_ONLY";
    }
}

// MARK: - Periodic Dispatcher

static void
wkWalletManagerPeriodicDispatcher (BREventHandler handler,
                                       BREventTimeout *event) {
    WKWalletManager cwm = (WKWalletManager) event->context;
    wkClientSyncPeriodic (cwm->canSync);
}

// MARK: - Transaction/Transfer Bundle

private_extern void
wkWalletManagerSaveTransactionBundle (WKWalletManager manager,
                                          OwnershipKept WKClientTransactionBundle bundle) {
    if (NULL != manager->handlers->saveTransactionBundle)
        manager->handlers->saveTransactionBundle (manager, bundle);
    else if (fileServiceHasType (manager->fileService, WK_FILE_SERVICE_TYPE_TRANSACTION))
        fileServiceSave (manager->fileService, WK_FILE_SERVICE_TYPE_TRANSACTION, bundle);
}

private_extern void
wkWalletManagerSaveTransferBundle (WKWalletManager manager,
                                       OwnershipKept WKClientTransferBundle bundle) {
    if (NULL != manager->handlers->saveTransferBundle)
        manager->handlers->saveTransferBundle (manager, bundle);
    else if (fileServiceHasType (manager->fileService, WK_FILE_SERVICE_TYPE_TRANSFER))
        fileServiceSave (manager->fileService, WK_FILE_SERVICE_TYPE_TRANSFER, bundle);
}

private_extern void
wkWalletManagerRecoverTransfersFromTransactionBundle (WKWalletManager cwm,
                                                          OwnershipKept WKClientTransactionBundle bundle) {
    cwm->handlers->recoverTransfersFromTransactionBundle (cwm, bundle);
}

private_extern void
wkWalletManagerRecoverTransferFromTransferBundle (WKWalletManager cwm,
                                                      OwnershipKept WKClientTransferBundle bundle) {
    cwm->handlers->recoverTransferFromTransferBundle (cwm, bundle);
}

private_extern void
wkWalletManagerRecoverTransferAttributesFromTransferBundle (WKWallet wallet,
                                                                WKTransfer transfer,
                                                                OwnershipKept WKClientTransferBundle bundle) {
    // If we are passed in attribues, they will replace any attribute already held
    // in `genTransfer`.  Specifically, for example, if we created an XRP transfer, then
    // we might have a 'DestinationTag'.  If the attributes provided do not include
    // 'DestinatinTag' then that attribute will be lost.  Losing such an attribute would
    // indicate a BlockSet error in processing transfers.
    if (bundle->attributesCount > 0) {
        WKAddress target = wkTransferGetTargetAddress (transfer);

        // Build the transfer attributes
        BRArrayOf(WKTransferAttribute) attributes;
        array_new(attributes, bundle->attributesCount);
        for (size_t index = 0; index < bundle->attributesCount; index++) {
            // Lookup a pre-existing attribute having `key`
            WKTransferAttribute attribute =
            wkWalletGetTransferAttributeForKey (wallet,
                                                    target,
                                                    bundle->attributeKeys[index]);

            // If an attribute exists, take the bundle's value and extent `attributes`.
            if (NULL != attribute) {
                array_add (attributes,
                           wkTransferAttributeCreate (wkTransferAttributeGetKey (attribute),
                                                          bundle->attributeVals[index],
                                                          wkTransferAttributeIsRequired (attribute)));
                wkTransferAttributeGive(attribute);
            }
        }
        
        wkTransferSetAttributes (transfer, array_count(attributes), attributes);
        wkTransferAttributeArrayRelease (attributes);
        wkAddressGive (target);
    }
}

private_extern WKFeeBasis
wkWalletManagerRecoverFeeBasisFromFeeEstimate (WKWalletManager cwm,
                                               WKTransfer transfer,
                                               WKNetworkFee networkFee,
                                               double costUnits,
                                               size_t attributesCount,
                                               OwnershipKept const char **attributeKeys,
                                               OwnershipKept const char **attributeVals) {
    assert (NULL != cwm->handlers->recoverFeeBasisFromFeeEstimate); // not supported by chain
    return cwm->handlers->recoverFeeBasisFromFeeEstimate (cwm,
                                                          transfer,
                                                          networkFee,
                                                          costUnits,
                                                          attributesCount,
                                                          attributeKeys,
                                                          attributeVals);
}
