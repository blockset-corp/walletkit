//
//  WKNetwork.c
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <ctype.h>

#include "WKNetworkP.h"
#include "WKUnit.h"
#include "WKAddressP.h"
#include "WKAmountP.h"
#include "WKAccountP.h"
#include "WKHashP.h"

#include "WKHandlersP.h"

// If '1' then display a detailed list of the builting currencies for each network
#define SHOW_BUILTIN_CURRENCIES 0 // DEBUG

private_extern BRArrayOf(WKUnit)
wkUnitGiveAll (BRArrayOf(WKUnit) units);

/// MARK: - Network Canonical Type

extern const char *
wkBlockChainTypeGetCurrencyCode (WKNetworkType type) {
    static const char *currencies[NUMBER_OF_NETWORK_TYPES] = {
        WK_NETWORK_CURRENCY_BTC,
        WK_NETWORK_CURRENCY_BCH,
        WK_NETWORK_CURRENCY_BSV,
        WK_NETWORK_CURRENCY_LTC,
        WK_NETWORK_CURRENCY_ETH,
        WK_NETWORK_CURRENCY_XRP,
        WK_NETWORK_CURRENCY_HBAR,
        WK_NETWORK_CURRENCY_XTZ,
        // "Stellar"
    };
    assert (type < NUMBER_OF_NETWORK_TYPES);
    return currencies[type];
}

/// MARK: - Network Fee

IMPLEMENT_WK_GIVE_TAKE (WKNetworkFee, wkNetworkFee)

extern WKNetworkFee
wkNetworkFeeCreate (uint64_t confirmationTimeInMilliseconds,
                        WKAmount pricePerCostFactor,
                        WKUnit   pricePerCostFactorUnit) {
    WKNetworkFee networkFee = calloc (1, sizeof (struct WKNetworkFeeRecord));

    networkFee->confirmationTimeInMilliseconds = confirmationTimeInMilliseconds;
    networkFee->pricePerCostFactor     = wkAmountTake (pricePerCostFactor);
    networkFee->pricePerCostFactorUnit = wkUnitTake (pricePerCostFactorUnit);
    networkFee->ref = WK_REF_ASSIGN(wkNetworkFeeRelease);

    return networkFee;
}

extern uint64_t
wkNetworkFeeGetConfirmationTimeInMilliseconds (WKNetworkFee networkFee) {
    return networkFee->confirmationTimeInMilliseconds;
}

extern WKAmount
wkNetworkFeeGetPricePerCostFactor (WKNetworkFee networkFee) {
    return wkAmountTake (networkFee->pricePerCostFactor);
}

extern WKUnit
wkNetworkFeeGetPricePerCostFactorUnit (WKNetworkFee networkFee) {
    return wkUnitTake (networkFee->pricePerCostFactorUnit);
}

extern WKBoolean
wkNetworkFeeEqual (WKNetworkFee nf1, WKNetworkFee nf2) {
    return AS_WK_BOOLEAN (nf1 == nf2 ||
                              (nf1->confirmationTimeInMilliseconds == nf2->confirmationTimeInMilliseconds) &&
                              WK_COMPARE_EQ == wkAmountCompare (nf1->pricePerCostFactor, nf2->pricePerCostFactor));
}

static void
wkNetworkFeeRelease (WKNetworkFee networkFee) {
    wkAmountGive (networkFee->pricePerCostFactor);
    wkUnitGive   (networkFee->pricePerCostFactorUnit);

    memset (networkFee, 0, sizeof(*networkFee));
    free (networkFee);
}

// MARK: - Crypto Association

static void
wkCurrencyAssociationRelease (WKCurrencyAssociation association) {
    wkCurrencyGive (association.currency);
    wkUnitGive (association.baseUnit);
    wkUnitGive (association.defaultUnit);
    wkUnitGiveAll (association.units);
    array_free (association.units);
}
/// MARK: - Network

#define WK_NETWORK_DEFAULT_CURRENCY_ASSOCIATIONS        (2)
#define WK_NETWORK_DEFAULT_FEES                         (3)
#define WK_NETWORK_DEFAULT_NETWORKS                     (5)

IMPLEMENT_WK_GIVE_TAKE (WKNetwork, wkNetwork)

extern WKNetwork
wkNetworkAllocAndInit (size_t sizeInBytes,
                           WKNetworkType type,
                           WKNetworkListener listener,
                           const char *uids,
                           const char *name,
                           const char *desc,
                           bool isMainnet,
                           uint32_t confirmationPeriodInSeconds,
                           WKAddressScheme defaultAddressScheme,
                           WKSyncMode defaultSyncMode,
                           WKCurrency currencyNative,
                           WKNetworkCreateContext createContext,
                           WKNetworkCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct WKNetworkRecord));
    WKNetwork network = calloc (1, sizeInBytes);

    network->type = type;
    network->handlers = wkHandlersLookup(type)->network;
    network->sizeInBytes = sizeInBytes;

    network->listener = listener;

    network->uids = strdup (uids);
    network->name = strdup (name);
    network->desc = strdup (desc);

    network->isMainnet = isMainnet;
    network->currency  = wkCurrencyTake (currencyNative);
    network->height    = 0;

    array_new (network->associations, WK_NETWORK_DEFAULT_CURRENCY_ASSOCIATIONS);
    array_new (network->fees, WK_NETWORK_DEFAULT_FEES);

    network->confirmationPeriodInSeconds = confirmationPeriodInSeconds;

    network->defaultAddressScheme = defaultAddressScheme;
    array_new (network->addressSchemes, NUMBER_OF_ADDRESS_SCHEMES);
    array_add (network->addressSchemes, network->defaultAddressScheme);

    network->defaultSyncMode = defaultSyncMode;
    array_new (network->syncModes, NUMBER_OF_SYNC_MODES);
    array_add (network->syncModes, network->defaultSyncMode);

    network->verifiedBlockHash = NULL;

    network->ref = WK_REF_ASSIGN(wkNetworkRelease);

    pthread_mutex_init_brd (&network->lock, PTHREAD_MUTEX_RECURSIVE);

    if (NULL != createCallback) createCallback (createContext, network);

    wkNetworkGenerateEvent (network, (WKNetworkEvent) {
        WK_NETWORK_EVENT_CREATED
    });

    return network;
}

static void
wkNetworkRelease (WKNetwork network) {
    network->handlers->release (network);

    wkHashGive (network->verifiedBlockHash);

    array_free_all (network->associations, wkCurrencyAssociationRelease);

    for (size_t index = 0; index < array_count (network->fees); index++) {
        wkNetworkFeeGive (network->fees[index]);
    }
    array_free (network->fees);

    if (network->addressSchemes) array_free (network->addressSchemes);
    if (network->syncModes)      array_free (network->syncModes);

    free (network->desc);
    free (network->name);
    free (network->uids);
    if (NULL != network->currency) wkCurrencyGive (network->currency);
    pthread_mutex_destroy (&network->lock);

    memset (network, 0, network->sizeInBytes);
    free (network);
}

private_extern WKNetworkType
wkNetworkGetType (WKNetwork network) {
    return network->type;
}

extern const char *
wkNetworkGetUids (WKNetwork network) {
    return network->uids;
}

extern const char *
wkNetworkGetName (WKNetwork network) {
    return network->name;
}

private_extern const char *
wkNetworkGetDesc (WKNetwork network) {
    return network->desc;
}

extern WKBoolean
wkNetworkIsMainnet (WKNetwork network) {
    return network->isMainnet;
}

private_extern uint32_t
wkNetworkGetConfirmationPeriodInSeconds (WKNetwork network) {
    return network->confirmationPeriodInSeconds;
}

extern WKBlockNumber
wkNetworkGetHeight (WKNetwork network) {
    pthread_mutex_lock (&network->lock);
    WKBlockNumber height = network->height;
    pthread_mutex_unlock (&network->lock);
    return height;
}

extern void
wkNetworkSetHeight (WKNetwork network,
                        WKBlockNumber height) {
    pthread_mutex_lock (&network->lock);
    network->height = height;
    pthread_mutex_unlock (&network->lock);
}

extern WKHash
wkNetworkGetVerifiedBlockHash (WKNetwork network) {
    return wkHashTake (network->verifiedBlockHash);
}

extern void
wkNetworkSetVerifiedBlockHash (WKNetwork network,
                                   WKHash verifiedBlockHash) {
    pthread_mutex_lock (&network->lock);
    wkHashGive (network->verifiedBlockHash);
    network->verifiedBlockHash = wkHashTake (verifiedBlockHash);
    pthread_mutex_unlock (&network->lock);
}

extern void
wkNetworkSetVerifiedBlockHashAsString (WKNetwork network,
                                           const char * blockHashString) {
    WKHash verifiedBlockHash = wkNetworkCreateHashFromString (network, blockHashString);
    wkNetworkSetVerifiedBlockHash (network, verifiedBlockHash);
    wkHashGive (verifiedBlockHash);
}

extern uint32_t
wkNetworkGetConfirmationsUntilFinal (WKNetwork network) {
    return network->confirmationsUntilFinal;
}

extern void
wkNetworkSetConfirmationsUntilFinal (WKNetwork network,
                                         uint32_t confirmationsUntilFinal) {
    network->confirmationsUntilFinal = confirmationsUntilFinal;
}

extern WKCurrency
wkNetworkGetCurrency (WKNetwork network) {
    pthread_mutex_lock (&network->lock);
    WKCurrency currency = wkCurrencyTake (network->currency);
    pthread_mutex_unlock (&network->lock);
    return currency;
}

extern const char *
wkNetworkGetCurrencyCode (WKNetwork network) {
    return wkCurrencyGetCode (network->currency);
}

extern size_t
wkNetworkGetCurrencyCount (WKNetwork network) {
    pthread_mutex_lock (&network->lock);
    size_t count = array_count (network->associations);
    pthread_mutex_unlock (&network->lock);
    return count;
}

extern WKCurrency
wkNetworkGetCurrencyAt (WKNetwork network,
                            size_t index) {
    pthread_mutex_lock (&network->lock);
    assert (index < array_count(network->associations));
    WKCurrency currency = wkCurrencyTake (network->associations[index].currency);
    pthread_mutex_unlock (&network->lock);
    return currency;
}

extern WKBoolean
wkNetworkHasCurrency (WKNetwork network,
                          WKCurrency currency) {
    WKBoolean r = WK_FALSE;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations) && WK_FALSE == r; index++) {
        r = wkCurrencyIsIdentical (network->associations[index].currency, currency);
    }
    pthread_mutex_unlock (&network->lock);
    return r;
}

extern WKCurrency
wkNetworkGetCurrencyForCode (WKNetwork network,
                                   const char *code) {
    WKCurrency currency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        if (0 == strcmp (code, wkCurrencyGetCode (network->associations[index].currency))) {
            currency = wkCurrencyTake (network->associations[index].currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return currency;
}

extern WKCurrency
wkNetworkGetCurrencyForUids (WKNetwork network,
                                 const char *uids) {
    WKCurrency currency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        if (0 == strcasecmp (uids, wkCurrencyGetUids (network->associations[index].currency))) {
            currency = wkCurrencyTake (network->associations[index].currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return currency;
}

extern WKCurrency
wkNetworkGetCurrencyForIssuer (WKNetwork network,
                                   const char *issuer) {
    WKCurrency currency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        const char *i = wkCurrencyGetIssuer(network->associations[index].currency);
        if (NULL != i && 0 == strcasecmp (issuer, i)) {
            currency = wkCurrencyTake (network->associations[index].currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return currency;
}

static WKCurrencyAssociation *
wkNetworkLookupCurrencyAssociation (WKNetwork network,
                                        WKCurrency currency) {
    // lock is not held for this static method; caller must hold it
    for (size_t index = 0; index < array_count(network->associations); index++) {
        if (WK_TRUE == wkCurrencyIsIdentical (currency, network->associations[index].currency)) {
            return &network->associations[index];
        }
    }
    return NULL;
}

static WKCurrencyAssociation *
wkNetworkLookupCurrencyAssociationByUids (WKNetwork network,
                                              const char *uids) {
    // lock is not held for this static method; caller must hold it
    for (size_t index = 0; index < array_count(network->associations); index++) {
        if (wkCurrencyHasUids (network->associations[index].currency, uids))
            return &network->associations[index];
    }
    return NULL;
}

extern WKUnit
wkNetworkGetUnitAsBase (WKNetwork network,
                            WKCurrency currency) {
    pthread_mutex_lock (&network->lock);
    currency = (NULL == currency ? network->currency : currency);
    WKCurrencyAssociation *association = wkNetworkLookupCurrencyAssociation (network, currency);
    WKUnit unit = NULL == association ? NULL : wkUnitTake (association->baseUnit);
    pthread_mutex_unlock (&network->lock);
    return unit;
}

extern WKUnit
wkNetworkGetUnitAsDefault (WKNetwork network,
                               WKCurrency currency) {
    pthread_mutex_lock (&network->lock);
    currency = (NULL == currency ? network->currency : currency);
    WKCurrencyAssociation *association = wkNetworkLookupCurrencyAssociation (network, currency);
    WKUnit unit = NULL == association ? NULL : wkUnitTake (association->defaultUnit);
    pthread_mutex_unlock (&network->lock);
    return unit;
}

extern size_t
wkNetworkGetUnitCount (WKNetwork network,
                           WKCurrency currency) {
    pthread_mutex_lock (&network->lock);
    currency = (NULL == currency ? network->currency : currency);
    WKCurrencyAssociation *association = wkNetworkLookupCurrencyAssociation (network, currency);
    size_t count = ((NULL == association || NULL == association->units)
                    ? 0
                    : array_count (association->units));
    pthread_mutex_unlock (&network->lock);
    return count;
}

extern WKUnit
wkNetworkGetUnitAt (WKNetwork network,
                        WKCurrency currency,
                        size_t index) {
    pthread_mutex_lock (&network->lock);
    currency = (NULL == currency ? network->currency : currency);
    WKCurrencyAssociation *association = wkNetworkLookupCurrencyAssociation (network, currency);
    WKUnit unit = ((NULL == association || NULL == association->units || index >= array_count(association->units))
                         ? NULL
                         : wkUnitTake (association->units[index]));
    pthread_mutex_unlock (&network->lock);
    return unit;
}

extern void
wkNetworkAddCurrency (WKNetwork network,
                          WKCurrency currency,
                          WKUnit baseUnit,
                          WKUnit defaultUnit) {
    WKCurrencyAssociation association = {
        wkCurrencyTake (currency),
        wkUnitTake (baseUnit),
        wkUnitTake (defaultUnit),
        NULL
    };

    pthread_mutex_lock (&network->lock);
    array_new (association.units, 2);
    array_add (network->associations, association);
    pthread_mutex_unlock (&network->lock);
}

extern void
wkNetworkAddCurrencyUnit (WKNetwork network,
                              WKCurrency currency,
                              WKUnit unit) {
    pthread_mutex_lock (&network->lock);
    WKCurrencyAssociation *association = wkNetworkLookupCurrencyAssociation (network, currency);
    if (NULL != association) array_add (association->units, wkUnitTake (unit));
    pthread_mutex_unlock (&network->lock);
}

// MARK: - Currency Association

private_extern void
wkNetworkAddCurrencyAssociationFromBundle (WKNetwork network,
                                               OwnershipKept WKClientCurrencyBundle bundle,
                                               WKBoolean needEvent) {

    pthread_mutex_lock (&network->lock);

    // Lookup an existing association for the bundle's id
    WKCurrencyAssociation *association = wkNetworkLookupCurrencyAssociationByUids (network, bundle->id);

    // If one exists; do not replace
    // TODO: Add an argument for `bool replace`
    if (NULL != association) {
        pthread_mutex_unlock (&network->lock);
        return;
    }

    WKCurrency currency = wkCurrencyCreate (bundle->id,
                                                      bundle->name,
                                                      bundle->code,
                                                      bundle->type,
                                                      bundle->address);

    BRArrayOf(WKUnit) units;
    array_new (units, array_count(bundle->denominations));

    // Find the base unit
    WKUnit baseUnit    = NULL;

    for (size_t index = 0; index < array_count (bundle->denominations); index++) {
        WKClientCurrencyDenominationBundle demBundle = bundle->denominations[index];
        if (0 == demBundle->decimals) {
            baseUnit = wkUnitCreateAsBase (currency, demBundle->code, demBundle->name, demBundle->symbol);
            break;
        }
    }

    if (NULL == baseUnit) {
        const char *code = wkCurrencyGetCode(currency);
        const char *name = wkCurrencyGetName(currency);

        char unitCode[strlen(code) + 1 + 1]; // lowecase+i
        char unitName[strlen(name) + 4 + 1]; // +" INT"
        char unitSymb[strlen(code) + 1 + 1]; // uppercase+I

        sprintf (unitCode, "%si", code);
        sprintf (unitName, "%s INT", name);
        sprintf (unitSymb, "%sI", code);

        for (size_t index = 0; index < strlen (unitCode); index++) {
            unitCode[index] = _tolower (unitCode[index]);
            unitSymb[index] = _toupper (unitSymb[index]);
        }

        baseUnit = wkUnitCreateAsBase (currency,
                                           unitCode,
                                           unitName,
                                           unitSymb);
    }
    array_add (units, baseUnit);

    for (size_t index = 0; index < array_count (bundle->denominations); index++) {
        WKClientCurrencyDenominationBundle demBundle = bundle->denominations[index];
        if (0 != demBundle->decimals) {
            WKUnit unit = wkUnitCreate (currency,
                                                  demBundle->code,
                                                  demBundle->name,
                                                  demBundle->symbol,
                                                  baseUnit,
                                                  demBundle->decimals);
            array_add (units, unit);
        }
    }

    // Find the default Unit - maximum decimals

    WKUnit defaultUnit = NULL;

    if (2 == array_count(units))
        defaultUnit = units[1];
    else {
        uint8_t decimals = 0;
        for (size_t index = 0; index < array_count(units); index++) {
            if (wkUnitGetBaseDecimalOffset(units[index]) > decimals) {
                defaultUnit = units[index];
                decimals = wkUnitGetBaseDecimalOffset (defaultUnit);
            }
        }
        if (NULL == defaultUnit) defaultUnit = baseUnit;
    }
    assert (NULL != defaultUnit);

    WKCurrencyAssociation newAssociation = {
        currency,
        baseUnit,
        defaultUnit,
        units
    };
    array_add (network->associations, newAssociation);
    pthread_mutex_unlock (&network->lock);

    if (WK_TRUE == needEvent)
        wkNetworkGenerateEvent (network, (WKNetworkEvent) {
            WK_NETWORK_EVENT_CURRENCIES_UPDATED
        });
}

private_extern void
wkNetworkAddCurrencyAssociationsFromBundles (WKNetwork network,
                                               OwnershipKept BRArrayOf(WKClientCurrencyBundle) bundles) {
    for (size_t index = 0; index < array_count(bundles); index++)
        wkNetworkAddCurrencyAssociationFromBundle (network, bundles[index], WK_FALSE);

    wkNetworkGenerateEvent (network, (WKNetworkEvent) {
        WK_NETWORK_EVENT_CURRENCIES_UPDATED
    });
}

// MARK: - Network Fees

extern void
wkNetworkAddNetworkFee (WKNetwork network,
                            WKNetworkFee fee) {
    pthread_mutex_lock (&network->lock);
    array_add (network->fees, wkNetworkFeeTake (fee));
    pthread_mutex_unlock (&network->lock);
}

static void
wkNetworkSetNetworkFeesInternal (WKNetwork network,
                                     const WKNetworkFee *fees,
                                     size_t count,
                                     bool needEvent) {
    assert (0 != count);
    pthread_mutex_lock (&network->lock);
    array_apply (network->fees, wkNetworkFeeGive);
    array_clear (network->fees);
    for (size_t idx = 0; idx < count; idx++) {
        array_add (network->fees, wkNetworkFeeTake (fees[idx]));
    }

    if (needEvent)
        wkListenerGenerateNetworkEvent (&network->listener, network, (WKNetworkEvent) {
            WK_NETWORK_EVENT_FEES_UPDATED
        });
    pthread_mutex_unlock (&network->lock);
}

extern void
wkNetworkSetNetworkFees (WKNetwork network,
                             const WKNetworkFee *fees,
                             size_t count) {
    wkNetworkSetNetworkFeesInternal (network, fees, count, true);
}

extern WKNetworkFee *
wkNetworkGetNetworkFees (WKNetwork network,
                             size_t *count) {
    pthread_mutex_lock (&network->lock);
    *count = array_count (network->fees);
    WKNetworkFee *fees = NULL;
    if (0 != *count) {
        fees = calloc (*count, sizeof(WKNetworkFee));
        for (size_t index = 0; index < *count; index++) {
            fees[index] = wkNetworkFeeTake(network->fees[index]);
        }
    }
    pthread_mutex_unlock (&network->lock);
    return fees;
}

// MARK: - Address Scheme

extern WKAddressScheme
wkNetworkGetDefaultAddressScheme (WKNetwork network) {
    assert (NULL != network->addressSchemes);
    return network->defaultAddressScheme;
}

static void
wkNetworkAddSupportedAddressScheme (WKNetwork network,
                                        WKAddressScheme scheme) {
    array_add (network->addressSchemes, scheme);
}

extern const WKAddressScheme *
wkNetworkGetSupportedAddressSchemes (WKNetwork network,
                                         WKCount *count) {
    assert (NULL != network->addressSchemes);
    assert (NULL != count);
    *count = array_count(network->addressSchemes);
    return network->addressSchemes;
}

extern WKBoolean
wkNetworkSupportsAddressScheme (WKNetwork network,
                                    WKAddressScheme scheme) {
    assert (NULL != network->addressSchemes);
    for (size_t index = 0; index < array_count (network->addressSchemes); index++)
        if (scheme == network->addressSchemes[index])
            return WK_TRUE;
    return WK_FALSE;
}

// MARK: - Address

extern WKAddress
wkNetworkCreateAddress (WKNetwork network,
                            const char *address) {
    return network->handlers->createAddress (network, address);
}

// MARK: - Sync Mode

extern WKSyncMode
wkNetworkGetDefaultSyncMode (WKNetwork network) {
    assert (NULL != network->syncModes);
    return network->defaultSyncMode;
}

static void
wkNetworkAddSupportedSyncMode (WKNetwork network,
                                   WKSyncMode scheme) {
    array_add (network->syncModes, scheme);
}

extern const WKSyncMode *
wkNetworkGetSupportedSyncModes (WKNetwork network,
                                    WKCount *count) {
    assert (NULL != network->syncModes);
    assert (NULL != count);
    *count = array_count(network->syncModes);
    return network->syncModes;
}

extern WKBoolean
wkNetworkSupportsSyncMode (WKNetwork network,
                               WKSyncMode mode) {
    assert (NULL != network->syncModes);
    for (size_t index = 0; index < array_count (network->syncModes); index++)
        if (mode == network->syncModes[index])
            return WK_TRUE;
    return WK_FALSE;
}

extern WKBoolean
wkNetworkRequiresMigration (WKNetwork network) {
    switch (network->type) {
        case WK_NETWORK_TYPE_BTC:
        case WK_NETWORK_TYPE_BCH:
            return WK_TRUE;
        default:
            return WK_FALSE;
    }
}

extern WKBoolean
wkNetworkIsAccountInitialized (WKNetwork network,
                                   WKAccount account) {
    return network->handlers->isAccountInitialized (network, account);
}


extern uint8_t *
wkNetworkGetAccountInitializationData (WKNetwork network,
                                           WKAccount account,
                                           size_t *bytesCount) {
    return network->handlers->getAccountInitializationData (network, account, bytesCount);
}

extern void
wkNetworkInitializeAccount (WKNetwork network,
                                WKAccount account,
                                const uint8_t *bytes,
                                size_t bytesCount) {
    network->handlers->initializeAccount (network, account, bytes, bytesCount);
}

private_extern WKNetworkType
wkNetworkGetBlockChainType (WKNetwork network) {
    return network->type;
}

private_extern WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestamp (WKNetwork network,
                                                WKTimestamp timestamp) {
    return network->handlers->getBlockNumberAtOrBeforeTimestamp (network, timestamp);
}

private_extern WKHash
wkNetworkCreateHashFromString (WKNetwork network,
                                   const char *string) {
    return network->handlers->createHashFromString (network, string);
}

private_extern OwnershipGiven char *
wkNetworkEncodeHash (WKHash hash) {
    const WKHandlers *handlers = wkHandlersLookup (hash->type);
    return handlers->network->encodeHash (hash);
}

extern const char *
wkNetworkEventTypeString (WKNetworkEventType type) {
    static const char *names[] = {
        "WK_NETWORK_EVENT_CREATED",
        "WK_NETWORK_EVENT_FEES_UPDATED",
        "WK_NETWORK_EVENT_DELETED"
    };
    return names [type];
}

// MARK: - Network Defaults

private_extern WKNetwork *
wkNetworkInstallBuiltins (WKCount *networksCount,
                              WKNetworkListener listener,
                              bool isMainnet) {
    // Network Specification
    struct NetworkSpecification {
        WKNetworkType type;
        char *networkId;
        char *name;
        char *network;
        bool isMainnet;
        uint64_t height;
        uint32_t confirmations;
        uint32_t confirmationPeriodInSeconds;
    } networkSpecifications[] = {
#define DEFINE_NETWORK(type, networkId, name, network, isMainnet, height, confirmations, confirmationPeriodInSeconds) \
{ type, networkId, name, network, isMainnet, height, confirmations, confirmationPeriodInSeconds },
#include "WKConfig.h"
//        { NULL }
    };
    size_t NUMBER_OF_NETWORKS = sizeof (networkSpecifications) / sizeof (struct NetworkSpecification);

    // Network Fee Specification
    struct NetworkFeeSpecification {
        char *networkId;
        char *amount;
        char *tier;
        uint32_t confirmationTimeInMilliseconds;
    } networkFeeSpecifications[] = {
#define DEFINE_NETWORK_FEE_ESTIMATE(networkId, amount, tier, confirmationTimeInMilliseconds)\
{ networkId, amount, tier, confirmationTimeInMilliseconds },
#include "WKConfig.h"
    };
    size_t NUMBER_OF_FEES = sizeof (networkFeeSpecifications) / sizeof (struct NetworkFeeSpecification);

    // Currency Specification
    struct CurrencySpecification {
        char *networkId;
        char *currencyId;
        char *name;
        char *code;
        char *type;
        char *address;
        bool verified;
    } currencySpecifications[] = {
#define DEFINE_CURRENCY(networkId, currencyId, name, code, type, address, verified) \
{ networkId, currencyId, name, code, type, address, verified },
#include "WKConfig.h"
    };
    size_t NUMBER_OF_CURRENCIES = sizeof (currencySpecifications) / sizeof(struct CurrencySpecification);

    // Unit Specification
    struct UnitSpecification {
        char *currencyId;
        char *name;
        char *code;
        uint32_t decimals;
        char *symbol;
    } unitSpecifications[] = {
#define DEFINE_UNIT(currencyId, name, code, decimals, symbol) \
{ currencyId, name, code, decimals, symbol },
#include "WKConfig.h"
    };
    size_t NUMBER_OF_UNITS = sizeof (unitSpecifications) / sizeof (struct UnitSpecification);

    // Address Schemes
    struct AddressSchemeSpecification {
        char *networkId;
        WKAddressScheme defaultScheme;
        WKAddressScheme schemes[NUMBER_OF_ADDRESS_SCHEMES];
        size_t numberOfSchemes;
    } addressSchemeSpecs[] = {
#define VAR_SCHEMES_COUNT(...)    (sizeof((WKAddressScheme[]){__VA_ARGS__})/sizeof(WKAddressScheme))
#define DEFINE_ADDRESS_SCHEMES(networkId, defaultScheme, otherSchemes...) \
{ networkId, defaultScheme, { defaultScheme, otherSchemes }, 1 + VAR_SCHEMES_COUNT(otherSchemes) },
#include "WKConfig.h"
    };
    size_t NUMBER_OF_SCHEMES = sizeof (addressSchemeSpecs) / sizeof (struct AddressSchemeSpecification);

    // Sync Modes
    struct SyncModeSpecification {
        char *networkId;
        WKSyncMode defaultMode;
        WKSyncMode modes[NUMBER_OF_SYNC_MODES];
        size_t numberOfModes;
    } modeSpecs[] = {
#define VAR_MODES_COUNT(...)    (sizeof((WKSyncMode[]){__VA_ARGS__})/sizeof(WKSyncMode))
#define DEFINE_MODES(networkId, defaultMode, otherModes...) \
{ networkId, defaultMode, { defaultMode, otherModes }, 1 + VAR_SCHEMES_COUNT(otherModes) },
#include "WKConfig.h"
    };
    size_t NUMBER_OF_MODES = sizeof (modeSpecs) / sizeof (struct SyncModeSpecification);

    // Ensure that accounts are installed... because this ensures that GEN blockchains, such as
    // XRP and HBAR, have their handlers installed.  Calling here catches all paths in
    // Network; this call makes Network consistent with the comment on `wkAccountInstall()`.
    wkAccountInstall();

    assert (NULL != networksCount);
    size_t networksCountInstalled = 0;
    WKNetwork *networks = calloc (NUMBER_OF_NETWORKS, sizeof (WKNetwork));

    for (size_t networkIndex = 0; networkIndex < NUMBER_OF_NETWORKS; networkIndex++) {
        struct NetworkSpecification *networkSpec = &networkSpecifications[networkIndex];
        if (isMainnet != networkSpec->isMainnet) continue;

        const WKHandlers *handlers = wkHandlersLookup(networkSpec->type);
        // If the network handlers are NULL, then we'll skip that network.  This is only
        // for debugging purposes - as a way to avoid unimplemented currencies.
        if (NULL == handlers->network) break;

        WKCurrency nativeCurrency = NULL;

        // Create the native Currency
        for (size_t currencyIndex = 0; currencyIndex < NUMBER_OF_CURRENCIES; currencyIndex++) {
            struct CurrencySpecification *currencySpec = &currencySpecifications[currencyIndex];
            if (0 == strcmp (networkSpec->networkId, currencySpec->networkId) &&
                0 == strcmp ("native", currencySpec->type))
                nativeCurrency = wkCurrencyCreate (currencySpec->currencyId,
                                                       currencySpec->name,
                                                       currencySpec->code,
                                                       currencySpec->type,
                                                       currencySpec->address);
        }

        WKAddressScheme defaultAddressScheme;

        // Fill out the Address Schemes
        for (size_t schemeIndex = 0; schemeIndex < NUMBER_OF_SCHEMES; schemeIndex++) {
            struct AddressSchemeSpecification *schemeSpec = &addressSchemeSpecs[schemeIndex];
            if (0 == strcmp (networkSpec->networkId, schemeSpec->networkId))
                defaultAddressScheme = schemeSpec->defaultScheme;
        }

        WKSyncMode defaultSyncMode;

        // Fill out the sync modes
        for (size_t modeIndex = 0; modeIndex < NUMBER_OF_MODES; modeIndex++) {
            struct SyncModeSpecification *modeSpec = &modeSpecs[modeIndex];
            if (0 == strcmp (networkSpec->networkId, modeSpec->networkId))
                defaultSyncMode = modeSpec->defaultMode;
        }

        WKNetwork network = handlers->network->create (listener,
                                                             networkSpec->networkId,
                                                             networkSpec->name,
                                                             networkSpec->network,
                                                             networkSpec->isMainnet,
                                                             networkSpec->confirmationPeriodInSeconds,
                                                             defaultAddressScheme,
                                                             defaultSyncMode,
                                                             nativeCurrency);

        WKCurrency currency = NULL;

        BRArrayOf(WKUnit) units;
        array_new (units, 5);

        BRArrayOf(WKNetworkFee) fees;
        array_new (fees, 3);

        // Create the currency
        for (size_t currencyIndex = 0; currencyIndex < NUMBER_OF_CURRENCIES; currencyIndex++) {
            struct CurrencySpecification *currencySpec = &currencySpecifications[currencyIndex];
            if (0 == strcmp (networkSpec->networkId, currencySpec->networkId)) {
                currency = (0 == strcmp ("native", currencySpec->type)
                            ? wkCurrencyTake (nativeCurrency)
                            : wkCurrencyCreate (currencySpec->currencyId,
                                                    currencySpec->name,
                                                    currencySpec->code,
                                                    currencySpec->type,
                                                    currencySpec->address));

                WKUnit unitBase    = NULL;
                WKUnit unitDefault = NULL;

                // Create the units
                for (size_t unitIndex = 0; unitIndex < NUMBER_OF_UNITS; unitIndex++) {
                    struct UnitSpecification *unitSpec = &unitSpecifications[unitIndex];
                    if (0 == strcmp (currencySpec->currencyId, unitSpec->currencyId)) {
                        if (NULL == unitBase) {
                            assert (0 == unitSpec->decimals);
                            unitBase = wkUnitCreateAsBase (currency,
                                                               unitSpec->code,
                                                               unitSpec->name,
                                                               unitSpec->symbol);
                            array_add (units, wkUnitTake (unitBase));
                        }
                        else {
                            WKUnit unit = wkUnitCreate (currency,
                                                                  unitSpec->code,
                                                                  unitSpec->name,
                                                                  unitSpec->symbol,
                                                                  unitBase,
                                                                  unitSpec->decimals);
                            array_add (units, unit);

                            if (NULL == unitDefault || wkUnitGetBaseDecimalOffset(unit) > wkUnitGetBaseDecimalOffset(unitDefault)) {
                                if (NULL != unitDefault) wkUnitGive(unitDefault);
                                unitDefault = wkUnitTake(unit);
                            }
                        }
                    }
                }

                wkNetworkAddCurrency (network, currency, unitBase, unitDefault);

                for (size_t unitIndex = 0; unitIndex < array_count(units); unitIndex++) {
                    wkNetworkAddCurrencyUnit (network, currency, units[unitIndex]);
                    wkUnitGive (units[unitIndex]);
                }
                array_clear (units);

                wkUnitGive(unitBase);
                wkUnitGive(unitDefault);
                wkCurrencyGive(currency);
            }
        }
        wkCurrencyGive(nativeCurrency);
        nativeCurrency = NULL;

        // Create the Network Fees
        WKUnit feeUnit = wkNetworkGetUnitAsDefault (network, network->currency);
        for (size_t networkFeeIndex = 0; networkFeeIndex < NUMBER_OF_FEES; networkFeeIndex++) {
            struct NetworkFeeSpecification *networkFeeSpec = &networkFeeSpecifications[networkFeeIndex];
            if (0 == strcmp (networkSpec->networkId, networkFeeSpec->networkId)) {
                WKAmount pricePerCostFactor = wkAmountCreateString (networkFeeSpec->amount,
                                                                              WK_FALSE,
                                                                              feeUnit);
                WKNetworkFee fee = wkNetworkFeeCreate (networkFeeSpec->confirmationTimeInMilliseconds,
                                                                 pricePerCostFactor,
                                                                 feeUnit);
                array_add (fees, fee);

                wkAmountGive(pricePerCostFactor);
            }
        }
        wkUnitGive(feeUnit);

        wkNetworkSetNetworkFeesInternal (network, fees, array_count(fees), false);
        for (size_t index = 0; index < array_count(fees); index++)
            wkNetworkFeeGive (fees[index]);
        array_free(fees);

        // Fill out the Address Schemes
        for (size_t schemeIndex = 0; schemeIndex < NUMBER_OF_SCHEMES; schemeIndex++) {
            struct AddressSchemeSpecification *schemeSpec = &addressSchemeSpecs[schemeIndex];
            if (0 == strcmp (networkSpec->networkId, schemeSpec->networkId)) {
                for (size_t index = 0; index < schemeSpec->numberOfSchemes; index++)
                    if (network->defaultAddressScheme != schemeSpec->schemes[index])
                        wkNetworkAddSupportedAddressScheme(network, schemeSpec->schemes[index]);
            }
        }

        // Fill out the sync modes
        for (size_t modeIndex = 0; modeIndex < NUMBER_OF_MODES; modeIndex++) {
            struct SyncModeSpecification *modeSpec = &modeSpecs[modeIndex];
            if (0 == strcmp (networkSpec->networkId, modeSpec->networkId)) {
                for (size_t index = 0; index < modeSpec->numberOfModes; index++)
                    if (network->defaultSyncMode != modeSpec->modes[index])
                        wkNetworkAddSupportedSyncMode (network, modeSpec->modes[index]);
            }
        }

        array_free (units);

        wkNetworkSetConfirmationsUntilFinal (network, networkSpec->confirmations);
        wkNetworkSetHeight (network, networkSpec->height);

        networks[networksCountInstalled++] = network;

#if SHOW_BUILTIN_CURRENCIES
        printf ("== Network: %s, '%s'\n", network->uids, network->name);
        for (size_t ai = 0; ai < array_count(network->associations); ai++) {
            WKCurrencyAssociation a = network->associations[ai];
            printf ("    Currency: %s, '%s'\n", wkCurrencyGetUids(a.currency), wkCurrencyGetName(a.currency));
            printf ("    Base Unit: %s\n", wkUnitGetUids(a.baseUnit));
            printf ("    Default Unit: %s\n", wkUnitGetUids(a.defaultUnit));
            printf ("    Units:\n");
            for (size_t ui = 0; ui < array_count(a.units); ui++) {
                WKUnit u = a.units[ui];
                printf ("      %s, '%s', %5s\n", wkUnitGetUids (u), wkUnitGetName(u), wkUnitGetSymbol(u));
            }
            printf ("\n");
        }
#endif
    }

    *networksCount = networksCountInstalled;
    return networks;
}

extern WKNetwork
wkNetworkFindBuiltin (const char *uids,
                          bool isMainnet) {
    size_t networksCount = 0;
    WKNetwork *networks = wkNetworkInstallBuiltins (&networksCount,
                                                              wkListenerCreateNetworkListener (NULL, NULL),
                                                              isMainnet);
    WKNetwork network = NULL;

    for (size_t index = 0; index < networksCount; index++) {
        if (NULL == network && 0 == strcmp (uids, networks[index]->uids))
            network = wkNetworkTake (networks[index]);
        wkNetworkGive(networks[index]);
    }
    free (networks);

    return network;
}

extern WKNetworkType
wkNetworkGetTypeFromName (const char *name, WKBoolean *isMainnet) {
    struct NetworkSpecification {
        WKNetworkType type;
        char *networkId;
        char *name;
        char *network;
        bool isMainnet;
        uint64_t height;
        uint32_t confirmations;
        uint32_t confirmationPeriodInSeconds;
    } networkSpecifications[] = {
#define DEFINE_NETWORK(type, networkId, name, network, isMainnet, height, confirmations, confirmationPeriodInSeconds) \
{ type, networkId, name, network, isMainnet, height, confirmations, confirmationPeriodInSeconds },
#include "WKConfig.h"
        //        { NULL }
    };
    size_t NUMBER_OF_NETWORKS = sizeof (networkSpecifications) / sizeof (struct NetworkSpecification);

    for (size_t index = 0; index < NUMBER_OF_NETWORKS; index++) {
        if (0 == strcasecmp (name, networkSpecifications[index].networkId)) {
            if (NULL != isMainnet) *isMainnet = AS_WK_BOOLEAN(networkSpecifications[index].isMainnet);
            return networkSpecifications[index].type;
        }
    }
    return (WKNetworkType) WK_NETWORK_TYPE_UNKNOWN;
}

