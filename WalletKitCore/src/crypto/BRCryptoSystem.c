//
//  BRCryptoWallet.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoSystemP.h"
#include "support/BROSCompat.h"
#include "crypto/BRCryptoListenerP.h"

#include <stdio.h>                  // sprintf

// MARK: - All Systems

#if defined (NOT_WORKABLE_NEEDS_RO_REFERENCE_THE_SWIFT__JAVA_INSTANCE)
static pthread_once_t  _cryptoAllSystemsOnce   = PTHREAD_ONCE_INIT;
static pthread_mutex_t _cryptoAllSystemsMutex;

// TODO: If this had a `void*` field to the Swift/Java reference... it might then work.
typedef struct {
    char *uids;
    BRCryptoSystem system;
    int foo;
    int bar;
} BRCryptoSystemEntry;

static BRArrayOf(BRCryptoSystemEntry) systems = NULL;

#define NUMBER_OF_SYSTEMS_DEFAULT       (3)

static void
_cryptoAllSystemsInitOnce  (void) {
    array_new (systems, NUMBER_OF_SYSTEMS_DEFAULT);
    pthread_mutex_init(&_cryptoAllSystemsMutex, NULL);
    // ...
}

static void
cryptoAllSystemsInit (void) {
    pthread_once (&_cryptoAllSystemsOnce, _cryptoAllSystemsInitOnce);
}

static BRCryptoSystemEntry *
_cryptoAllSystemsFind (BRCryptoSystem system) {
    cryptoAllSystemsInit();
    pthread_mutex_lock (&_cryptoAllSystemsMutex);
    BRCryptoSystemEntry *entry = NULL;
    pthread_mutex_unlock (&_cryptoAllSystemsMutex);

    return entry;
}

static BRCryptoSystemEntry *
_cryptoAllSystemsFindByUIDS (const char *uids) {
    cryptoAllSystemsInit();
    pthread_mutex_lock (&_cryptoAllSystemsMutex);
    BRCryptoSystemEntry *entry = NULL;
    pthread_mutex_unlock (&_cryptoAllSystemsMutex);

    return entry;
}

static void
cryptoAllSystemsAdd (BRCryptoSystem system) { // , void* context)
    cryptoAllSystemsInit();

    BRCryptoSystemEntry entry = {
        strdup (cryptoSystemGetResolvedPath(system)),
        cryptoSystemTake(system),
        0,
        0
    };
    pthread_mutex_lock (&_cryptoAllSystemsMutex);
    array_add (systems, entry);
    pthread_mutex_unlock (&_cryptoAllSystemsMutex);
}
#endif

// MARK: - System
IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoSystem, cryptoSystem)

extern BRCryptoSystem
cryptoSystemCreate (BRCryptoClient client,
                    BRCryptoListener listener,
                    BRCryptoAccount account,
                    const char *basePath,
                    BRCryptoBoolean onMainnet) {
    BRCryptoSystem system = calloc (1, sizeof (struct BRCryptoSystemRecord));

    system->state       = CRYPTO_SYSTEM_STATE_CREATED;
    system->onMainnet   = onMainnet;
    system->isReachable = CRYPTO_TRUE;
    system->client      = client;
    system->listener    = cryptoListenerTake (listener);
    system->account     = cryptoAccountTake  (account);

    // Build a `path` specific to `account`
    char *accountFileSystemIdentifier = cryptoAccountGetFileSystemIdentifier(account);
    system->path = malloc (strlen(basePath) + 1 + strlen(accountFileSystemIdentifier) + 1);
    sprintf (system->path, "%s/%s", basePath, accountFileSystemIdentifier);
    free (accountFileSystemIdentifier);

    // Fill in the builtin networks
    size_t networksCount = 0;
    BRCryptoNetwork *networks = cryptoNetworkInstallBuiltins (&networksCount,
                                                              cryptoListenerCreateNetworkListener(listener, system),
                                                              CRYPTO_TRUE == onMainnet);
    array_new (system->networks, networksCount);
    array_add_array (system->networks, networks, networksCount);

    // Start w/ no `managers`; never more than `networksCount`
    array_new (system->managers, networksCount);

    system->ref = CRYPTO_REF_ASSIGN (cryptoSystemRelease);

    pthread_mutex_init_brd (&system->lock, PTHREAD_MUTEX_NORMAL);  // PTHREAD_MUTEX_RECURSIVE

    cryptoSystemGenerateEvent (system, (BRCryptoSystemEvent) {
        CRYPTO_SYSTEM_EVENT_CREATED
    });

#if defined (NOT_WORKABLE_NEEDS_RO_REFERENCE_THE_SWIFT__JAVA_INSTANCE)
    cryptoAllSystemsAdd (system);
#endif

    return system;
}

static void
cryptoSystemRelease (BRCryptoSystem system) {
    pthread_mutex_lock (&system->lock);
    cryptoSystemSetState (system, CRYPTO_SYSTEM_STATE_DELETED);

    // Networks
    array_free_all (system->networks, cryptoNetworkGive);

    // Managers
    array_free_all (system->managers, cryptoWalletManagerGive);

    cryptoAccountGive  (system->account);
    cryptoListenerGive (system->listener);
    free (system->path);

    pthread_mutex_unlock  (&system->lock);
    pthread_mutex_destroy (&system->lock);

    #if 0
        cryptoWalletGenerateEvent (wallet, (BRCryptoWalletEvent) {
            CRYPTO_WALLET_EVENT_DELETED
        });
    #endif

    memset (system, 0, sizeof(*system));
    free (system);
}

extern BRCryptoBoolean
cryptoSystemOnMainnet (BRCryptoSystem system) {
    return system->onMainnet;
}

extern BRCryptoBoolean
cryptoSystemIsReachable (BRCryptoSystem system) {
    return system->isReachable;
}

private_extern void
cryptoSystemSetReachable (BRCryptoSystem system,
                          BRCryptoBoolean isReachable) {
    system->isReachable = isReachable;
    for (size_t index = 0; index < array_count(system->managers); index++)
        cryptoWalletManagerSetNetworkReachable (system->managers[index], isReachable);
}

extern const char *
cryptoSystemGetResolvedPath (BRCryptoSystem system) {
    return system->path;
}

extern BRCryptoSystemState
cryptoSystemGetState (BRCryptoSystem system) {
    return system->state;
}

private_extern void
cryptoSystemSetState (BRCryptoSystem system,
                      BRCryptoSystemState state) {
    BRCryptoSystemState newState = state;
    BRCryptoSystemState oldState = system->state;

    system->state = state;

    if (oldState != newState)
         cryptoSystemGenerateEvent (system, (BRCryptoSystemEvent) {
             CRYPTO_SYSTEM_EVENT_CHANGED,
             { .state = { oldState, newState }}
         });
}

// MARK: System Network

static BRCryptoBoolean
cryptoSystemHasNetworkFor (BRCryptoSystem system,
                           BRCryptoNetwork network,
                           size_t *forIndex) {
    for (size_t index = 0; index < array_count(system->networks); index++) {
        if (network == system->networks[index]) {
            if (forIndex) *forIndex = index;
            return CRYPTO_TRUE;
        }
    }
    return CRYPTO_FALSE;
}

extern BRCryptoBoolean
cryptoSystemHasNetwork (BRCryptoSystem system,
                        BRCryptoNetwork network) {
    return cryptoSystemHasNetworkFor (system, network, NULL);
}

extern BRCryptoNetwork *
cryptoSystemGetNetworks (BRCryptoSystem system,
                         size_t *count) {
    *count = array_count (system->networks);
    BRCryptoNetwork *networks = NULL;
    if (0 != *count) {
        networks = calloc (*count, sizeof(BRCryptoNetwork));
        for (size_t index = 0; index < *count; index++) {
            networks[index] = cryptoNetworkTake(system->networks[index]);
        }
    }
    return networks;
}

extern BRCryptoNetwork
 cryptoSystemGetNetworkAt (BRCryptoSystem system,
                           size_t index) {
     return index < array_count(system->networks) ? system->networks[index] : NULL;
}

extern BRCryptoNetwork
cryptoSystemGetNetworkForUids (BRCryptoSystem system,
                               const char *uids) {
    for (size_t index = 0; index < array_count(system->networks); index++) {
        if (0 == strcmp (uids, cryptoNetworkGetUids(system->networks[index])))
            return cryptoNetworkTake (system->networks[index]);
    }
    return NULL;
}

extern size_t
cryptoSystemGetNetworksCount (BRCryptoSystem system) {
    return array_count(system->networks);
}

private_extern void
cryptoSystemAddNetwork (BRCryptoSystem system,
                        BRCryptoNetwork network) {
    if (CRYPTO_FALSE == cryptoSystemHasNetwork (system, network)) {
        array_add (system->networks, cryptoNetworkTake(network));
        cryptoListenerGenerateSystemEvent (system->listener, system, (BRCryptoSystemEvent) {
            CRYPTO_SYSTEM_EVENT_NETWORK_ADDED,
            { .network = cryptoNetworkTake (network) }
        });
    }
}

private_extern void
cryptoSystemRemNetwork (BRCryptoSystem system,
                        BRCryptoNetwork network) {
    size_t index;
    if (CRYPTO_TRUE == cryptoSystemHasNetworkFor (system, network, &index)) {
        array_rm (system->networks, index);
        cryptoListenerGenerateSystemEvent (system->listener, system, (BRCryptoSystemEvent) {
             CRYPTO_SYSTEM_EVENT_NETWORK_DELETED,
            { .network = network }  // no cryptoNetworkTake -> releases system->networks reference
         });
    }
}

// MARK: - System Wallet Managers

static BRCryptoBoolean
cryptoSystemHasWalletManagerFor (BRCryptoSystem system,
                                 BRCryptoWalletManager manager,
                                 size_t *forIndex) {
    for (size_t index = 0; index < array_count(system->managers); index++) {
        if (manager == system->managers[index]) {
            if (forIndex) *forIndex = index;
            return CRYPTO_TRUE;
        }
    }
    return CRYPTO_FALSE;
}

extern BRCryptoBoolean
cryptoSystemHasWalletManager (BRCryptoSystem system,
                              BRCryptoWalletManager manager) {
    return cryptoSystemHasWalletManagerFor (system, manager, NULL);

}

extern BRCryptoWalletManager *
cryptoSystemGetWalletManagers (BRCryptoSystem system,
                               size_t *count) {
    *count = array_count (system->managers);
    BRCryptoWalletManager *managers = NULL;
    if (0 != *count) {
        managers = calloc (*count, sizeof(BRCryptoWalletManager));
        for (size_t index = 0; index < *count; index++) {
            managers[index] = cryptoWalletManagerTake(system->managers[index]);
        }
    }
    return managers;
}

extern BRCryptoWalletManager
cryptoSystemGetWalletManagerAt (BRCryptoSystem system,
                                size_t index) {
    return index < array_count(system->managers) ? system->managers[index] : NULL;
}

extern BRCryptoWalletManager
cryptoSystemGetWalletManagerByNetwork (BRCryptoSystem system,
                                       BRCryptoNetwork network) {
    for (size_t index = 0; index < array_count(system->managers); index++)
        if (cryptoWalletManagerHasNetwork (system->managers[index], network))
            return system->managers[index];
    return NULL;
}

extern size_t
cryptoSystemGetWalletManagersCount (BRCryptoSystem system) {
    return array_count(system->managers);
}

private_extern void
cryptoSystemAddWalletManager (BRCryptoSystem system,
                              BRCryptoWalletManager manager) {
    if (CRYPTO_FALSE == cryptoSystemHasWalletManager (system, manager)) {
        array_add (system->managers, cryptoWalletManagerTake(manager));
        cryptoListenerGenerateSystemEvent (system->listener, system, (BRCryptoSystemEvent) {
            CRYPTO_SYSTEM_EVENT_MANAGER_ADDED,
            { .manager = cryptoWalletManagerTake (manager) }
        });
    }
}

private_extern void
cryptoSystemRemWalletManager (BRCryptoSystem system,
                              BRCryptoWalletManager manager) {
    size_t index;
    if (CRYPTO_TRUE == cryptoSystemHasWalletManagerFor (system, manager, &index)) {
        array_rm (system->managers, index);
        cryptoListenerGenerateSystemEvent (system->listener, system, (BRCryptoSystemEvent) {
             CRYPTO_SYSTEM_EVENT_MANAGER_DELETED,
            { .manager = manager }  // no cryptoNetworkTake -> releases system->managers reference
         });
    }
}

extern BRCryptoWalletManager
cryptoSystemCreateWalletManager (BRCryptoSystem system,
                                 BRCryptoNetwork network,
                                 BRCryptoSyncMode mode,
                                 BRCryptoAddressScheme scheme,
                                 BRCryptoCurrency *currencies,
                                 size_t currenciesCount) {
    if (CRYPTO_FALSE == cryptoNetworkIsAccountInitialized (network, system->account)) {
        return NULL;
    }

    BRCryptoWalletManager manager =
    cryptoWalletManagerCreate (cryptoListenerCreateWalletManagerListener (system->listener, system),
                               system->client,
                               system->account,
                               network,
                               mode,
                               scheme,
                               system->path);

    cryptoSystemAddWalletManager (system, manager);

    cryptoWalletManagerSetNetworkReachable (manager, system->isReachable);

    for (size_t index = 0; index < currenciesCount; index++)
        if (cryptoNetworkHasCurrency (network, currencies[index]))
            cryptoWalletManagerCreateWallet (manager, currencies[index]);

    return manager;
}

extern void
cryptoSystemStart (BRCryptoSystem system) {
    cryptoListenerStart(system->listener);
    // client
    // query
}

extern void
cryptoSystemStop (BRCryptoSystem system) {
    // query
    // client
    cryptoListenerStop(system->listener);
}

extern void
cryptoSystemConnect (BRCryptoSystem system) {
    for (size_t index = 0; index < array_count(system->managers); index++)
        cryptoWalletManagerConnect (system->managers[index], NULL);
}

extern void
cryptoSystemDisconnect (BRCryptoSystem system) {
    for (size_t index = 0; index < array_count(system->managers); index++)
         cryptoWalletManagerDisconnect (system->managers[index]);
}

extern const char *
cryptoSystemEventTypeString (BRCryptoSystemEventType type) {
    static const char *names[] = {
        "CRYPTO_WALLET_EVENT_CREATED",
    };
    return names [type];
}
