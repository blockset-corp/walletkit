//
//  BRCryptoNetwork.c
//  BRCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <ctype.h>

#include "BRCryptoNetworkP.h"
#include "BRCryptoUnit.h"
#include "BRCryptoAddressP.h"
#include "BRCryptoAmountP.h"
#include "BRCryptoAccountP.h"
#include "BRCryptoHashP.h"

#include "BRCryptoHandlersP.h"

// If '1' then display a detailed list of the builting currencies for each network
#define SHOW_BUILTIN_CURRENCIES 0 // DEBUG

private_extern BRArrayOf(BRCryptoUnit)
cryptoUnitGiveAll (BRArrayOf(BRCryptoUnit) units);

/// MARK: - Network Canonical Type

extern const char *
cryptoBlockChainTypeGetCurrencyCode (BRCryptoBlockChainType type) {
    static const char *currencies[NUMBER_OF_NETWORK_TYPES] = {
        CRYPTO_NETWORK_CURRENCY_BTC,
        CRYPTO_NETWORK_CURRENCY_BCH,
        CRYPTO_NETWORK_CURRENCY_BSV,
        CRYPTO_NETWORK_CURRENCY_ETH,
        CRYPTO_NETWORK_CURRENCY_XRP,
        CRYPTO_NETWORK_CURRENCY_HBAR,
        CRYPTO_NETWORK_CURRENCY_XTZ,
        // "Stellar"
    };
    assert (type < NUMBER_OF_NETWORK_TYPES);
    return currencies[type];
}

/// MARK: - Network Fee

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoNetworkFee, cryptoNetworkFee)

extern BRCryptoNetworkFee
cryptoNetworkFeeCreate (uint64_t confirmationTimeInMilliseconds,
                        BRCryptoAmount pricePerCostFactor,
                        BRCryptoUnit   pricePerCostFactorUnit) {
    BRCryptoNetworkFee networkFee = calloc (1, sizeof (struct BRCryptoNetworkFeeRecord));

    networkFee->confirmationTimeInMilliseconds = confirmationTimeInMilliseconds;
    networkFee->pricePerCostFactor     = cryptoAmountTake (pricePerCostFactor);
    networkFee->pricePerCostFactorUnit = cryptoUnitTake (pricePerCostFactorUnit);
    networkFee->ref = CRYPTO_REF_ASSIGN(cryptoNetworkFeeRelease);

    return networkFee;
}

extern uint64_t
cryptoNetworkFeeGetConfirmationTimeInMilliseconds (BRCryptoNetworkFee networkFee) {
    return networkFee->confirmationTimeInMilliseconds;
}

extern BRCryptoAmount
cryptoNetworkFeeGetPricePerCostFactor (BRCryptoNetworkFee networkFee) {
    return cryptoAmountTake (networkFee->pricePerCostFactor);
}

extern BRCryptoUnit
cryptoNetworkFeeGetPricePerCostFactorUnit (BRCryptoNetworkFee networkFee) {
    return cryptoUnitTake (networkFee->pricePerCostFactorUnit);
}

extern BRCryptoBoolean
cryptoNetworkFeeEqual (BRCryptoNetworkFee nf1, BRCryptoNetworkFee nf2) {
    return AS_CRYPTO_BOOLEAN (nf1 == nf2 ||
                              (nf1->confirmationTimeInMilliseconds == nf2->confirmationTimeInMilliseconds) &&
                              CRYPTO_COMPARE_EQ == cryptoAmountCompare (nf1->pricePerCostFactor, nf2->pricePerCostFactor));
}

static void
cryptoNetworkFeeRelease (BRCryptoNetworkFee networkFee) {
    cryptoAmountGive (networkFee->pricePerCostFactor);
    cryptoUnitGive   (networkFee->pricePerCostFactorUnit);

    memset (networkFee, 0, sizeof(*networkFee));
    free (networkFee);
}

// MARK: - Crypto Association

static void
cryptoCurrencyAssociationRelease (BRCryptoCurrencyAssociation association) {
    cryptoCurrencyGive (association.currency);
    cryptoUnitGive (association.baseUnit);
    cryptoUnitGive (association.defaultUnit);
    cryptoUnitGiveAll (association.units);
    array_free (association.units);
}
/// MARK: - Network

#define CRYPTO_NETWORK_DEFAULT_CURRENCY_ASSOCIATIONS        (2)
#define CRYPTO_NETWORK_DEFAULT_FEES                         (3)
#define CRYPTO_NETWORK_DEFAULT_NETWORKS                     (5)

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoNetwork, cryptoNetwork)

extern BRCryptoNetwork
cryptoNetworkAllocAndInit (size_t sizeInBytes,
                           BRCryptoBlockChainType type,
                           BRCryptoNetworkListener listener,
                           const char *uids,
                           const char *name,
                           const char *desc,
                           bool isMainnet,
                           uint32_t confirmationPeriodInSeconds,
                           BRCryptoAddressScheme defaultAddressScheme,
                           BRCryptoSyncMode defaultSyncMode,
                           BRCryptoCurrency currencyNative,
                           BRCryptoNetworkCreateContext createContext,
                           BRCryptoNetworkCreateCallback createCallback) {
    assert (sizeInBytes >= sizeof (struct BRCryptoNetworkRecord));
    BRCryptoNetwork network = calloc (1, sizeInBytes);

    network->type = type;
    network->handlers = cryptoHandlersLookup(type)->network;
    network->sizeInBytes = sizeInBytes;

    network->listener = listener;

    network->uids = strdup (uids);
    network->name = strdup (name);
    network->desc = strdup (desc);

    network->isMainnet = isMainnet;
    network->currency  = cryptoCurrencyTake (currencyNative);
    network->height    = 0;

    array_new (network->associations, CRYPTO_NETWORK_DEFAULT_CURRENCY_ASSOCIATIONS);
    array_new (network->fees, CRYPTO_NETWORK_DEFAULT_FEES);

    network->confirmationPeriodInSeconds = confirmationPeriodInSeconds;

    network->defaultAddressScheme = defaultAddressScheme;
    array_new (network->addressSchemes, NUMBER_OF_ADDRESS_SCHEMES);
    array_add (network->addressSchemes, network->defaultAddressScheme);

    network->defaultSyncMode = defaultSyncMode;
    array_new (network->syncModes, NUMBER_OF_SYNC_MODES);
    array_add (network->syncModes, network->defaultSyncMode);

    network->verifiedBlockHash = NULL;

    network->ref = CRYPTO_REF_ASSIGN(cryptoNetworkRelease);

    pthread_mutex_init_brd (&network->lock, PTHREAD_MUTEX_RECURSIVE);

    if (NULL != createCallback) createCallback (createContext, network);

    cryptoNetworkGenerateEvent (network, (BRCryptoNetworkEvent) {
        CRYPTO_NETWORK_EVENT_CREATED
    });

    return network;
}

static void
cryptoNetworkRelease (BRCryptoNetwork network) {
    network->handlers->release (network);

    cryptoHashGive (network->verifiedBlockHash);

    array_free_all (network->associations, cryptoCurrencyAssociationRelease);

    for (size_t index = 0; index < array_count (network->fees); index++) {
        cryptoNetworkFeeGive (network->fees[index]);
    }
    array_free (network->fees);

    if (network->addressSchemes) array_free (network->addressSchemes);
    if (network->syncModes)      array_free (network->syncModes);

    free (network->desc);
    free (network->name);
    free (network->uids);
    if (NULL != network->currency) cryptoCurrencyGive (network->currency);
    pthread_mutex_destroy (&network->lock);

    memset (network, 0, network->sizeInBytes);
    free (network);
}

private_extern BRCryptoBlockChainType
cryptoNetworkGetType (BRCryptoNetwork network) {
    return network->type;
}

extern const char *
cryptoNetworkGetUids (BRCryptoNetwork network) {
    return network->uids;
}

extern const char *
cryptoNetworkGetName (BRCryptoNetwork network) {
    return network->name;
}

private_extern const char *
cryptoNetworkGetDesc (BRCryptoNetwork network) {
    return network->desc;
}

extern BRCryptoBoolean
cryptoNetworkIsMainnet (BRCryptoNetwork network) {
    return network->isMainnet;
}

private_extern uint32_t
cryptoNetworkGetConfirmationPeriodInSeconds (BRCryptoNetwork network) {
    return network->confirmationPeriodInSeconds;
}

extern BRCryptoBlockNumber
cryptoNetworkGetHeight (BRCryptoNetwork network) {
    pthread_mutex_lock (&network->lock);
    BRCryptoBlockNumber height = network->height;
    pthread_mutex_unlock (&network->lock);
    return height;
}

extern void
cryptoNetworkSetHeight (BRCryptoNetwork network,
                        BRCryptoBlockNumber height) {
    pthread_mutex_lock (&network->lock);
    network->height = height;
    pthread_mutex_unlock (&network->lock);
}

extern BRCryptoHash
cryptoNetworkGetVerifiedBlockHash (BRCryptoNetwork network) {
    return cryptoHashTake (network->verifiedBlockHash);
}

extern void
cryptoNetworkSetVerifiedBlockHash (BRCryptoNetwork network,
                                   BRCryptoHash verifiedBlockHash) {
    pthread_mutex_lock (&network->lock);
    cryptoHashGive (network->verifiedBlockHash);
    network->verifiedBlockHash = cryptoHashTake (verifiedBlockHash);
    pthread_mutex_unlock (&network->lock);
}

extern void
cryptoNetworkSetVerifiedBlockHashAsString (BRCryptoNetwork network,
                                           const char * blockHashString) {
    BRCryptoHash verifiedBlockHash = cryptoNetworkCreateHashFromString (network, blockHashString);
    cryptoNetworkSetVerifiedBlockHash (network, verifiedBlockHash);
    cryptoHashGive (verifiedBlockHash);
}

extern uint32_t
cryptoNetworkGetConfirmationsUntilFinal (BRCryptoNetwork network) {
    return network->confirmationsUntilFinal;
}

extern void
cryptoNetworkSetConfirmationsUntilFinal (BRCryptoNetwork network,
                                         uint32_t confirmationsUntilFinal) {
    network->confirmationsUntilFinal = confirmationsUntilFinal;
}

extern BRCryptoCurrency
cryptoNetworkGetCurrency (BRCryptoNetwork network) {
    pthread_mutex_lock (&network->lock);
    BRCryptoCurrency currency = cryptoCurrencyTake (network->currency);
    pthread_mutex_unlock (&network->lock);
    return currency;
}

extern const char *
cryptoNetworkGetCurrencyCode (BRCryptoNetwork network) {
    return cryptoCurrencyGetCode (network->currency);
}

extern size_t
cryptoNetworkGetCurrencyCount (BRCryptoNetwork network) {
    pthread_mutex_lock (&network->lock);
    size_t count = array_count (network->associations);
    pthread_mutex_unlock (&network->lock);
    return count;
}

extern BRCryptoCurrency
cryptoNetworkGetCurrencyAt (BRCryptoNetwork network,
                            size_t index) {
    pthread_mutex_lock (&network->lock);
    assert (index < array_count(network->associations));
    BRCryptoCurrency currency = cryptoCurrencyTake (network->associations[index].currency);
    pthread_mutex_unlock (&network->lock);
    return currency;
}

extern BRCryptoBoolean
cryptoNetworkHasCurrency (BRCryptoNetwork network,
                          BRCryptoCurrency currency) {
    BRCryptoBoolean r = CRYPTO_FALSE;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations) && CRYPTO_FALSE == r; index++) {
        r = cryptoCurrencyIsIdentical (network->associations[index].currency, currency);
    }
    pthread_mutex_unlock (&network->lock);
    return r;
}

extern BRCryptoCurrency
cryptoNetworkGetCurrencyForCode (BRCryptoNetwork network,
                                   const char *code) {
    BRCryptoCurrency currency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        if (0 == strcmp (code, cryptoCurrencyGetCode (network->associations[index].currency))) {
            currency = cryptoCurrencyTake (network->associations[index].currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return currency;
}

extern BRCryptoCurrency
cryptoNetworkGetCurrencyForUids (BRCryptoNetwork network,
                                 const char *uids) {
    BRCryptoCurrency currency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        if (0 == strcasecmp (uids, cryptoCurrencyGetUids (network->associations[index].currency))) {
            currency = cryptoCurrencyTake (network->associations[index].currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return currency;
}

extern BRCryptoCurrency
cryptoNetworkGetCurrencyForIssuer (BRCryptoNetwork network,
                                   const char *issuer) {
    BRCryptoCurrency currency = NULL;
    pthread_mutex_lock (&network->lock);
    for (size_t index = 0; index < array_count(network->associations); index++) {
        const char *i = cryptoCurrencyGetIssuer(network->associations[index].currency);
        if (NULL != i && 0 == strcasecmp (issuer, i)) {
            currency = cryptoCurrencyTake (network->associations[index].currency);
            break;
        }
    }
    pthread_mutex_unlock (&network->lock);
    return currency;
}

static BRCryptoCurrencyAssociation *
cryptoNetworkLookupCurrencyAssociation (BRCryptoNetwork network,
                                        BRCryptoCurrency currency) {
    // lock is not held for this static method; caller must hold it
    for (size_t index = 0; index < array_count(network->associations); index++) {
        if (CRYPTO_TRUE == cryptoCurrencyIsIdentical (currency, network->associations[index].currency)) {
            return &network->associations[index];
        }
    }
    return NULL;
}

static BRCryptoCurrencyAssociation *
cryptoNetworkLookupCurrencyAssociationByUids (BRCryptoNetwork network,
                                              const char *uids) {
    // lock is not held for this static method; caller must hold it
    for (size_t index = 0; index < array_count(network->associations); index++) {
        if (cryptoCurrencyHasUids (network->associations[index].currency, uids))
            return &network->associations[index];
    }
    return NULL;
}

extern BRCryptoUnit
cryptoNetworkGetUnitAsBase (BRCryptoNetwork network,
                            BRCryptoCurrency currency) {
    pthread_mutex_lock (&network->lock);
    currency = (NULL == currency ? network->currency : currency);
    BRCryptoCurrencyAssociation *association = cryptoNetworkLookupCurrencyAssociation (network, currency);
    BRCryptoUnit unit = NULL == association ? NULL : cryptoUnitTake (association->baseUnit);
    pthread_mutex_unlock (&network->lock);
    return unit;
}

extern BRCryptoUnit
cryptoNetworkGetUnitAsDefault (BRCryptoNetwork network,
                               BRCryptoCurrency currency) {
    pthread_mutex_lock (&network->lock);
    currency = (NULL == currency ? network->currency : currency);
    BRCryptoCurrencyAssociation *association = cryptoNetworkLookupCurrencyAssociation (network, currency);
    BRCryptoUnit unit = NULL == association ? NULL : cryptoUnitTake (association->defaultUnit);
    pthread_mutex_unlock (&network->lock);
    return unit;
}

extern size_t
cryptoNetworkGetUnitCount (BRCryptoNetwork network,
                           BRCryptoCurrency currency) {
    pthread_mutex_lock (&network->lock);
    currency = (NULL == currency ? network->currency : currency);
    BRCryptoCurrencyAssociation *association = cryptoNetworkLookupCurrencyAssociation (network, currency);
    size_t count = ((NULL == association || NULL == association->units)
                    ? 0
                    : array_count (association->units));
    pthread_mutex_unlock (&network->lock);
    return count;
}

extern BRCryptoUnit
cryptoNetworkGetUnitAt (BRCryptoNetwork network,
                        BRCryptoCurrency currency,
                        size_t index) {
    pthread_mutex_lock (&network->lock);
    currency = (NULL == currency ? network->currency : currency);
    BRCryptoCurrencyAssociation *association = cryptoNetworkLookupCurrencyAssociation (network, currency);
    BRCryptoUnit unit = ((NULL == association || NULL == association->units || index >= array_count(association->units))
                         ? NULL
                         : cryptoUnitTake (association->units[index]));
    pthread_mutex_unlock (&network->lock);
    return unit;
}

extern void
cryptoNetworkAddCurrency (BRCryptoNetwork network,
                          BRCryptoCurrency currency,
                          BRCryptoUnit baseUnit,
                          BRCryptoUnit defaultUnit) {
    BRCryptoCurrencyAssociation association = {
        cryptoCurrencyTake (currency),
        cryptoUnitTake (baseUnit),
        cryptoUnitTake (defaultUnit),
        NULL
    };

    pthread_mutex_lock (&network->lock);
    array_new (association.units, 2);
    array_add (network->associations, association);
    pthread_mutex_unlock (&network->lock);
}

extern void
cryptoNetworkAddCurrencyUnit (BRCryptoNetwork network,
                              BRCryptoCurrency currency,
                              BRCryptoUnit unit) {
    pthread_mutex_lock (&network->lock);
    BRCryptoCurrencyAssociation *association = cryptoNetworkLookupCurrencyAssociation (network, currency);
    if (NULL != association) array_add (association->units, cryptoUnitTake (unit));
    pthread_mutex_unlock (&network->lock);
}

// MARK: - Currency Association

private_extern void
cryptoNetworkAddCurrencyAssociationFromBundle (BRCryptoNetwork network,
                                               OwnershipKept BRCryptoClientCurrencyBundle bundle,
                                               BRCryptoBoolean needEvent) {

    pthread_mutex_lock (&network->lock);

    // Lookup an existing association for the bundle's id
    BRCryptoCurrencyAssociation *association = cryptoNetworkLookupCurrencyAssociationByUids (network, bundle->id);

    // If one exists; do not replace
    // TODO: Add an argument for `bool replace`
    if (NULL != association) {
        pthread_mutex_unlock (&network->lock);
        return;
    }

    BRCryptoCurrency currency = cryptoCurrencyCreate (bundle->id,
                                                      bundle->name,
                                                      bundle->code,
                                                      bundle->type,
                                                      bundle->address);

    BRArrayOf(BRCryptoUnit) units;
    array_new (units, array_count(bundle->denominations));

    // Find the base unit
    BRCryptoUnit baseUnit    = NULL;

    for (size_t index = 0; index < array_count (bundle->denominations); index++) {
        BRCryptoClientCurrencyDenominationBundle demBundle = bundle->denominations[index];
        if (0 == demBundle->decimals) {
            baseUnit = cryptoUnitCreateAsBase (currency, demBundle->code, demBundle->name, demBundle->symbol);
            break;
        }
    }

    if (NULL == baseUnit) {
        const char *code = cryptoCurrencyGetCode(currency);
        const char *name = cryptoCurrencyGetName(currency);

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

        baseUnit = cryptoUnitCreateAsBase (currency,
                                           unitCode,
                                           unitName,
                                           unitSymb);
    }
    array_add (units, baseUnit);

    for (size_t index = 0; index < array_count (bundle->denominations); index++) {
        BRCryptoClientCurrencyDenominationBundle demBundle = bundle->denominations[index];
        if (0 != demBundle->decimals) {
            BRCryptoUnit unit = cryptoUnitCreate (currency,
                                                  demBundle->code,
                                                  demBundle->name,
                                                  demBundle->symbol,
                                                  baseUnit,
                                                  demBundle->decimals);
            array_add (units, unit);
        }
    }

    // Find the default Unit - maximum decimals

    BRCryptoUnit defaultUnit = NULL;

    if (2 == array_count(units))
        defaultUnit = units[1];
    else {
        uint8_t decimals = 0;
        for (size_t index = 0; index < array_count(units); index++) {
            if (cryptoUnitGetBaseDecimalOffset(units[index]) > decimals) {
                defaultUnit = units[index];
                decimals = cryptoUnitGetBaseDecimalOffset (defaultUnit);
            }
        }
        if (NULL == defaultUnit) defaultUnit = baseUnit;
    }
    assert (NULL != defaultUnit);

    BRCryptoCurrencyAssociation newAssociation = {
        currency,
        baseUnit,
        defaultUnit,
        units
    };
    array_add (network->associations, newAssociation);
    pthread_mutex_unlock (&network->lock);

    if (CRYPTO_TRUE == needEvent)
        cryptoNetworkGenerateEvent (network, (BRCryptoNetworkEvent) {
            CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED
        });
}

private_extern void
cryptoNetworkAddCurrencyAssociationsFromBundles (BRCryptoNetwork network,
                                               OwnershipKept BRArrayOf(BRCryptoClientCurrencyBundle) bundles) {
    for (size_t index = 0; index < array_count(bundles); index++)
        cryptoNetworkAddCurrencyAssociationFromBundle (network, bundles[index], CRYPTO_FALSE);

    cryptoNetworkGenerateEvent (network, (BRCryptoNetworkEvent) {
        CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED
    });
}

// MARK: - Network Fees

extern void
cryptoNetworkAddNetworkFee (BRCryptoNetwork network,
                            BRCryptoNetworkFee fee) {
    pthread_mutex_lock (&network->lock);
    array_add (network->fees, cryptoNetworkFeeTake (fee));
    pthread_mutex_unlock (&network->lock);
}

static void
cryptoNetworkSetNetworkFeesInternal (BRCryptoNetwork network,
                                     const BRCryptoNetworkFee *fees,
                                     size_t count,
                                     bool needEvent) {
    assert (0 != count);
    pthread_mutex_lock (&network->lock);
    array_apply (network->fees, cryptoNetworkFeeGive);
    array_clear (network->fees);
    for (size_t idx = 0; idx < count; idx++) {
        array_add (network->fees, cryptoNetworkFeeTake (fees[idx]));
    }

    if (needEvent)
        cryptoListenerGenerateNetworkEvent (&network->listener, network, (BRCryptoNetworkEvent) {
            CRYPTO_NETWORK_EVENT_FEES_UPDATED
        });
    pthread_mutex_unlock (&network->lock);
}

extern void
cryptoNetworkSetNetworkFees (BRCryptoNetwork network,
                             const BRCryptoNetworkFee *fees,
                             size_t count) {
    cryptoNetworkSetNetworkFeesInternal (network, fees, count, true);
}

extern BRCryptoNetworkFee *
cryptoNetworkGetNetworkFees (BRCryptoNetwork network,
                             size_t *count) {
    pthread_mutex_lock (&network->lock);
    *count = array_count (network->fees);
    BRCryptoNetworkFee *fees = NULL;
    if (0 != *count) {
        fees = calloc (*count, sizeof(BRCryptoNetworkFee));
        for (size_t index = 0; index < *count; index++) {
            fees[index] = cryptoNetworkFeeTake(network->fees[index]);
        }
    }
    pthread_mutex_unlock (&network->lock);
    return fees;
}

// MARK: - Address Scheme

extern BRCryptoAddressScheme
cryptoNetworkGetDefaultAddressScheme (BRCryptoNetwork network) {
    assert (NULL != network->addressSchemes);
    return network->defaultAddressScheme;
}

static void
cryptoNetworkAddSupportedAddressScheme (BRCryptoNetwork network,
                                        BRCryptoAddressScheme scheme) {
    array_add (network->addressSchemes, scheme);
}

extern const BRCryptoAddressScheme *
cryptoNetworkGetSupportedAddressSchemes (BRCryptoNetwork network,
                                         BRCryptoCount *count) {
    assert (NULL != network->addressSchemes);
    assert (NULL != count);
    *count = array_count(network->addressSchemes);
    return network->addressSchemes;
}

extern BRCryptoBoolean
cryptoNetworkSupportsAddressScheme (BRCryptoNetwork network,
                                    BRCryptoAddressScheme scheme) {
    assert (NULL != network->addressSchemes);
    for (size_t index = 0; index < array_count (network->addressSchemes); index++)
        if (scheme == network->addressSchemes[index])
            return CRYPTO_TRUE;
    return CRYPTO_FALSE;
}

// MARK: - Address

extern BRCryptoAddress
cryptoNetworkCreateAddress (BRCryptoNetwork network,
                            const char *address) {
    return network->handlers->createAddress (network, address);
}

// MARK: - Sync Mode

extern BRCryptoSyncMode
cryptoNetworkGetDefaultSyncMode (BRCryptoNetwork network) {
    assert (NULL != network->syncModes);
    return network->defaultSyncMode;
}

static void
cryptoNetworkAddSupportedSyncMode (BRCryptoNetwork network,
                                   BRCryptoSyncMode scheme) {
    array_add (network->syncModes, scheme);
}

extern const BRCryptoSyncMode *
cryptoNetworkGetSupportedSyncModes (BRCryptoNetwork network,
                                    BRCryptoCount *count) {
    assert (NULL != network->syncModes);
    assert (NULL != count);
    *count = array_count(network->syncModes);
    return network->syncModes;
}

extern BRCryptoBoolean
cryptoNetworkSupportsSyncMode (BRCryptoNetwork network,
                               BRCryptoSyncMode mode) {
    assert (NULL != network->syncModes);
    for (size_t index = 0; index < array_count (network->syncModes); index++)
        if (mode == network->syncModes[index])
            return CRYPTO_TRUE;
    return CRYPTO_FALSE;
}

extern BRCryptoBoolean
cryptoNetworkRequiresMigration (BRCryptoNetwork network) {
    switch (network->type) {
        case CRYPTO_NETWORK_TYPE_BTC:
        case CRYPTO_NETWORK_TYPE_BCH:
            return CRYPTO_TRUE;
        default:
            return CRYPTO_FALSE;
    }
}

extern BRCryptoBoolean
cryptoNetworkIsAccountInitialized (BRCryptoNetwork network,
                                   BRCryptoAccount account) {
    return network->handlers->isAccountInitialized (network, account);
}


extern uint8_t *
cryptoNetworkGetAccountInitializationData (BRCryptoNetwork network,
                                           BRCryptoAccount account,
                                           size_t *bytesCount) {
    return network->handlers->getAccountInitializationData (network, account, bytesCount);
}

extern void
cryptoNetworkInitializeAccount (BRCryptoNetwork network,
                                BRCryptoAccount account,
                                const uint8_t *bytes,
                                size_t bytesCount) {
    network->handlers->initializeAccount (network, account, bytes, bytesCount);
}

private_extern BRCryptoBlockChainType
cryptoNetworkGetBlockChainType (BRCryptoNetwork network) {
    return network->type;
}

private_extern BRCryptoBlockNumber
cryptoNetworkGetBlockNumberAtOrBeforeTimestamp (BRCryptoNetwork network,
                                                BRCryptoTimestamp timestamp) {
    return network->handlers->getBlockNumberAtOrBeforeTimestamp (network, timestamp);
}

private_extern BRCryptoHash
cryptoNetworkCreateHashFromString (BRCryptoNetwork network,
                                   const char *string) {
    return network->handlers->createHashFromString (network, string);
}

private_extern OwnershipGiven char *
cryptoNetworkEncodeHash (BRCryptoHash hash) {
    const BRCryptoHandlers *handlers = cryptoHandlersLookup (hash->type);
    return handlers->network->encodeHash (hash);
}

extern const char *
cryptoNetworkEventTypeString (BRCryptoNetworkEventType type) {
    static const char *names[] = {
        "CRYPTO_NETWORK_EVENT_CREATED",
        "CRYPTO_NETWORK_EVENT_FEES_UPDATED",
        "CRYPTO_NETWORK_EVENT_DELETED"
    };
    return names [type];
}

// MARK: - Network Defaults

extern BRCryptoNetwork *
cryptoNetworkInstallBuiltins (BRCryptoCount *networksCount,
                              BRCryptoNetworkListener listener,
                              bool isMainnet) {
    // Network Specification
    struct NetworkSpecification {
        BRCryptoBlockChainType type;
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
#include "BRCryptoConfig.h"
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
#include "BRCryptoConfig.h"
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
#include "BRCryptoConfig.h"
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
#include "BRCryptoConfig.h"
    };
    size_t NUMBER_OF_UNITS = sizeof (unitSpecifications) / sizeof (struct UnitSpecification);

    // Address Schemes
    struct AddressSchemeSpecification {
        char *networkId;
        BRCryptoAddressScheme defaultScheme;
        BRCryptoAddressScheme schemes[NUMBER_OF_ADDRESS_SCHEMES];
        size_t numberOfSchemes;
    } addressSchemeSpecs[] = {
#define VAR_SCHEMES_COUNT(...)    (sizeof((BRCryptoAddressScheme[]){__VA_ARGS__})/sizeof(BRCryptoAddressScheme))
#define DEFINE_ADDRESS_SCHEMES(networkId, defaultScheme, otherSchemes...) \
{ networkId, defaultScheme, { defaultScheme, otherSchemes }, 1 + VAR_SCHEMES_COUNT(otherSchemes) },
#include "BRCryptoConfig.h"
    };
    size_t NUMBER_OF_SCHEMES = sizeof (addressSchemeSpecs) / sizeof (struct AddressSchemeSpecification);

    // Sync Modes
    struct SyncModeSpecification {
        char *networkId;
        BRCryptoSyncMode defaultMode;
        BRCryptoSyncMode modes[NUMBER_OF_SYNC_MODES];
        size_t numberOfModes;
    } modeSpecs[] = {
#define VAR_MODES_COUNT(...)    (sizeof((BRCryptoSyncMode[]){__VA_ARGS__})/sizeof(BRCryptoSyncMode))
#define DEFINE_MODES(networkId, defaultMode, otherModes...) \
{ networkId, defaultMode, { defaultMode, otherModes }, 1 + VAR_SCHEMES_COUNT(otherModes) },
#include "BRCryptoConfig.h"
    };
    size_t NUMBER_OF_MODES = sizeof (modeSpecs) / sizeof (struct SyncModeSpecification);

    // Ensure that accounts are installed... because this ensures that GEN blockchains, such as
    // XRP and HBAR, have their handlers installed.  Calling here catches all paths in
    // Network; this call makes Network consistent with the comment on `cryptoAccountInstall()`.
    cryptoAccountInstall();

    assert (NULL != networksCount);
    size_t networksCountInstalled = 0;
    BRCryptoNetwork *networks = calloc (NUMBER_OF_NETWORKS, sizeof (BRCryptoNetwork));

    for (size_t networkIndex = 0; networkIndex < NUMBER_OF_NETWORKS; networkIndex++) {
        struct NetworkSpecification *networkSpec = &networkSpecifications[networkIndex];
        if (isMainnet != networkSpec->isMainnet) continue;

        const BRCryptoHandlers *handlers = cryptoHandlersLookup(networkSpec->type);
        // If the network handlers are NULL, then we'll skip that network.  This is only
        // for debugging purposes - as a way to avoid unimplemented currencies.
        if (NULL == handlers->network) break;

        BRCryptoCurrency nativeCurrency = NULL;

        // Create the native Currency
        for (size_t currencyIndex = 0; currencyIndex < NUMBER_OF_CURRENCIES; currencyIndex++) {
            struct CurrencySpecification *currencySpec = &currencySpecifications[currencyIndex];
            if (0 == strcmp (networkSpec->networkId, currencySpec->networkId) &&
                0 == strcmp ("native", currencySpec->type))
                nativeCurrency = cryptoCurrencyCreate (currencySpec->currencyId,
                                                       currencySpec->name,
                                                       currencySpec->code,
                                                       currencySpec->type,
                                                       currencySpec->address);
        }

        BRCryptoAddressScheme defaultAddressScheme;

        // Fill out the Address Schemes
        for (size_t schemeIndex = 0; schemeIndex < NUMBER_OF_SCHEMES; schemeIndex++) {
            struct AddressSchemeSpecification *schemeSpec = &addressSchemeSpecs[schemeIndex];
            if (0 == strcmp (networkSpec->networkId, schemeSpec->networkId))
                defaultAddressScheme = schemeSpec->defaultScheme;
        }

        BRCryptoSyncMode defaultSyncMode;

        // Fill out the sync modes
        for (size_t modeIndex = 0; modeIndex < NUMBER_OF_MODES; modeIndex++) {
            struct SyncModeSpecification *modeSpec = &modeSpecs[modeIndex];
            if (0 == strcmp (networkSpec->networkId, modeSpec->networkId))
                defaultSyncMode = modeSpec->defaultMode;
        }

        BRCryptoNetwork network = handlers->network->create (listener,
                                                             networkSpec->networkId,
                                                             networkSpec->name,
                                                             networkSpec->network,
                                                             networkSpec->isMainnet,
                                                             networkSpec->confirmationPeriodInSeconds,
                                                             defaultAddressScheme,
                                                             defaultSyncMode,
                                                             nativeCurrency);

        BRCryptoCurrency currency = NULL;

        BRArrayOf(BRCryptoUnit) units;
        array_new (units, 5);

        BRArrayOf(BRCryptoNetworkFee) fees;
        array_new (fees, 3);

        // Create the currency
        for (size_t currencyIndex = 0; currencyIndex < NUMBER_OF_CURRENCIES; currencyIndex++) {
            struct CurrencySpecification *currencySpec = &currencySpecifications[currencyIndex];
            if (0 == strcmp (networkSpec->networkId, currencySpec->networkId)) {
                currency = (0 == strcmp ("native", currencySpec->type)
                            ? cryptoCurrencyTake (nativeCurrency)
                            : cryptoCurrencyCreate (currencySpec->currencyId,
                                                    currencySpec->name,
                                                    currencySpec->code,
                                                    currencySpec->type,
                                                    currencySpec->address));

                BRCryptoUnit unitBase    = NULL;
                BRCryptoUnit unitDefault = NULL;

                // Create the units
                for (size_t unitIndex = 0; unitIndex < NUMBER_OF_UNITS; unitIndex++) {
                    struct UnitSpecification *unitSpec = &unitSpecifications[unitIndex];
                    if (0 == strcmp (currencySpec->currencyId, unitSpec->currencyId)) {
                        if (NULL == unitBase) {
                            assert (0 == unitSpec->decimals);
                            unitBase = cryptoUnitCreateAsBase (currency,
                                                               unitSpec->code,
                                                               unitSpec->name,
                                                               unitSpec->symbol);
                            array_add (units, cryptoUnitTake (unitBase));
                        }
                        else {
                            BRCryptoUnit unit = cryptoUnitCreate (currency,
                                                                  unitSpec->code,
                                                                  unitSpec->name,
                                                                  unitSpec->symbol,
                                                                  unitBase,
                                                                  unitSpec->decimals);
                            array_add (units, unit);

                            if (NULL == unitDefault || cryptoUnitGetBaseDecimalOffset(unit) > cryptoUnitGetBaseDecimalOffset(unitDefault)) {
                                if (NULL != unitDefault) cryptoUnitGive(unitDefault);
                                unitDefault = cryptoUnitTake(unit);
                            }
                        }
                    }
                }

                cryptoNetworkAddCurrency (network, currency, unitBase, unitDefault);

                for (size_t unitIndex = 0; unitIndex < array_count(units); unitIndex++) {
                    cryptoNetworkAddCurrencyUnit (network, currency, units[unitIndex]);
                    cryptoUnitGive (units[unitIndex]);
                }
                array_clear (units);

                cryptoUnitGive(unitBase);
                cryptoUnitGive(unitDefault);
                cryptoCurrencyGive(currency);
            }
        }
        cryptoCurrencyGive(nativeCurrency);
        nativeCurrency = NULL;

        // Create the Network Fees
        BRCryptoUnit feeUnit = cryptoNetworkGetUnitAsDefault (network, network->currency);
        for (size_t networkFeeIndex = 0; networkFeeIndex < NUMBER_OF_FEES; networkFeeIndex++) {
            struct NetworkFeeSpecification *networkFeeSpec = &networkFeeSpecifications[networkFeeIndex];
            if (0 == strcmp (networkSpec->networkId, networkFeeSpec->networkId)) {
                BRCryptoAmount pricePerCostFactor = cryptoAmountCreateString (networkFeeSpec->amount,
                                                                              CRYPTO_FALSE,
                                                                              feeUnit);
                BRCryptoNetworkFee fee = cryptoNetworkFeeCreate (networkFeeSpec->confirmationTimeInMilliseconds,
                                                                 pricePerCostFactor,
                                                                 feeUnit);
                array_add (fees, fee);

                cryptoAmountGive(pricePerCostFactor);
            }
        }
        cryptoUnitGive(feeUnit);

        cryptoNetworkSetNetworkFeesInternal (network, fees, array_count(fees), false);
        for (size_t index = 0; index < array_count(fees); index++)
            cryptoNetworkFeeGive (fees[index]);
        array_free(fees);

        // Fill out the Address Schemes
        for (size_t schemeIndex = 0; schemeIndex < NUMBER_OF_SCHEMES; schemeIndex++) {
            struct AddressSchemeSpecification *schemeSpec = &addressSchemeSpecs[schemeIndex];
            if (0 == strcmp (networkSpec->networkId, schemeSpec->networkId)) {
                for (size_t index = 0; index < schemeSpec->numberOfSchemes; index++)
                    if (network->defaultAddressScheme != schemeSpec->schemes[index])
                        cryptoNetworkAddSupportedAddressScheme(network, schemeSpec->schemes[index]);
            }
        }

        // Fill out the sync modes
        for (size_t modeIndex = 0; modeIndex < NUMBER_OF_MODES; modeIndex++) {
            struct SyncModeSpecification *modeSpec = &modeSpecs[modeIndex];
            if (0 == strcmp (networkSpec->networkId, modeSpec->networkId)) {
                for (size_t index = 0; index < modeSpec->numberOfModes; index++)
                    if (network->defaultSyncMode != modeSpec->modes[index])
                        cryptoNetworkAddSupportedSyncMode (network, modeSpec->modes[index]);
            }
        }

        array_free (units);

        cryptoNetworkSetConfirmationsUntilFinal (network, networkSpec->confirmations);
        cryptoNetworkSetHeight (network, networkSpec->height);

        networks[networksCountInstalled++] = network;

#if SHOW_BUILTIN_CURRENCIES
        printf ("== Network: %s, '%s'\n", network->uids, network->name);
        for (size_t ai = 0; ai < array_count(network->associations); ai++) {
            BRCryptoCurrencyAssociation a = network->associations[ai];
            printf ("    Currency: %s, '%s'\n", cryptoCurrencyGetUids(a.currency), cryptoCurrencyGetName(a.currency));
            printf ("    Base Unit: %s\n", cryptoUnitGetUids(a.baseUnit));
            printf ("    Default Unit: %s\n", cryptoUnitGetUids(a.defaultUnit));
            printf ("    Units:\n");
            for (size_t ui = 0; ui < array_count(a.units); ui++) {
                BRCryptoUnit u = a.units[ui];
                printf ("      %s, '%s', %5s\n", cryptoUnitGetUids (u), cryptoUnitGetName(u), cryptoUnitGetSymbol(u));
            }
            printf ("\n");
        }
#endif
    }

    *networksCount = networksCountInstalled;
    return networks;
}

extern BRCryptoNetwork
cryptoNetworkFindBuiltin (const char *uids,
                          bool isMainnet) {
    size_t networksCount = 0;
    BRCryptoNetwork *networks = cryptoNetworkInstallBuiltins (&networksCount,
                                                              cryptoListenerCreateNetworkListener (NULL, NULL),
                                                              isMainnet);
    BRCryptoNetwork network = NULL;

    for (size_t index = 0; index < networksCount; index++) {
        if (NULL == network && 0 == strcmp (uids, networks[index]->uids))
            network = cryptoNetworkTake (networks[index]);
        cryptoNetworkGive(networks[index]);
    }
    free (networks);

    return network;
}

extern BRCryptoBlockChainType
cryptoNetworkGetTypeFromName (const char *name, BRCryptoBoolean *isMainnet) {
    struct NetworkSpecification {
        BRCryptoBlockChainType type;
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
#include "BRCryptoConfig.h"
        //        { NULL }
    };
    size_t NUMBER_OF_NETWORKS = sizeof (networkSpecifications) / sizeof (struct NetworkSpecification);

    for (size_t index = 0; index < NUMBER_OF_NETWORKS; index++) {
        if (0 == strcasecmp (name, networkSpecifications[index].networkId)) {
            if (NULL != isMainnet) *isMainnet = AS_CRYPTO_BOOLEAN(networkSpecifications[index].isMainnet);
            return networkSpecifications[index].type;
        }
    }
    return (BRCryptoBlockChainType) CRYPTO_NETWORK_TYPE_UNKNOWN;
}

