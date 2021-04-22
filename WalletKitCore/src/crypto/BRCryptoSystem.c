//
//  BRCryptoWallet.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "support/BROSCompat.h"
#include "support/BRFileService.h"
#include "support/BRCrypto.h"

#include "crypto/BRCryptoSystemP.h"
#include "crypto/BRCryptoNetworkP.h"
#include "crypto/BRCryptoWalletManagerP.h"
#include "crypto/BRCryptoClientP.h"
#include "crypto/BRCryptoListenerP.h"

#include <stdio.h>                  // sprintf

// MARK: - All Systems

#if defined (NOT_WORKABLE_NEEDS_TO_REFERENCE_THE_SWIFT__JAVA_INSTANCE)
static pthread_once_t  _cryptoAllSystemsOnce   = PTHREAD_ONCE_INIT;
static pthread_mutex_t _cryptoAllSystemsMutex;

//
// We can't create a Swift/Java System based solely on BRCryptoSystem - because the System
// holds references to SystemClient and SystemListener which are not - and can never be - in the
// C code.
//
// We'd like to keep a table of all Systems herein; rather than distinctly in the Swift and the
// Java and the <other languages>.  To do so, requires a reference to the Swift/Java/other
// System.  Not sure what to hold, if anything.
//
// We keep System because BRCryptoListener, created in the Swift/Java, needs a 'Listener Context'
// which must be something that allows the Swift/Java to find the System.  
//
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
#endif // defined (NOT_WORKABLE_NEEDS_TO_REFERENCE_THE_SWIFT__JAVA_INSTANCE)

// MARK: - System File Service

#define FILE_SERVICE_TYPE_CURRENCY_BUNDLE      "currency-bundle"

enum {
    FILE_SERVICE_TYPE_CURRENCY_BUNDLE_VERSION_1
};

static UInt256
fileServiceTypeCurrencyBundleV1Identifier (BRFileServiceContext context,
                                           BRFileService fs,
                                           const void *entity) {
    BRCryptoSystem system = (BRCryptoSystem) context; (void) system;
    const BRCryptoClientCurrencyBundle bundle = (const BRCryptoClientCurrencyBundle) entity;

    UInt256 identifier;

    BRSHA256 (identifier.u8, bundle->id, strlen(bundle->id));

    return identifier;
}

static void *
fileServiceTypeCurrencyBundleV1Reader (BRFileServiceContext context,
                                    BRFileService fs,
                                    uint8_t *bytes,
                                    uint32_t bytesCount) {
    BRCryptoSystem system = (BRCryptoSystem) context; (void) system;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpData  data  = (BRRlpData) { bytesCount, bytes };
    BRRlpItem  item  = rlpDataGetItem (coder, data);

    BRCryptoClientCurrencyBundle bundle = cryptoClientCurrencyBundleRlpDecode(item, coder);

    rlpItemRelease (coder, item);
    rlpCoderRelease(coder);

    return bundle;
}

static uint8_t *
fileServiceTypeCurrencyBundleV1Writer (BRFileServiceContext context,
                                       BRFileService fs,
                                       const void* entity,
                                       uint32_t *bytesCount) {
    BRCryptoSystem system = (BRCryptoSystem) context; (void) system;
    const BRCryptoClientCurrencyBundle bundle = (const BRCryptoClientCurrencyBundle) entity;

    BRRlpCoder coder = rlpCoderCreate();
    BRRlpItem  item  = cryptoClientCurrencyBundleRlpEncode (bundle, coder);
    BRRlpData  data  = rlpItemGetData (coder, item);

    rlpItemRelease  (coder, item);
    rlpCoderRelease (coder);

    *bytesCount = (uint32_t) data.bytesCount;
    return data.bytes;
}

static BRSetOf (BRCryptoClientCurrencyBundle)
cryptoSystemInitialCurrencyBundlesLoad (BRCryptoSystem system) {
    BRSetOf(BRCryptoClientCurrencyBundle) bundles =  cryptoClientCurrencyBundleSetCreate(100);

    if (fileServiceHasType (system->fileService, FILE_SERVICE_TYPE_CURRENCY_BUNDLE) &&
        1 != fileServiceLoad (system->fileService, bundles, FILE_SERVICE_TYPE_CURRENCY_BUNDLE, 1)) {
        printf ("CRY: system failed to load currency bundles\n");
        cryptoClientCurrencyBundleSetRelease(bundles);
        return NULL;
    }

    return bundles;
}

static void
cryptoSystemFileServiceErrorHandler (BRFileServiceContext context,
                                            BRFileService fs,
                                            BRFileServiceError error) {
    switch (error.type) {
        case FILE_SERVICE_IMPL:
            // This actually a FATAL - an unresolvable coding error.
            printf ("CRY: System FileService Error: IMPL: %s\n", error.u.impl.reason);
            break;
        case FILE_SERVICE_UNIX:
            printf ("CRY: System FileService Error: UNIX: %s\n", strerror(error.u.unx.error));
            break;
        case FILE_SERVICE_ENTITY:
            // This is likely a coding error too.
            printf ("CRY: System FileService Error: ENTITY (%s): %s\n",
                         error.u.entity.type,
                         error.u.entity.reason);
            break;
        case FILE_SERVICE_SDB:
            printf ("CRY: System FileService Error: SDB: (%d): %s\n",
                         error.u.sdb.code,
                         error.u.sdb.reason);
            break;
    }
}

static BRFileServiceTypeSpecification systemFileServiceSpecifications[] = {
    FILE_SERVICE_TYPE_CURRENCY_BUNDLE,
    FILE_SERVICE_TYPE_CURRENCY_BUNDLE_VERSION_1,
    1,
    {
        {
            FILE_SERVICE_TYPE_CURRENCY_BUNDLE_VERSION_1,
            fileServiceTypeCurrencyBundleV1Identifier,
            fileServiceTypeCurrencyBundleV1Reader,
            fileServiceTypeCurrencyBundleV1Writer
        }
    }
};

static size_t systemFileServiceSpecificationsCount = (sizeof (systemFileServiceSpecifications) / sizeof (BRFileServiceTypeSpecification));

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

    // Create the system-state file service
    system->fileService = fileServiceCreateFromTypeSpecifications (system->path, "system", "state",
                                                                   system,
                                                                   cryptoSystemFileServiceErrorHandler,
                                                                   systemFileServiceSpecificationsCount,
                                                                   systemFileServiceSpecifications);

    // Fill in the builtin networks
    size_t networksCount = 0;
    BRCryptoNetwork *networks = cryptoNetworkInstallBuiltins (&networksCount,
                                                              cryptoListenerCreateNetworkListener(listener, system),
                                                              CRYPTO_TRUE == onMainnet);
    array_new (system->networks, networksCount);
    array_add_array (system->networks, networks, networksCount);
    free (networks);

    // Start w/ no `managers`; never more than `networksCount`
    array_new (system->managers, networksCount);

    system->ref = CRYPTO_REF_ASSIGN (cryptoSystemRelease);

    pthread_mutex_init_brd (&system->lock, PTHREAD_MUTEX_NORMAL);  // PTHREAD_MUTEX_RECURSIVE

    // Extract currency bundles
    BRSetOf (BRCryptoClientCurrencyBundle) currencyBundles = cryptoSystemInitialCurrencyBundlesLoad (system);
    if (NULL != currencyBundles) {
        FOR_SET (BRCryptoClientCurrencyBundle, currencyBundle, currencyBundles) {
            BRCryptoNetwork network = cryptoSystemGetNetworkForUids (system, currencyBundle->bid);
            if (NULL != network) {
                cryptoNetworkAddCurrencyAssociationFromBundle (network, currencyBundle, CRYPTO_FALSE);
            }
        }
        cryptoClientCurrencyBundleSetRelease(currencyBundles);
    }

    // The System has been created.
    cryptoSystemGenerateEvent (system, (BRCryptoSystemEvent) {
        CRYPTO_SYSTEM_EVENT_CREATED
    });

    // Each Network has been added to System.
    for (size_t index = 0; index < array_count(system->networks); index++)
        cryptoSystemGenerateEvent (system, (BRCryptoSystemEvent) {
            CRYPTO_SYSTEM_EVENT_NETWORK_ADDED,
            { .network = cryptoNetworkTake (system->networks[index]) }
        });

    // All the available networks have been discovered
    cryptoSystemGenerateEvent (system, (BRCryptoSystemEvent) {
        CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS
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

extern void
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

extern const char *
cryptoSystemEventTypeString (BRCryptoSystemEventType type) {
    static const char *names[] = {
        "CRYPTO_WALLET_EVENT_CREATED",
        "CRYPTO_SYSTEM_EVENT_CHANGED",
        "CRYPTO_SYSTEM_EVENT_DELETED",

        "CRYPTO_SYSTEM_EVENT_NETWORK_ADDED",
        "CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED",
        "CRYPTO_SYSTEM_EVENT_NETWORK_DELETED",

        "CRYPTO_SYSTEM_EVENT_MANAGER_ADDED",
        "CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED",
        "CRYPTO_SYSTEM_EVENT_MANAGER_DELETED",

        "CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS",
    };
    return names [type];
}

// MARK: - System Network

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

static BRCryptoNetwork
cryptoSystemGetNetworkForUidsWithIndex (BRCryptoSystem system,
                                        const char *uids,
                                        size_t *indexOfNetwork) {
    for (size_t index = 0; index < array_count(system->networks); index++) {
        if (0 == strcmp (uids, cryptoNetworkGetUids(system->networks[index]))) {
            if (NULL != indexOfNetwork) *indexOfNetwork = index;
            return cryptoNetworkTake (system->networks[index]);
        }
    }
    return NULL;
}

extern BRCryptoNetwork
cryptoSystemGetNetworkForUids (BRCryptoSystem system,
                               const char *uids) {
    return cryptoSystemGetNetworkForUidsWithIndex (system, uids, NULL);
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

    // Start the event handler.
    cryptoWalletManagerStart (manager);

    return manager;
}

// MARK: - Currency

private_extern void
cryptoSystemHandleCurrencyBundles (BRCryptoSystem system,
                                  OwnershipKept BRArrayOf (BRCryptoClientCurrencyBundle) bundles) {
    // Partition `bundles` by `network`

    size_t networksCount = array_count(system->networks);
    BRArrayOf (BRCryptoClientCurrencyBundle) bundlesForNetworks[networksCount];

    for (size_t index = 0; index < networksCount; index++)
        array_new (bundlesForNetworks[index], 10);

    size_t networkIndex = 0;
    for (size_t bundleIndex = 0; bundleIndex < array_count(bundles); bundleIndex++) {
        fileServiceSave (system->fileService, FILE_SERVICE_TYPE_CURRENCY_BUNDLE, bundles[bundleIndex]);

        BRCryptoNetwork network = cryptoSystemGetNetworkForUidsWithIndex (system, bundles[bundleIndex]->bid, &networkIndex);
        if (NULL != network)
            array_add (bundlesForNetworks[networkIndex], bundles[bundleIndex]);
    }

    for (size_t index = 0; index < networksCount; index++) {
        cryptoNetworkAddCurrencyAssociationsFromBundles (system->networks[index], bundlesForNetworks[index]);
        array_free (bundlesForNetworks[index]);
    }
}

// MARK: - System Control

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
