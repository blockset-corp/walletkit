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

#include "BRCryptoWalletManager.h"
#include "BRCryptoWalletManagerP.h"

#include "BRCryptoGenericP.h"
#include "support/BRFileService.h"
#include "ethereum/event/BREventAlarm.h"

// We'll do a period QRY 'tick-tock' CWM_CONFIRMATION_PERIOD_FACTOR times in
// each network's ConfirmationPeriod.  Thus, for example, the Bitcoin confirmation period is
// targeted for every 10 minutes; we'll check every 2.5 minutes.
#define CWM_CONFIRMATION_PERIOD_FACTOR  (4)

uint64_t BLOCK_HEIGHT_UNBOUND_VALUE = UINT64_MAX;

static void
cryptoWalletManagerPeriodicDispatcher (BREventHandler handler,
                                       BREventTimeout *event);

static void
cryptoWalletManagerFileServiceErrorHandler (BRFileServiceContext context,
                                            BRFileService fs,
                                            BRFileServiceError error);

#ifdef REFACTOR
static void
cryptoWalletManagerSyncCallbackGEN (BRGenericManagerSyncContext context,
                                    BRGenericManager manager,
                                    uint64_t begBlockHeight,
                                    uint64_t endBlockHeight,
                                    uint64_t fullSyncIncrement);
#endif
IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoWalletManager, cryptoWalletManager)

/// =============================================================================================
///
/// MARK: - Wallet Manager
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

#pragma clang diagnostic push
#pragma GCC diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-function"
static BRArrayOf(BRCryptoCurrency)
cryptoWalletManagerGetCurrenciesOfIntereest (BRCryptoWalletManager cwm) {
    BRArrayOf(BRCryptoCurrency) currencies;

    array_new (currencies, 3);
    return currencies;
}

static void
cryptoWalletManagerReleaseCurrenciesOfIntereest (BRCryptoWalletManager cwm,
                                                 BRArrayOf(BRCryptoCurrency) currencies) {
    for (size_t index = 0; index < array_count(currencies); index++)
        cryptoCurrencyGive (currencies[index]);
    array_free (currencies);
}
#pragma clang diagnostic pop
#pragma GCC diagnostic pop

extern BRCryptoWalletManager
cryptoWalletManagerAllocAndInit (size_t sizeInBytes,
                                 BRCryptoBlockChainType type,
                                 BRCryptoListener listener,
                                 BRCryptoClient client,
                                 BRCryptoAccount account,
                                 BRCryptoNetwork network,
                                 BRCryptoAddressScheme scheme,
                                 const char *path,
                                 BRCryptoClientQRYByType byType) {
    assert (sizeInBytes >= sizeof (struct BRCryptoWalletManagerRecord));
    assert (type == cryptoNetworkGetType(network));

    BRCryptoWalletManager cwm = calloc (1, sizeInBytes);
    if (NULL == cwm) return NULL;

    cwm->type = type;
    cwm->handlers = cryptoGenericHandlersLookup(type)->manager;
    network->sizeInBytes = sizeInBytes;

    cwm->listener = listener;
    cwm->client  = client;
    cwm->network = cryptoNetworkTake (network);
    cwm->account = cryptoAccountTake (account);
    cwm->state   = cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_CREATED);
    cwm->addressScheme = scheme;
    cwm->path = strdup (path);

    // File Service
    const char *currencyName = cryptoBlockChainTypeGetCurrencyCode (cwm->type);
    const char *networkName  = cryptoNetworkGetDesc(network);

    // TODO: Replace `createFileService` with `getFileServiceSpecifications`
    cwm->fileService = cwm->handlers->createFileService (cwm,
                                                         cwm->path,
                                                         currencyName,
                                                         networkName,
                                                         cwm,
                                                         cryptoWalletManagerFileServiceErrorHandler);

    // Create the alarm clock, but don't start it.
    alarmClockCreateIfNecessary(0);

    // Create the event handler name (useful for debugging).
    char handlerName[5 + strlen(currencyName) + 1];
    sprintf(handlerName, "Core %s", currencyName);

    // Get the event handler types.
    size_t eventTypesCount;
    const BREventType **eventTypes = cwm->handlers->getEventTypes (cwm, &eventTypesCount);

    // Create the event handler
    cwm->handler = eventHandlerCreate (handlerName,
                                       eventTypes,
                                       eventTypesCount,
                                       &cwm->lock);

    eventHandlerSetTimeoutDispatcher (cwm->handler,
                                      (1000 * cryptoNetworkGetConfirmationPeriodInSeconds(network)) / CWM_CONFIRMATION_PERIOD_FACTOR,
                                      (BREventDispatcher) cryptoWalletManagerPeriodicDispatcher,
                                      (void*) cwm);

    cwm->p2pManager = cwm->handlers->createP2PManager (cwm);
    cwm->qryManager = cryptoClientQRYManagerCreate(client, cwm, byType);

    cwm->syncMode = CRYPTO_SYNC_MODE_API_ONLY;
    cryptoWalletManagerSetMode (cwm, cwm->syncMode);

    cwm->wallet = NULL;
    array_new (cwm->wallets, 1);

    cwm->ref = CRYPTO_REF_ASSIGN (cryptoWalletManagerRelease);

    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

        pthread_mutex_init(&cwm->lock, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    return cwm;
}

extern BRCryptoWalletManager
cryptoWalletManagerCreate (BRCryptoListener listener,
                           BRCryptoClient client,
                           BRCryptoAccount account,
                           BRCryptoNetwork network,
                           BRCryptoSyncMode mode,
                           BRCryptoAddressScheme scheme,
                           const char *path) {
    // Only create a wallet manager for accounts that are initializedon network.
    if (CRYPTO_FALSE == cryptoNetworkIsAccountInitialized(network, account))
        return NULL;

    // Lookup the handler for the network's type.
    BRCryptoBlockChainType type = cryptoNetworkGetType(network);
    const BRCryptoWalletManagerHandlers *handlers = cryptoGenericHandlersLookup(type)->manager;

    // Create the manager
    BRCryptoWalletManager manager = handlers->create (listener,
                                                      client,
                                                      account,
                                                      network,
                                                      mode,
                                                      scheme,
                                                      path);
    if (NULL == manager) return NULL;

    // Initialize the manager.  This will restore persistent state - such as for transactions.
    // It will create the primary wallet and any other associated wallets (that are knowable at
    // this time).  No events will be announced.
    //
    // This will fully configure the P2P and QRY managers.
    manager->handlers->initialize (manager);

#if 0
    // Primary wallet currency and unit.
    BRCryptoCurrency currency = cryptoNetworkGetCurrency (manager->network);
    BRCryptoUnit     unit     = cryptoNetworkGetUnitAsDefault (manager->network, currency);

    // TODO: Tranactions, Block, Peers
    
    // Create all wallets for 'network'
    manager->wallets = manager->handlers->createWallets (manager, NULL, &manager->wallet);
    // Announce wallets
#endif

    cryptoWalletManagerSetMode (manager, mode);

    // Start
    cryptoWalletManagerStart (manager);

    return manager;
    
#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWalletManagerClient client = cryptoWalletManagerClientCreateBTCClient (cwm);

            // Create BWM - will also create the BWM primary wallet....
            cwm->u.btc = BRWalletManagerNew (client,
                                             cryptoAccountAsBTC (account),
                                             cryptoNetworkAsBTC (network),
                                             (uint32_t) cryptoAccountGetTimestamp(account),
                                             mode,
                                             cwmPath,
                                             cryptoNetworkGetHeight(network),
                                             cryptoNetworkGetConfirmationsUntilFinal (network));
            if (NULL == cwm->u.btc) { error = true; break ; }

            // ... get the CWM primary wallet in place...
            cwm->wallet = cryptoWalletCreateAsBTC (unit, unit, cwm->u.btc, BRWalletManagerGetWallet (cwm->u.btc));

            // ... add the CWM primary wallet to CWM
            cryptoWalletManagerAddWallet (cwm, cwm->wallet);

            // ... and finally start the BWM event handling (with CWM fully in place).
            BRWalletManagerStart (cwm->u.btc);

            break;
        }

        case BLOCK_CHAIN_TYPE_ETH: {
            BREthereumClient client = cryptoWalletManagerClientCreateETHClient (cwm);

            // Create EWM - will also create the EWM primary wallet....
            cwm->u.eth = ewmCreate (cryptoNetworkAsETH(network),
                                    cryptoAccountAsETH(account),
                                    (BREthereumTimestamp) cryptoAccountGetTimestamp(account),
                                    mode,
                                    client,
                                    cwmPath,
                                    cryptoNetworkGetHeight(network),
                                    cryptoNetworkGetConfirmationsUntilFinal (network));
            if (NULL == cwm->u.eth) { error = true; break; }

            // ... get the CWM primary wallet in place...
            cwm->wallet = cryptoWalletCreateAsETH (unit, unit, cwm->u.eth, ewmGetWallet(cwm->u.eth));

            // ... add the CWM primary wallet to CWM
            cryptoWalletManagerAddWallet (cwm, cwm->wallet);

            // ... and finally start the EWM event handling (with CWM fully in place).
            ewmStart (cwm->u.eth);

            // This will install ERC20 Tokens for the CWM Currencies.  Corresponding Wallets are
            // not created for these currencies.
            cryptoWalletManagerInstallETHTokensForCurrencies(cwm);

            // We finish here with possibly EWM events in the EWM handler queue and/or with
            // CWM events in the CWM handler queue.

            break;
        }

        case BLOCK_CHAIN_TYPE_GEN: {
#define GEN_DISPATCHER_PERIOD       (10)        // related to block proccessing

            pthread_mutex_lock (&cwm->lock);
            BRGenericClient client = cryptoWalletManagerClientCreateGENClient (cwm);

            // Create CWM as 'GEN' based on the network's base currency.
            BRCryptoNetworkCanonicalType type = cryptoNetworkGetCanonicalType (network);

            cwm->u.gen = genManagerCreate (client,
                                           type,
                                           cryptoNetworkAsGEN (network),
                                           cryptoAccountAsGEN (account, type),
                                           cryptoAccountGetTimestamp(account),
                                           cwmPath,
                                           GEN_DISPATCHER_PERIOD,
                                           cwm,
                                           cryptoWalletManagerSyncCallbackGEN,
                                           cryptoNetworkGetHeight(network));
            if (NULL == cwm->u.gen) {
                pthread_mutex_unlock (&cwm->lock);
                error = true;
                break; }

            // ... and create the primary wallet
            cwm->wallet = cryptoWalletCreateAsGEN (unit, unit, genManagerGetPrimaryWallet (cwm->u.gen));

            // ... and add the primary wallet to the wallet manager...
            cryptoWalletManagerAddWallet (cwm, cwm->wallet);

            pthread_mutex_unlock (&cwm->lock);

            // Announce the new wallet manager;
            cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                      cryptoWalletManagerTake (cwm),
                                                      (BRCryptoWalletManagerEvent) {
                                                          CRYPTO_WALLET_MANAGER_EVENT_CREATED
                                                      });

            // ... and announce the created wallet.
            cwm->listener.walletEventCallback (cwm->listener.context,
                                               cryptoWalletManagerTake (cwm),
                                               cryptoWalletTake (cwm->wallet),
                                               (BRCryptoWalletEvent) {
                                                   CRYPTO_WALLET_EVENT_CREATED
                                               });

            // ... and announce the manager's new wallet.
            cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                      cryptoWalletManagerTake (cwm),
                                                      (BRCryptoWalletManagerEvent) {
                                                          CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED,
                                                          { .wallet = { cryptoWalletTake (cwm->wallet) }}
                                                      });
            pthread_mutex_lock (&cwm->lock);

            // Load transfers from persistent storage
            BRArrayOf(BRGenericTransfer) transfers = genManagerLoadTransfers (cwm->u.gen);
            for (size_t index = 0; index < array_count (transfers); index++) {
                // TODO: A BRGenericTransfer must allow us to determine the Wallet (via a Currency).
                cryptoWalletManagerHandleTransferGEN (cwm, transfers[index]);
            }
            array_free (transfers);

            // Having added the transfers, get the wallet balance...
            BRCryptoAmount balance = cryptoWalletGetBalance (cwm->wallet);
            pthread_mutex_unlock (&cwm->lock);

            // ... and announce the balance
            cwm->listener.walletEventCallback (cwm->listener.context,
                                               cryptoWalletManagerTake (cwm),
                                               cryptoWalletTake (cwm->wallet),
                                               (BRCryptoWalletEvent) {
                                                   CRYPTO_WALLET_EVENT_BALANCE_UPDATED,
                                                   { .balanceUpdated = { balance }}
                                               });
            break;
        }
    }

    if (error) {
        cryptoWalletManagerGive (manager);
        manager = NULL;
    }

    cryptoUnitGive(unit);
    cryptoCurrencyGive(currency);

    return manager;

    #endif
}

#define _peer_log printf
static void
cryptoWalletManagerFileServiceErrorHandler (BRFileServiceContext context,
                                            BRFileService fs,
                                            BRFileServiceError error) {
    switch (error.type) {
        case FILE_SERVICE_IMPL:
            // This actually a FATAL - an unresolvable coding error.
            _peer_log ("BWM: FileService Error: IMPL: %s", error.u.impl.reason);
            break;
        case FILE_SERVICE_UNIX:
            _peer_log ("BWM: FileService Error: UNIX: %s", strerror(error.u.unx.error));
            break;
        case FILE_SERVICE_ENTITY:
            // This is likely a coding error too.
            _peer_log ("BWM: FileService Error: ENTITY (%s): %s",
                     error.u.entity.type,
                     error.u.entity.reason);
            break;
        case FILE_SERVICE_SDB:
            _peer_log ("BWM: FileService Error: SDB: (%d): %s",
                       error.u.sdb.code,
                       error.u.sdb.reason);
            break;
    }
    _peer_log ("BWM: FileService Error: FORCED SYNC%s", "");

    // BRWalletManager bwm = (BRWalletManager) context;
    // TODO(fix): What do we actually want to happen here?
    // if (NULL != bwm->peerManager)
    //     BRPeerManagerRescan (bwm->peerManager);
}
#undef _peer_log


static void
cryptoWalletManagerRelease (BRCryptoWalletManager cwm) {
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
    fileServiceRelease (cwm->fileService);

    // ... then the eventHandler
    eventHandlerDestroy (cwm->handler);

    // ... and finally individual memory allocations
    free (cwm->path);

    pthread_mutex_destroy (&cwm->lock);

    memset (cwm, 0, sizeof(*cwm));
    free (cwm);
}

extern BRCryptoNetwork
cryptoWalletManagerGetNetwork (BRCryptoWalletManager cwm) {
    return cryptoNetworkTake (cwm->network);
}

extern BRCryptoAccount
cryptoWalletManagerGetAccount (BRCryptoWalletManager cwm) {
    return cryptoAccountTake (cwm->account);
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

#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            BRWalletManagerSetMode (cwm->u.btc, mode);
            break;
        case BLOCK_CHAIN_TYPE_ETH:
            ewmUpdateMode (cwm->u.eth, mode);
            break;
        case BLOCK_CHAIN_TYPE_GEN:
            assert (CRYPTO_SYNC_MODE_API_ONLY == mode);
            break;
        default:
            assert (0);
            break;
    }
#endif

}

extern BRCryptoSyncMode
cryptoWalletManagerGetMode (BRCryptoWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    BRCryptoSyncMode mode = cwm->syncMode;
    pthread_mutex_unlock (&cwm->lock);
    return mode;
#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC:
           return BRWalletManagerGetMode (cwm->u.btc);
        case BLOCK_CHAIN_TYPE_ETH:
            return ewmGetMode (cwm->u.eth);
        case BLOCK_CHAIN_TYPE_GEN:
            return CRYPTO_SYNC_MODE_API_ONLY;
        default:
            assert (0);
            return CRYPTO_SYNC_MODE_API_ONLY;

    }
#endif
}

extern BRCryptoWalletManagerState
cryptoWalletManagerGetState (BRCryptoWalletManager cwm) {
    pthread_mutex_lock (&cwm->lock);
    BRCryptoWalletManagerState state = cwm->state;
    pthread_mutex_unlock (&cwm->lock);
    return state;
}

private_extern void
cryptoWalletManagerSetState (BRCryptoWalletManager cwm,
                             BRCryptoWalletManagerState state) {
    pthread_mutex_lock (&cwm->lock);
    cwm->state = state;
    pthread_mutex_unlock (&cwm->lock);
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
#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            BRWalletManagerSetNetworkReachable (cwm->u.btc, CRYPTO_TRUE == isNetworkReachable);
            break;
        default:
            break;
    }
#endif
}

//extern BRCryptoPeer
//cryptoWalletManagerGetPeer (BRCryptoWalletManager cwm) {
//    return (NULL == cwm->peer ? NULL : cryptoPeerTake (cwm->peer));
//}
//
//extern void
//cryptoWalletManagerSetPeer (BRCryptoWalletManager cwm,
//                            BRCryptoPeer peer) {
//    BRCryptoPeer oldPeer = cwm->peer;
//    cwm->peer = (NULL == peer ? NULL : cryptoPeerTake(peer));
//    if (NULL != oldPeer) cryptoPeerGive (oldPeer);
//}

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
        if (currency == cryptoWalletGetCurrency (cwm->wallets[index])) {
            wallet = cryptoWalletTake (cwm->wallets[index]);
            break;
        }
    }
    pthread_mutex_unlock (&cwm->lock);
    return wallet;
}

extern BRCryptoWallet
cryptoWalletManagerRegisterWallet (BRCryptoWalletManager cwm,
                                   BRCryptoCurrency currency) {
    BRCryptoWallet wallet = cryptoWalletManagerGetWalletForCurrency (cwm, currency);
    if (NULL == wallet) {
#ifdef REFACTOR
        switch (cwm->type) {
            case BLOCK_CHAIN_TYPE_BTC:
                assert (0); // Only BTC currency; has `primaryWallet
                break;

            case BLOCK_CHAIN_TYPE_ETH: {
                const char *issuer = cryptoCurrencyGetIssuer (currency);
                BREthereumAddress ethAddress = ethAddressCreate (issuer);
                BREthereumToken ethToken = ewmLookupToken (cwm->u.eth, ethAddress);
                assert (NULL != ethToken);
                ewmGetWalletHoldingToken (cwm->u.eth, ethToken);
                break;
            }
            case BLOCK_CHAIN_TYPE_GEN:
                assert (0);
                break;
        }
#endif
    }
    return wallet;
}

extern BRCryptoBoolean
cryptoWalletManagerHasWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet) {
    BRCryptoBoolean r = CRYPTO_FALSE;
    pthread_mutex_lock (&cwm->lock);
    for (size_t index = 0; index < array_count (cwm->wallets) && CRYPTO_FALSE == r; index++) {
        r = cryptoWalletEqual(cwm->wallets[index], wallet);
    }
    pthread_mutex_unlock (&cwm->lock);
    return r;
}

extern void
cryptoWalletManagerAddWallet (BRCryptoWalletManager cwm,
                              BRCryptoWallet wallet) {
    pthread_mutex_lock (&cwm->lock);
    if (CRYPTO_FALSE == cryptoWalletManagerHasWallet (cwm, wallet)) {
        array_add (cwm->wallets, cryptoWalletTake (wallet));
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
    eventHandlerStop (cwm->handler);

    // P2P Manager
    // QRY Manager

}

extern void
cryptoWalletManagerStop (BRCryptoWalletManager cwm) {
    // Stop the CWM 'Event Handler'
    eventHandlerStop (cwm->handler);

    // P2P Manager
    // QRY Manager

    #ifdef REFACTOR
    // Stop the specific cwm type, if it exists.
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            if (NULL != cwm->u.btc)
                BRWalletManagerStop (cwm->u.btc);
            break;
        case BLOCK_CHAIN_TYPE_ETH:
            if (NULL != cwm->u.eth)
                ewmStop (cwm->u.eth);
            break;
        case BLOCK_CHAIN_TYPE_GEN:
            if (NULL != cwm->u.gen)
                genManagerStop (cwm->u.gen);
            break;
    }
#endif
}

/// MARK: - Connect/Disconnect/Sync

extern void
cryptoWalletManagerConnect (BRCryptoWalletManager cwm,
                            BRCryptoPeer peer) {
    switch (cwm->state.type) {
        case CRYPTO_WALLET_MANAGER_STATE_CREATED:
        case CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED: {
            BRCryptoWalletManagerState oldState = cwm->state;
            BRCryptoWalletManagerState newState = cryptoWalletManagerStateInit (CRYPTO_WALLET_MANAGER_STATE_CONNECTED);

            cryptoClientP2PManagerConnect (cwm->p2pManager, peer);
            cryptoClientQRYManagerConnect (cwm->qryManager);

            cryptoWalletManagerSetState (cwm, newState);

            (void) oldState;
#ifdef REFACTOR
            cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                      cryptoWalletManagerTake (cwm),
                                                      (BRCryptoWalletManagerEvent) {
                CRYPTO_WALLET_MANAGER_EVENT_CHANGED,
                { .state = { oldState, newState }}
            });
#endif
            break;
        }
        case CRYPTO_WALLET_MANAGER_STATE_CONNECTED:
        case CRYPTO_WALLET_MANAGER_STATE_SYNCING:
            break;

        case CRYPTO_WALLET_MANAGER_STATE_DELETED:
            break;
    }
}

extern void
cryptoWalletManagerDisconnect (BRCryptoWalletManager cwm) {
    switch (cwm->state.type) {
        case CRYPTO_WALLET_MANAGER_STATE_CREATED:
        case CRYPTO_WALLET_MANAGER_STATE_CONNECTED:
        case CRYPTO_WALLET_MANAGER_STATE_SYNCING: {
            BRCryptoWalletManagerState oldState = cwm->state;
            BRCryptoWalletManagerState newState = cryptoWalletManagerStateDisconnectedInit (cryptoWalletManagerDisconnectReasonRequested());

            cryptoClientP2PManagerDisconnect (cwm->p2pManager);
            cryptoClientQRYManagerDisconnect (cwm->qryManager);

            cryptoWalletManagerSetState (cwm, newState);

            (void) oldState;
#ifdef REFACTOR
            cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                      cryptoWalletManagerTake (cwm),
                                                      (BRCryptoWalletManagerEvent) {
                CRYPTO_WALLET_MANAGER_EVENT_CHANGED,
                { .state = { oldState, newState }}
            });
#endif
            break;
        }

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
    if (CRYPTO_WALLET_MANAGER_STATE_CONNECTED == cwm->state.type)
        cryptoClientSync (cwm->canSync, depth, cryptoNetworkGetHeight(cwm->network));
}

// MARK: - Wipe


extern void
cryptoWalletManagerWipe (BRCryptoNetwork network,
                         const char *path) {

    const char *currencyName = cryptoBlockChainTypeGetCurrencyCode (cryptoNetworkGetType(network));
    const char *networkName  = cryptoNetworkGetDesc(network);

    fileServiceWipe (path, currencyName, networkName);
}

// MARK: - Gen Stuff

#ifdef REFACTOR
static BRCryptoTransferState
cryptoTransferStateCreateGEN (BRGenericTransferState generic,
                              BRCryptoUnit feeUnit) { // feeUnit already taken
    switch (generic.type) {
        case GENERIC_TRANSFER_STATE_CREATED:
            return cryptoTransferStateInit(CRYPTO_TRANSFER_STATE_CREATED);
        case GENERIC_TRANSFER_STATE_SIGNED:
            return cryptoTransferStateInit(CRYPTO_TRANSFER_STATE_SIGNED);
        case GENERIC_TRANSFER_STATE_SUBMITTED:
            return cryptoTransferStateInit(CRYPTO_TRANSFER_STATE_SUBMITTED);
        case GENERIC_TRANSFER_STATE_INCLUDED: {
            BRCryptoFeeBasis      basis = cryptoFeeBasisCreateAsGEN (feeUnit, generic.u.included.feeBasis);
            BRCryptoTransferState state = cryptoTransferStateIncludedInit (generic.u.included.blockNumber,
                                                                           generic.u.included.transactionIndex,
                                                                           generic.u.included.timestamp,
                                                                           basis,
                                                                           generic.u.included.success,
                                                                           generic.u.included.error);
            cryptoFeeBasisGive (basis);
            return state;
        }
        case GENERIC_TRANSFER_STATE_ERRORED:
            return cryptoTransferStateErroredInit (cryptoTransferSubmitErrorUnknown());
        case GENERIC_TRANSFER_STATE_DELETED:
            return cryptoTransferStateInit(CRYPTO_TRANSFER_STATE_SIGNED);
    }
}

private_extern void
cryptoWalletManagerSetTransferStateGEN (BRCryptoWalletManager cwm,
                                        BRCryptoWallet wallet,
                                        BRCryptoTransfer transfer,
                                        BRGenericTransferState newGenericState) {
    pthread_mutex_lock (&cwm->lock);

    BRGenericTransfer      genericTransfer = cryptoTransferAsGEN (transfer);
    BRGenericTransferState oldGenericState = genTransferGetState (genericTransfer);

    if (!genTransferStateEqual (oldGenericState, newGenericState)) {
        BRCryptoTransferState oldState = cryptoTransferGetState (transfer);
        BRCryptoTransferState newState = cryptoTransferStateCreateGEN (newGenericState,
                                                                       cryptoTransferGetUnitForFee(transfer));

        pthread_mutex_unlock (&cwm->lock);
        cwm->listener.transferEventCallback (cwm->listener.context,
                                             cryptoWalletManagerTake (cwm),
                                             cryptoWalletTake (wallet),
                                             cryptoTransferTake(transfer),
                                             (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CHANGED,
            { .state = {
                cryptoTransferStateCopy (&oldState),
                cryptoTransferStateCopy (&newState) }}
        });
        pthread_mutex_lock (&cwm->lock);

        genTransferSetState (genericTransfer, newGenericState);
        cryptoTransferSetState (transfer, newState);

        cryptoTransferStateRelease (&oldState);
        cryptoTransferStateRelease (&newState);
    }

    //
    // If this is an error case, then we must remove the genericTransfer from the
    // genericWallet; otherwise the GEN balance and sequence number will be off.
    //
    // However, we leave the `transfer` in `wallet`.  And trouble is forecasted...
    //
    if (GENERIC_TRANSFER_STATE_ERRORED == newGenericState.type) {
        genWalletRemTransfer(cryptoWalletAsGEN(wallet), genericTransfer);

        BRCryptoAmount balance = cryptoWalletGetBalance(wallet);
        cwm->listener.walletEventCallback (cwm->listener.context,
                                           cryptoWalletManagerTake (cwm),
                                           cryptoWalletTake (cwm->wallet),
                                           (BRCryptoWalletEvent) {
                                               CRYPTO_WALLET_EVENT_BALANCE_UPDATED,
                                               { .balanceUpdated = { balance }}
                                           });

        cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                  cryptoWalletManagerTake (cwm),
                                                  (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED,
            { .wallet = cryptoWalletTake (cwm->wallet) }
        });
    }

    pthread_mutex_unlock (&cwm->lock);
}
#endif

extern BRCryptoTransfer
cryptoWalletManagerCreateTransferMultiple (BRCryptoWalletManager cwm,
                                           BRCryptoWallet wallet,
                                           size_t outputsCount,
                                           BRCryptoTransferOutput *outputs,
                                           BRCryptoFeeBasis estimatedFeeBasis) {
    BRCryptoTransfer transfer = cryptoWalletCreateTransferMultiple (wallet, outputsCount, outputs, estimatedFeeBasis);

#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            break;
        case BLOCK_CHAIN_TYPE_ETH:
            break;
        case BLOCK_CHAIN_TYPE_GEN:
            if (NULL != transfer) {
                cwm->listener.transferEventCallback (cwm->listener.context,
                                                     cryptoWalletManagerTake (cwm),
                                                     cryptoWalletTake (wallet),
                                                     cryptoTransferTake(transfer),
                                                     (BRCryptoTransferEvent) {
                    CRYPTO_TRANSFER_EVENT_CREATED
                });
            }
            break;
    }
#endif

    return transfer;
}

extern BRCryptoTransfer
cryptoWalletManagerCreateTransfer (BRCryptoWalletManager cwm,
                                   BRCryptoWallet wallet,
                                   BRCryptoAddress target,
                                   BRCryptoAmount amount,
                                   BRCryptoFeeBasis estimatedFeeBasis,
                                   size_t attributesCount,
                                   OwnershipKept BRCryptoTransferAttribute *attributes) {
    BRCryptoTransfer transfer = cryptoWalletCreateTransfer (wallet, target, amount,
                                                            estimatedFeeBasis,
                                                            attributesCount,
                                                            attributes);

#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC:
            break;
        case BLOCK_CHAIN_TYPE_ETH:
            break;
        case BLOCK_CHAIN_TYPE_GEN:
            if (NULL != transfer) {
                cwm->listener.transferEventCallback (cwm->listener.context,
                                                     cryptoWalletManagerTake (cwm),
                                                     cryptoWalletTake (wallet),
                                                     cryptoTransferTake(transfer),
                                                     (BRCryptoTransferEvent) {
                    CRYPTO_TRANSFER_EVENT_CREATED
                });
            }
            break;
    }
#endif
    return transfer;
}

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
    // TODO: Transfer Signed Event

#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            success = AS_CRYPTO_BOOLEAN (BRWalletManagerSignTransaction (cwm->u.btc,
                                                                         cryptoWalletAsBTC (wallet),
                                                                         cryptoTransferAsBTC(transfer),
                                                                         &seed,
                                                                         sizeof (seed)));
            break;
        }

        case BLOCK_CHAIN_TYPE_ETH:
            // TODO(fix): ewmWalletSignTransferWithPaperKey() doesn't return a status
            break;

        case BLOCK_CHAIN_TYPE_GEN:
            success = AS_CRYPTO_BOOLEAN (genManagerSignTransfer (cwm->u.gen,
                                                                 cryptoWalletAsGEN (wallet),
                                                                 cryptoTransferAsGEN (transfer),
                                                                 seed));
            if (CRYPTO_TRUE == success)
                cryptoWalletManagerSetTransferStateGEN (cwm, wallet, transfer,
                                                        genTransferStateCreateOther (GENERIC_TRANSFER_STATE_SIGNED));
            break;
    }
#endif
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
    // TODO: Transfer Signed Event

    return success;
}

extern void
cryptoWalletManagerSubmit (BRCryptoWalletManager manager,
                           BRCryptoWallet wallet,
                           BRCryptoTransfer transfer,
                           const char *paperKey) {

    if (CRYPTO_TRUE == cryptoWalletManagerSign (manager, wallet, transfer, paperKey))
        cryptoWalletManagerSubmitSigned (manager, wallet, transfer);

#ifdef REFACTOR
    // Derive the seed used for signing
    UInt512 seed = cryptoAccountDeriveSeed(paperKey);

    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {

            if (BRWalletManagerSignTransaction (cwm->u.btc,
                                                cryptoWalletAsBTC (wallet),
                                                cryptoTransferAsBTC(transfer),
                                                &seed,
                                                sizeof (seed))) {
                cryptoWalletManagerSubmitSigned (cwm, wallet, transfer);
            }
            break;
        }

        case BLOCK_CHAIN_TYPE_ETH: {
            ewmWalletSignTransferWithPaperKey (cwm->u.eth,
                                               cryptoWalletAsETH (wallet),
                                               cryptoTransferAsETH (transfer),
                                               paperKey);

            cryptoWalletManagerSubmitSigned (cwm, wallet, transfer);
            break;
        }

        case BLOCK_CHAIN_TYPE_GEN: {
            BRGenericWallet genWallet = cryptoWalletAsGEN (wallet);
            BRGenericTransfer genTransfer = cryptoTransferAsGEN (transfer);

            // Sign the transfer
            if (genManagerSignTransfer (cwm->u.gen, genWallet, genTransfer, seed)) {
                cryptoWalletManagerSetTransferStateGEN (cwm, wallet, transfer,
                                                        genTransferStateCreateOther (GENERIC_TRANSFER_STATE_SIGNED));
                // Submit the transfer
                cryptoWalletManagerSubmitSigned (cwm, wallet, transfer);
            }
            break;
        }
    }
    // Zero-out the seed.
    seed = UINT512_ZERO;

    return;
    #endif
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

#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            if (BRWalletManagerSignTransactionForKey (cwm->u.btc,
                                                      cryptoWalletAsBTC (wallet),
                                                      cryptoTransferAsBTC(transfer),
                                                      cryptoKeyGetCore (key))) {
                cryptoWalletManagerSubmitSigned (cwm, wallet, transfer);
            }
            break;
        }

        case BLOCK_CHAIN_TYPE_ETH: {
            ewmWalletSignTransfer (cwm->u.eth,
                                   cryptoWalletAsETH (wallet),
                                   cryptoTransferAsETH (transfer),
                                   *cryptoKeyGetCore (key));

            cryptoWalletManagerSubmitSigned (cwm, wallet, transfer);
            break;
        }

        case BLOCK_CHAIN_TYPE_GEN: {
            BRGenericWallet genWallet = cryptoWalletAsGEN (wallet);
            BRGenericTransfer genTransfer = cryptoTransferAsGEN (transfer);

            if (genManagerSignTransferWithKey (cwm->u.gen, genWallet, genTransfer, cryptoKeyGetCore (key))) {
                cryptoWalletManagerSetTransferStateGEN (cwm, wallet, transfer,
                                                        genTransferStateCreateOther (GENERIC_TRANSFER_STATE_SIGNED));
                cryptoWalletManagerSubmitSigned (cwm, wallet, transfer);
            }
            break;
        }
    }
#endif
}

extern void
cryptoWalletManagerSubmitSigned (BRCryptoWalletManager cwm,
                                 BRCryptoWallet wallet,
                                 BRCryptoTransfer transfer) {
    cryptoClientSend (cwm->canSend, transfer);
    // TODO: Transfer Submit Event

#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWalletManagerSubmitTransaction (cwm->u.btc,
                                              cryptoWalletAsBTC (wallet),
                                              cryptoTransferAsBTC(transfer));
            break;
        }

        case BLOCK_CHAIN_TYPE_ETH: {
            ewmWalletSubmitTransfer (cwm->u.eth,
                                     cryptoWalletAsETH (wallet),
                                     cryptoTransferAsETH (transfer));
            break;
        }

        case BLOCK_CHAIN_TYPE_GEN: {
            // We don't have GEN Events bubbling up that we can handle by adding transfer
            // to wallet.  So, we'll add transfer here...
            cryptoWalletAddTransfer (wallet, transfer);

            // Add the signed/submitted transfer to the GEN wallet.  If the submission fails
            // we'll remove it then.  For now, include transfer when computing the balance and
            // the sequence-number 
            genWalletAddTransfer (cryptoWalletAsGEN(wallet), cryptoTransferAsGEN(transfer));

            // ... and announce the wallet's newly added transfer
            cwm->listener.walletEventCallback (cwm->listener.context,
                                               cryptoWalletManagerTake (cwm),
                                               cryptoWalletTake (wallet),
                                               (BRCryptoWalletEvent) {
                CRYPTO_WALLET_EVENT_TRANSFER_ADDED,
                { .transfer = { cryptoTransferTake (transfer) }}
            });

            // ... perform the actual submit
            genManagerSubmitTransfer (cwm->u.gen,
                                      cryptoWalletAsGEN (wallet),
                                      cryptoTransferAsGEN (transfer));

            // ... and then announce the submission.
            cwm->listener.walletEventCallback (cwm->listener.context,
                                               cryptoWalletManagerTake (cwm),
                                               cryptoWalletTake (wallet),
                                               (BRCryptoWalletEvent) {
                CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED,
                { .transfer = { cryptoTransferTake (transfer) }}
            });

            break;
        }
    }
#endif
}

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

#ifdef REFACTOR
    switch (wallet->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWallet *wid = wallet->u.btc.wid;

            // Amount may be zero if insufficient fees
            *isZeroIfInsuffientFunds = CRYPTO_TRUE;

            // NOTE: We know BTC/BCH has a minimum balance of zero.

            uint64_t balance     = BRWalletBalance (wid);
            uint64_t feePerKB    = 1000 * cryptoNetworkFeeAsBTC (fee);
            uint64_t amountInSAT = (CRYPTO_FALSE == asMaximum
                                    ? BRWalletMinOutputAmountWithFeePerKb (wid, feePerKB)
                                    : BRWalletMaxOutputAmountWithFeePerKb (wid, feePerKB));
            uint64_t fee         = (amountInSAT > 0
                                    ? BRWalletFeeForTxAmountWithFeePerKb (wid, feePerKB, amountInSAT)
                                    : 0);

//            if (CRYPTO_TRUE == asMaximum)
//                assert (balance == amountInSAT + fee);

            if (amountInSAT + fee > balance)
                amountInSAT = 0;

            amount = uint256Create(amountInSAT);
            break;
        }

        case BLOCK_CHAIN_TYPE_ETH: {
            BREthereumEWM ewm = wallet->u.eth.ewm;
            BREthereumWallet wid = wallet->u.eth.wid;

            // We always need an estimate as we do not know the fees.
            *needEstimate = CRYPTO_TRUE;

            if (CRYPTO_FALSE == asMaximum)
                amount = uint256Create(0);
            else {
                BREthereumAmount ethAmount = ewmWalletGetBalance (ewm, wid);

                // NOTE: We know ETH has a minimum balance of zero.

                amount = (AMOUNT_ETHER == ethAmountGetType(ethAmount)
                          ? ethAmountGetEther(ethAmount).valueInWEI
                          : ethAmountGetTokenQuantity(ethAmount).valueAsInteger);
            }
            break;
        }

        case BLOCK_CHAIN_TYPE_GEN: {
            // TODO: Probably, unfortunately, the need for an estimate is likely currency dependent.
            *needEstimate = CRYPTO_FALSE;

            if (CRYPTO_FALSE == asMaximum)
                amount = uint256Create(0);
            else {
                int negative = 0, overflow = 0;

                // Get the balance
                UInt256 balance = genWalletGetBalance (wallet->u.gen);

                // We are looking for the maximum amount; check if the wallet has a minimum
                // balance.  If so, reduce the above balance.
                BRCryptoBoolean hasMinimum = CRYPTO_FALSE;
                UInt256 balanceMinimum = genWalletGetBalanceLimit (wallet->u.gen, CRYPTO_FALSE, &hasMinimum);

                if (CRYPTO_TRUE == hasMinimum) {
                    balance = uint256Sub_Negative(balance, balanceMinimum, &negative);
                    if (negative) balance = UINT256_ZERO;
                }

                // Get the pricePerCostFactor for the (network) fee.
                BRCryptoAmount pricePerCostFactor = cryptoNetworkFeeGetPricePerCostFactor (fee);
                
                // Get a feeBasis using some sketchy defaults
                BRGenericAddress address   = genWalletGetAddress (wallet->u.gen);
                BRGenericFeeBasis feeBasis = genWalletEstimateTransferFee (wallet->u.gen,
                                                                           address,
                                                                           balance,
                                                                           cryptoAmountGetValue(pricePerCostFactor));

                // Finally, compute the fee.
                UInt256 fee = genFeeBasisGetFee (&feeBasis, &overflow);
                assert (!overflow);

                amount = uint256Sub_Negative (balance, fee, &negative);
                if (negative) amount = UINT256_ZERO;

                genAddressRelease(address);
                cryptoAmountGive(pricePerCostFactor);

            }
            break;
        }
    }

    return cryptoAmountCreateInternal (unit,
                                       CRYPTO_FALSE,
                                       amount,
                                       0);
#endif
}


extern void
cryptoWalletManagerEstimateFeeBasis (BRCryptoWalletManager manager,
                                     BRCryptoWallet  wallet,
                                     BRCryptoCookie cookie,
                                     BRCryptoAddress target,
                                     BRCryptoAmount  amount,
                                     BRCryptoNetworkFee fee) {
    manager->handlers->estimateFeeBasis (manager,
                                         wallet,
                                         cookie,
                                         target,
                                         amount,
                                         fee);
#ifdef REFACTOR
    //    assert (cryptoWalletGetType (wallet) == cryptoFeeBasisGetType(feeBasis));
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWalletManager bwm = cwm->u.btc;
            BRWallet *wid = cryptoWalletAsBTC(wallet);

            BRCryptoBoolean overflow = CRYPTO_FALSE;
            uint64_t feePerKB        = 1000 * cryptoNetworkFeeAsBTC (fee);
            uint64_t btcAmount       = cryptoAmountGetIntegerRaw (amount, &overflow);
            assert(CRYPTO_FALSE == overflow);

            BRWalletManagerEstimateFeeForTransfer (bwm,
                                                   wid,
                                                   cookie,
                                                   btcAmount,
                                                   feePerKB);
            break;
        }

        case BLOCK_CHAIN_TYPE_ETH: {
            BREthereumEWM ewm = cwm->u.eth;
            BREthereumWallet wid = cryptoWalletAsETH(wallet);

            BRCryptoAddress source = cryptoWalletGetAddress (wallet, CRYPTO_ADDRESS_SCHEME_ETH_DEFAULT);
            UInt256 ethValue       = cryptoAmountGetValue (amount);

            BREthereumToken  ethToken  = ewmWalletGetToken (ewm, wid);
            BREthereumAmount ethAmount = (NULL != ethToken
                                          ? ethAmountCreateToken (ethTokenQuantityCreate (ethToken, ethValue))
                                          : ethAmountCreateEther (ethEtherCreate (ethValue)));

            ewmWalletEstimateTransferFeeForTransfer (ewm,
                                                     wid,
                                                     cookie,
                                                     cryptoAddressAsETH (source),
                                                     cryptoAddressAsETH (target),
                                                     ethAmount,
                                                     cryptoNetworkFeeAsETH (fee),
                                                     ewmWalletGetDefaultGasLimit (ewm, wid));

            cryptoAddressGive (source);
            break;
        }

        case BLOCK_CHAIN_TYPE_GEN: {
            BRGenericWallet  genWallet  = cryptoWalletAsGEN (wallet);
            BRGenericAddress genAddress = cryptoAddressAsGEN (target);

            UInt256 genValue = cryptoAmountGetValue(amount);
            UInt256 genPricePerCostFactor = uint256Create (cryptoNetworkFeeAsGEN(fee));

            BRGenericFeeBasis genFeeBasis = genWalletEstimateTransferFee (genWallet,
                                                                          genAddress,
                                                                          genValue,
                                                                          genPricePerCostFactor);

            BRCryptoUnit unitForFee = cryptoWalletGetUnitForFee (wallet);
            BRCryptoFeeBasis feeBasis = cryptoFeeBasisCreateAsGEN (unitForFee, genFeeBasis);
            
            cwm->listener.walletEventCallback (cwm->listener.context,
                                               cryptoWalletManagerTake (cwm),
                                               cryptoWalletTake (wallet),
                                               (BRCryptoWalletEvent) {
                CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED,
                { .feeBasisEstimated = {
                    CRYPTO_SUCCESS,
                    cookie,
                    cryptoFeeBasisTake (feeBasis)
                }}
            });

            cryptoFeeBasisGive (feeBasis);
            cryptoUnitGive(unitForFee);
        }
    }
#endif
}

extern void
cryptoWalletManagerEstimateFeeBasisForWalletSweep (BRCryptoWalletManager cwm,
                                                   BRCryptoWallet wallet,
                                                   BRCryptoCookie cookie,
                                                   BRCryptoWalletSweeper sweeper,
                                                   BRCryptoNetworkFee fee) {
#ifdef REFACTOR
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWalletManager bwm = cwm->u.btc;
            BRWallet *wid = cryptoWalletAsBTC (wallet);
            uint64_t feePerKB = 1000 * cryptoNetworkFeeAsBTC (fee);

            BRWalletManagerEstimateFeeForSweep (bwm,
                                                wid,
                                                cookie,
                                                cryptoWalletSweeperAsBTC(sweeper),
                                                feePerKB);
            break;
        }
        default:
            assert (0);
            break;
    }
#endif
}

#ifdef REFACTOR
extern void
cryptoWalletManagerEstimateFeeBasisForPaymentProtocolRequest (BRCryptoWalletManager cwm,
                                                              BRCryptoWallet wallet,
                                                              BRCryptoCookie cookie,
                                                              BRCryptoPaymentProtocolRequest request,
                                                              BRCryptoNetworkFee fee) {
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWalletManager bwm = cwm->u.btc;
            BRWallet *wid = cryptoWalletAsBTC (wallet);
            uint64_t feePerKB = 1000 * cryptoNetworkFeeAsBTC (fee);

            switch (cryptoPaymentProtocolRequestGetType (request)) {
                case CRYPTO_PAYMENT_PROTOCOL_TYPE_BITPAY:
                case CRYPTO_PAYMENT_PROTOCOL_TYPE_BIP70: {
                    BRArrayOf(BRTxOutput) outputs = cryptoPaymentProtocolRequestGetOutputsAsBTC (request);
                    if (NULL != outputs) {
                        BRWalletManagerEstimateFeeForOutputs (bwm, wid, cookie, outputs, array_count (outputs),
                                                              feePerKB);
                        array_free (outputs);
                    }
                    break;
                }
                default: {
                    assert (0);
                    break;
                }
            }
            break;
        }
        default:
            assert (0);
            break;
    }
}
#endif

#ifdef REFACTOR
extern void
cryptoWalletManagerHandleTransferGEN (BRCryptoWalletManager cwm,
                                      OwnershipGiven BRGenericTransfer transferGeneric) {
    int transferWasCreated = 0;

    // TODO: Determine the currency from `transferGeneric`
    BRCryptoCurrency currency   = cryptoNetworkGetCurrency   (cwm->network);
    BRCryptoUnit     unit       = cryptoNetworkGetUnitAsBase (cwm->network, currency);
    BRCryptoUnit     unitForFee = cryptoNetworkGetUnitAsBase (cwm->network, currency);
    BRCryptoWallet   wallet     = cryptoWalletManagerGetWalletForCurrency (cwm, currency);

    // TODO: I don't think any overall locks are needed here...

    // Look for a known transfer
    BRCryptoTransfer transfer = cryptoWalletFindTransferAsGEN (wallet, transferGeneric);

    // If we don't know about `transferGeneric`, create a crypto transfer
    if (NULL == transfer) {
        // Create the generic transfer... `transferGeneric` owned by `transfer`
        transfer = cryptoTransferCreateAsGEN (unit, unitForFee, transferGeneric);

        transferWasCreated = 1;
    }

    // We know 'transfer'; ensure it is up to date.  This is important for the case where
    // we created the transfer and then submitted it.  In that case `transfer` is what we
    // created and `transferGeneric` is what we recovered.  The recovered transfer will have
    // additional information - notably the UIDS.
    else {
        BRGenericTransfer transferGenericOrig = cryptoTransferAsGEN (transfer);

        // Update the UIDS
        if (NULL == genTransferGetUIDS(transferGenericOrig))
            genTransferSetUIDS (transferGenericOrig,
                                genTransferGetUIDS (transferGeneric));
    }

    // Fill in any attributes
    BRArrayOf(BRGenericTransferAttribute) genAttributes = genTransferGetAttributes(transferGeneric);
    BRArrayOf(BRCryptoTransferAttribute)  attributes;
    array_new(attributes, array_count(genAttributes));
    for (size_t index = 0; index < array_count(genAttributes); index++) {
        array_add (attributes,
                   cryptoTransferAttributeCreate (genTransferAttributeGetKey(genAttributes[index]),
                                                  genTransferAttributeGetVal(genAttributes[index]),
                                                  AS_CRYPTO_BOOLEAN (genTransferAttributeIsRequired(genAttributes[index]))));
    }
    cryptoTransferSetAttributes (transfer, attributes);
    array_free_all (attributes, cryptoTransferAttributeGive);

    // Set the state from `transferGeneric`.  This is where we move from 'submitted' to 'included'
    BRCryptoTransferState oldState = cryptoTransferGetState (transfer);
    BRCryptoTransferState newState = cryptoTransferStateCreateGEN (genTransferGetState(transferGeneric), unitForFee);
    cryptoTransferSetState (transfer, newState);

    if (!transferWasCreated)
        genTransferRelease(transferGeneric);

    // Save the transfer as it is now fully updated.
    genManagerSaveTransfer (cwm->u.gen, cryptoTransferAsGEN(transfer));

    // If we created the transfer...
    if (transferWasCreated) {
        // ... announce the newly created transfer.
        cwm->listener.transferEventCallback (cwm->listener.context,
                                             cryptoWalletManagerTake (cwm),
                                             cryptoWalletTake (wallet),
                                             cryptoTransferTake(transfer),
                                             (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CREATED
        });

        // ... add the transfer to its wallet...
        cryptoWalletAddTransfer (wallet, transfer);

        // ... tell 'generic wallet' about it.
        genWalletAddTransfer (cryptoWalletAsGEN(wallet), cryptoTransferAsGEN(transfer));

        // ... and announce the wallet's newly added transfer
        cwm->listener.walletEventCallback (cwm->listener.context,
                                           cryptoWalletManagerTake (cwm),
                                           cryptoWalletTake (wallet),
                                           (BRCryptoWalletEvent) {
            CRYPTO_WALLET_EVENT_TRANSFER_ADDED,
            { .transfer = { cryptoTransferTake (transfer) }}
        });

        BRCryptoAmount balance = cryptoWalletGetBalance(wallet);
        cwm->listener.walletEventCallback (cwm->listener.context,
                                           cryptoWalletManagerTake (cwm),
                                           cryptoWalletTake (cwm->wallet),
                                           (BRCryptoWalletEvent) {
                                               CRYPTO_WALLET_EVENT_BALANCE_UPDATED,
                                               { .balanceUpdated = { balance }}
                                           });

        cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                  cryptoWalletManagerTake (cwm),
                                                  (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED,
            { .wallet = cryptoWalletTake (cwm->wallet) }
        });
    }

    // If the state is not created and changed, announce a transfer state change.
    if (CRYPTO_TRANSFER_STATE_CREATED != newState.type && oldState.type != newState.type) {
        cwm->listener.transferEventCallback (cwm->listener.context,
                                             cryptoWalletManagerTake (cwm),
                                             cryptoWalletTake (wallet),
                                             cryptoTransferTake(transfer),
                                             (BRCryptoTransferEvent) {
            CRYPTO_TRANSFER_EVENT_CHANGED,
            { .state = {
                cryptoTransferStateCopy (&oldState),
                cryptoTransferStateCopy (&newState) }}
        });
    }

    cryptoTransferStateRelease (&oldState);
    cryptoTransferStateRelease (&newState);
    cryptoUnitGive(unitForFee);
    cryptoUnitGive(unit);
    cryptoTransferGive(transfer);
    cryptoWalletGive (wallet);
    cryptoCurrencyGive(currency);
}

static void
cryptoWalletManagerSyncCallbackGEN (BRGenericManagerSyncContext context,
                                    BRGenericManager manager,
                                    uint64_t begBlockHeight,
                                    uint64_t endBlockHeight,
                                    uint64_t fullSyncIncrement) {
    BRCryptoWalletManager cwm = cryptoWalletManagerTakeWeak ((BRCryptoWalletManager) context);
    if (NULL == cwm) return;

    // If the sync block range is larger than fullSyncIncrement, then this is a full sync.
    // Otherwise this is an ongoing, periodic sync - which we do not report.  It is as if in
    // P2P mode, a new block is announced.
    int fullSync = (endBlockHeight - begBlockHeight > fullSyncIncrement);

    pthread_mutex_lock (&cwm->lock);

    // If an ongoing sync, we are simply CONNECTED.
    BRCryptoWalletManagerState oldState = cwm->state;
    BRCryptoWalletManagerState newState = cryptoWalletManagerStateInit (fullSync
                                                                        ? CRYPTO_WALLET_MANAGER_STATE_SYNCING
                                                                        : CRYPTO_WALLET_MANAGER_STATE_CONNECTED);

    // Callback a Wallet Manager Event, but only on state changes.  We won't announce incremental
    // progress (with a blockHeight and timestamp.
    if (newState.type != oldState.type) {

        // Update the CWM state before any event callbacks.
        cryptoWalletManagerSetState (cwm, newState);

        pthread_mutex_unlock (&cwm->lock);

        if (fullSync) {
            // Generate a SYNC_STARTED...
            cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                      cryptoWalletManagerTake (cwm),
                                                      (BRCryptoWalletManagerEvent) {
                CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED
            });

            // ... and then a SYNC_CONTINUES at %100
            //            cwm->listener.walletManagerEventCallback (cwm->listener.context,
            //                                                      cryptoWalletManagerTake (cwm),
            //                                                      (BRCryptoWalletManagerEvent) {
            //                CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
            //                { .syncContinues = { NO_CRYPTO_SYNC_TIMESTAMP, 0 }}
            //            });
        }
        else {
            // Generate a SYNC_CONTINUES at %100...
            //            cwm->listener.walletManagerEventCallback (cwm->listener.context,
            //                                                      cryptoWalletManagerTake (cwm),
            //                                                      (BRCryptoWalletManagerEvent) {
            //                CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES,
            //                { .syncContinues = { NO_CRYPTO_SYNC_TIMESTAMP, 100 }}
            //            });

            // ... and then a CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED
            cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                      cryptoWalletManagerTake (cwm),
                                                      (BRCryptoWalletManagerEvent) {
                CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED,
                { .syncStopped = { CRYPTO_SYNC_STOPPED_REASON_COMPLETE }}
            });
        }

        cwm->listener.walletManagerEventCallback (cwm->listener.context,
                                                  cryptoWalletManagerTake (cwm),
                                                  (BRCryptoWalletManagerEvent) {
            CRYPTO_WALLET_MANAGER_EVENT_CHANGED,
            { .state = { oldState, newState }}
        });
    }
    else pthread_mutex_unlock (&cwm->lock);

    cryptoWalletManagerGive (cwm);
}
#endif

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

/// MARK: Wallet Migrator

#ifdef REFACTOR
struct BRCryptoWalletMigratorRecord {
    BRFileService fileService;
    const char *fileServiceTransactionType;
    const char *fileServiceBlockType;
    const char *fileServicePeerType;

    int theErrorHackHappened;
    BRFileServiceError theErrorHack;
};

static void theErrorHackReset (BRCryptoWalletMigrator migrator) {
    migrator->theErrorHackHappened = 0;
}

static void
cryptoWalletMigratorErrorHandler (BRFileServiceContext context,
                                  BRFileService fs,
                                  BRFileServiceError error) {
    // TODO: Racy on 'cryptoWalletMigratorRelease'?
    BRCryptoWalletMigrator migrator = (BRCryptoWalletMigrator) context;

    migrator->theErrorHackHappened = 1;
    migrator->theErrorHack = error;
}

extern BRCryptoWalletMigrator
cryptoWalletMigratorCreate (BRCryptoNetwork network,
                            const char *storagePath) {
    BRCryptoWalletMigrator migrator = calloc (1, sizeof (struct BRCryptoWalletMigratorRecord));

    migrator->fileService = BRWalletManagerCreateFileService (cryptoNetworkAsBTC(network),
                                                              storagePath,
                                                              migrator,
                                                              cryptoWalletMigratorErrorHandler);
    if (NULL == migrator->fileService) {
        cryptoWalletMigratorRelease(migrator);
        return NULL;
    }

    BRWalletManagerExtractFileServiceTypes (migrator->fileService,
                                            &migrator->fileServiceTransactionType,
                                            &migrator->fileServiceBlockType,
                                            &migrator->fileServicePeerType);

    return migrator;
}

extern void
cryptoWalletMigratorRelease (BRCryptoWalletMigrator migrator) {
    if (NULL != migrator->fileService) fileServiceRelease(migrator->fileService);

    memset (migrator, 0, sizeof(*migrator));
    free (migrator);
}

extern BRCryptoWalletMigratorStatus
cryptoWalletMigratorHandleTransactionAsBTC (BRCryptoWalletMigrator migrator,
                                            const uint8_t *bytes,
                                            size_t bytesCount,
                                            uint32_t blockHeight,
                                            uint32_t timestamp) {
    BRTransaction *tx = BRTransactionParse(bytes, bytesCount);
    if (NULL == tx)
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_ERROR_TRANSACTION
        };

    tx->blockHeight = blockHeight;
    tx->timestamp   = timestamp;

    // Calls cryptoWalletMigratorErrorHandler on error.
    theErrorHackReset(migrator);
    fileServiceSave (migrator->fileService, migrator->fileServiceTransactionType, tx);
    BRTransactionFree(tx);

    if (migrator->theErrorHackHappened)
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_ERROR_TRANSACTION
        };
    else
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_SUCCESS
        };
}

extern BRCryptoWalletMigratorStatus
cryptoWalletMigratorHandleBlockAsBTC (BRCryptoWalletMigrator migrator,
                                      BRCryptoData32 hash,
                                      uint32_t height,
                                      uint32_t nonce,
                                      uint32_t target,
                                      uint32_t txCount,
                                      uint32_t version,
                                      uint32_t timestamp,
                                      uint8_t *flags,  size_t flagsLen,
                                      BRCryptoData32 *hashes, size_t hashesCount,
                                      BRCryptoData32 merkleRoot,
                                      BRCryptoData32 prevBlock) {
    BRMerkleBlock *block = BRMerkleBlockNew();

    memcpy (block->blockHash.u8, hash.data, sizeof (hash.data));
    block->height = height;
    block->nonce  = nonce;
    block->target = target;
    block->totalTx = txCount;
    block->version = version;
    if (0 != timestamp) block->timestamp = timestamp;

    BRMerkleBlockSetTxHashes (block, (UInt256*) hashes, hashesCount, flags, flagsLen);

    memcpy (block->merkleRoot.u8, merkleRoot.data, sizeof (merkleRoot.data));
    memcpy (block->prevBlock.u8,  prevBlock.data,  sizeof (prevBlock.data));

    // ...
    theErrorHackReset(migrator);
    fileServiceSave (migrator->fileService, migrator->fileServiceBlockType, block);
    BRMerkleBlockFree (block);

    if (migrator->theErrorHackHappened)
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_ERROR_BLOCK
        };
    else
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_SUCCESS
        };
}

extern BRCryptoWalletMigratorStatus
cryptoWalletMigratorHandleBlockBytesAsBTC (BRCryptoWalletMigrator migrator,
                                           const uint8_t *bytes,
                                           size_t bytesCount,
                                           uint32_t height) {
    BRMerkleBlock *block = BRMerkleBlockParse (bytes, bytesCount);
    if (NULL == block)
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_ERROR_BLOCK
        };

    block->height = height;

    // ...
    theErrorHackReset(migrator);
    fileServiceSave (migrator->fileService, migrator->fileServiceBlockType, block);
    BRMerkleBlockFree (block);

    if (migrator->theErrorHackHappened)
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_ERROR_BLOCK
        };
    else
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_SUCCESS
        };
}

extern BRCryptoWalletMigratorStatus
cryptoWalletMigratorHandlePeerAsBTC (BRCryptoWalletMigrator migrator,
                                     uint32_t address,
                                     uint16_t port,
                                     uint64_t services,
                                     uint32_t timestamp) {
    BRPeer peer;

    peer.address = (UInt128) { .u32 = { 0, 0, 0xffff, address }};
    peer.port = port;
    peer.services = services;
    peer.timestamp = timestamp;
    peer.flags = 0;

    theErrorHackReset(migrator);
    fileServiceSave (migrator->fileService, migrator->fileServicePeerType, &peer);

    if (migrator->theErrorHackHappened)
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_ERROR_PEER
        };
    else
        return (BRCryptoWalletMigratorStatus) {
            CRYPTO_WALLET_MIGRATOR_SUCCESS
        };
}
#endif
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

private_extern OwnershipGiven BRArrayOf(BRCryptoTransfer)
cryptoWalletManagerRecoverTransfersFromTransactionBundle (BRCryptoWalletManager cwm,
                                                          OwnershipKept BRCryptoClientTransactionBundle bundle) {
    return cwm->handlers->recoverTransfersFromTransactionBundle (cwm, bundle);
}

private_extern OwnershipGiven BRCryptoTransfer
cryptoWalletManagerRecoverTransferFromTransferBundle (BRCryptoWalletManager cwm,
                                                      OwnershipKept BRCryptoClientTransferBundle bundle) {
    return cwm->handlers->recoverTransferFromTransferBundle (cwm, bundle);
}

private_extern void
cryptoWalletManagerHandleRecoveredTransfer (BRCryptoWalletManager cwm,
                                            OwnershipGiven BRCryptoTransfer transfer) {
    private_extern OwnershipGiven BRArrayOf(BRCryptoTransfer)
    cryptoWalletManagerRecoverTransfersFromTransactionBundle (BRCryptoWalletManager cwm,
                                                              OwnershipKept BRCryptoClientTransactionBundle bundle);

    private_extern OwnershipGiven BRCryptoTransfer
    cryptoWalletManagerRecoverTransferFromTransferBundle (BRCryptoWalletManager cwm,
                                                          OwnershipKept BRCryptoClientTransferBundle bundle);

    private_extern void
    cryptoWalletManagerHandleRecoveredTransfer (BRCryptoWalletManager cwm,
                                                OwnershipGiven BRCryptoTransfer transfer);


}


// MARK: - Generate Events

private_extern void
cryptoWalletManagerGenerateTransferEvent (BRCryptoWalletManager cwm,
                                          BRCryptoWallet wallet,
                                          BRCryptoTransfer transfer,
                                          BRCryptoTransferEvent event) {
    cwm->listener.transferEventCallback (cwm->listener.context,
                                         cryptoWalletManagerTake (cwm),
                                         cryptoWalletTake (wallet),
                                         cryptoTransferTake (transfer),
                                         event);
}
