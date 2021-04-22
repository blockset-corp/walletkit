//
//  BRCryptoNetwork.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoNetwork_h
#define BRCryptoNetwork_h

#include "BRCryptoAccount.h"
#include "BRCryptoAddress.h"
#include "BRCryptoAmount.h"
#include "BRCryptoSync.h"
#include "BRCryptoListener.h"
#include "BRCryptoHash.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - (Network) Address Scheme

/**
 * An enumeration of possible network address schemes.
 */
typedef enum {
    CRYPTO_ADDRESS_SCHEME_BTC_LEGACY,
    CRYPTO_ADDRESS_SCHEME_BTC_SEGWIT,
    CRYPTO_ADDRESS_SCHEME_NATIVE,          // Default for Currency
} BRCryptoAddressScheme;

#define NUMBER_OF_ADDRESS_SCHEMES   (1 + CRYPTO_ADDRESS_SCHEME_NATIVE)

/**
 * A NetworkFee represents the possible 'pricePerCostFactors' and the price's corresponding
 * expected confirmation time.
 *
 * @discussion Generally to achieve a shorter confimation time, one must pay a higher price.
 * That is, price is a measure of miner priority.
 */
typedef struct BRCryptoNetworkFeeRecord *BRCryptoNetworkFee;

/**
 * Create a NetworkFee
 */
extern BRCryptoNetworkFee
cryptoNetworkFeeCreate (uint64_t confirmationTimeInMilliseconds,
                        BRCryptoAmount pricePerCostFactor,
                        BRCryptoUnit   pricePerCostFactorUnit);

/**
 * The estimated time to confirm a transfer for this network fee
 *
 * @param networkFee the network fee
 *
 * @return time in milliseconds
 */
extern uint64_t
cryptoNetworkFeeGetConfirmationTimeInMilliseconds (BRCryptoNetworkFee networkFee);

/**
 * Get the network fee's pricePerCostFactor
 */
extern BRCryptoAmount
cryptoNetworkFeeGetPricePerCostFactor (BRCryptoNetworkFee networkFee);

/**
 * Get the network fee's pricePerCostFactor unit
 */
extern BRCryptoUnit
cryptoNetworkFeeGetPricePerCostFactorUnit (BRCryptoNetworkFee networkFee);

/**
 * Check of two network fee's are equal.
 */
extern BRCryptoBoolean
cryptoNetworkFeeEqual (BRCryptoNetworkFee nf1, BRCryptoNetworkFee nf2);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoNetworkFee, cryptoNetworkFee);


// MARK: - Network

/**
 * Get the network's unique-identifier.  All networks, whether mainnet or testnet, will have
 * a globally unique identifier.
 */
extern const char *
cryptoNetworkGetUids (BRCryptoNetwork network);

/**
 * Get the network's name.  For bitcoin this is "Bitcoin"; for ethereum this is "Ethereum"
 */
extern const char *
cryptoNetworkGetName (BRCryptoNetwork network);

/**
 * Check of the network is a mainnet network.
 */
extern BRCryptoBoolean
cryptoNetworkIsMainnet (BRCryptoNetwork network);

/**
 * Get the network's type.  For bitcoin the type is `CRYPTO_NETWORK_TYPE_BTC`.
 */
extern BRCryptoNetworkType
cryptoNetworkGetType (BRCryptoNetwork network);

/**
 * Return the Blockchain type the network with `name` or CRYPTO_NETWORK_TYPE_UNKNOWN if
 * there is no network with `name`.
 *
 * @param name the name
 * @param isMainnet filled with true if `name` is for mainnet; false otherwise.
 */
extern BRCryptoNetworkType
cryptoNetworkGetTypeFromName (const char *name, BRCryptoBoolean *isMainnet);

/**
 * Returns the network's currency.  This is typically (always?) the currency used to pay
 * for network fees.
 
 @param network The network
 @return the network's currency w/ an incremented reference count (aka 'taken')
 */
extern BRCryptoCurrency
cryptoNetworkGetCurrency (BRCryptoNetwork network);

/**
 * Add `currency`, with the provided base and default units, to the network.
 */
extern void
cryptoNetworkAddCurrency (BRCryptoNetwork network,
                          BRCryptoCurrency currency,
                          BRCryptoUnit baseUnit,
                          BRCryptoUnit defaultUnit);

/**
 * Get the code for the network's native currency.
 */
extern const char *
cryptoNetworkGetCurrencyCode (BRCryptoNetwork network);

/**
 * Returns the currency's default unit or NULL
 *
 * @param network the network
 * @param currency the currency or NULL for the network's currency.
 *
 * @return the currency's default unit or NULL w/ an incremented reference count (aka 'taken')
 */
extern BRCryptoUnit
cryptoNetworkGetUnitAsDefault (BRCryptoNetwork network,
                               BRCryptoCurrency currency);

/**
 * Returns the currency's base unit or NULL
 *
 * @param network the network
 * @param currency the currency or NULL for the network's currency.
 *
 * @return the currency's base unit or NULL w/ an incremented reference count (aka 'taken')
 */
extern BRCryptoUnit
cryptoNetworkGetUnitAsBase (BRCryptoNetwork network,
                            BRCryptoCurrency currency);

/**
 * Add `unit` to the known units for `currency` on the network.
 */
extern void
cryptoNetworkAddCurrencyUnit (BRCryptoNetwork network,
                              BRCryptoCurrency currency,
                              BRCryptoUnit unit);

/**
 * Get the network's height.  The height changes as the blockchain is extended.
 */
extern BRCryptoBlockNumber
cryptoNetworkGetHeight (BRCryptoNetwork network);

/**
 * Set the network's height.
 */
extern void
cryptoNetworkSetHeight (BRCryptoNetwork network,
                        BRCryptoBlockNumber height);

/**
 * Get the network's verified block hash.
 */
extern BRCryptoHash
cryptoNetworkGetVerifiedBlockHash (BRCryptoNetwork network);

/**
 * Set the network's verified block hash.
 */
extern void
cryptoNetworkSetVerifiedBlockHash (BRCryptoNetwork network,
                                   BRCryptoHash verifiedBlockHash);

/**
 * Set the network's verified block hash from the string encoding of the hash.
 */
extern void
cryptoNetworkSetVerifiedBlockHashAsString (BRCryptoNetwork network,
                                           const char * verifiedBlockHashString);

/**
 * Get the network's count of blocks needed to confirm a transaction.  For Bitcoin this is six.
 */
extern uint32_t
cryptoNetworkGetConfirmationsUntilFinal (BRCryptoNetwork network);

/**
 * Returns the number of network currencies.  This is the index exclusive limit to be used
 * in `cryptoNetworkGetCurrencyAt()`.
 *
 * @param network the network
 *
 * @return number of network currencies.
 */
extern size_t
cryptoNetworkGetCurrencyCount (BRCryptoNetwork network);

/**
 * Returns the network's currency at `index`.  The index must satisfy [0, count) otherwise
 * an assertion is signaled.
 *
 * @param network the network
 * @param index the desired currency index
 *
 * @return The currency w/ an incremented reference count (aka 'taken')
 */
extern BRCryptoCurrency
cryptoNetworkGetCurrencyAt (BRCryptoNetwork network,
                            size_t index);

/**
 * Return 'TRUE' is `network` has `currency`.
 *
 * @param network the network
 *@param currency the currency
 *
 *@return CRYPTO_TRUE if `network` has `currency`.
 */
extern BRCryptoBoolean
cryptoNetworkHasCurrency (BRCryptoNetwork network,
                          BRCryptoCurrency currency);

/**
 * Returns the network's currency with `symbol` or NULL.
 *
 * @param network the network
 * @param index the desired currency's symbol
 *
 * @return The currency w/ an incremented reference count (aka 'taken')
 */
extern BRCryptoCurrency
cryptoNetworkGetCurrencyForCode (BRCryptoNetwork network,
                                 const char *code);

/**
 * Get the network's currency with `uids`; otherwise `NULL`
 */
extern BRCryptoCurrency
cryptoNetworkGetCurrencyForUids (BRCryptoNetwork network,
                                 const char *uids);

/**
 * Get the network's currency with `issuer`; otherwise `NULL`
 */
extern BRCryptoCurrency
cryptoNetworkGetCurrencyForIssuer (BRCryptoNetwork network,
                                   const char *issuer);

/**
 * Returns the number of units for network's `currency`.  This is the index exclusive limit to
 * be used in `cryptoNetworkGetUnitAt()`.
 *
 * @param network the network
 * @param currency the currency or NULL for the network's currency.
 *
 * @return the number of units for `currency`
 */
extern size_t
cryptoNetworkGetUnitCount (BRCryptoNetwork network,
                           BRCryptoCurrency currency);

/**
 * Returns the currency's unit at `index`.  The index must satisfy [0, count) otherwise an
 * assertion is signaled.
 *
 * @param network the network
 * @param currency the currency or NULL for the network's currency.
 * @param index the desired unit's index
 *
 * @return the currency unit w/ an incremented reference count (aka 'taken')
 */
extern BRCryptoUnit
cryptoNetworkGetUnitAt (BRCryptoNetwork network,
                        BRCryptoCurrency currency,
                        size_t index);

/**
 * Get the network's count of network fees.
 */
extern size_t
cryptoNetworkGetNetworkFeeCount (BRCryptoNetwork network);

/**
 * Get the network fee at `index` from among the network's fees
 */
extern BRCryptoNetworkFee
cryptoNetworkGetNetworkFeeAt (BRCryptoNetwork network,
                              size_t index);

/*
 * Get all the network's fee and fill `count` with the number of fees.
 */
extern BRCryptoNetworkFee *
cryptoNetworkGetNetworkFees (BRCryptoNetwork network,
                             size_t *count);

/**
 * Set the network's fee.
 */
extern void
cryptoNetworkSetNetworkFees (BRCryptoNetwork network,
                             const BRCryptoNetworkFee *fees,
                             size_t count);

/**
 * Add `fee` to the network's fees.
 */
extern void
cryptoNetworkAddNetworkFee (BRCryptoNetwork network,
                            BRCryptoNetworkFee fee);

// MARK: - Address Scheme

/**
 * Get the network's default address scheme.
 */
extern BRCryptoAddressScheme
cryptoNetworkGetDefaultAddressScheme (BRCryptoNetwork network);

/**
 * Get a newly allocated array of the network's address schemes.  Fill `count` with the number
 * of schemes.
 */
extern const BRCryptoAddressScheme *
cryptoNetworkGetSupportedAddressSchemes (BRCryptoNetwork network,
                                         BRCryptoCount *count);

/**
 * Check if network supports scheme
 */
extern BRCryptoBoolean
cryptoNetworkSupportsAddressScheme (BRCryptoNetwork network,
                                    BRCryptoAddressScheme scheme);

// MARK: - Address

/**
 * Create a network address from the string representation of in `address`
 */
extern BRCryptoAddress
cryptoNetworkCreateAddress (BRCryptoNetwork network,
                            const char *address);

// MARK: - Sync Mode

/**
 * Get the network's default sync-mode
 */
extern BRCryptoSyncMode
cryptoNetworkGetDefaultSyncMode (BRCryptoNetwork network);

/**
 * Get a newly allocated array of the network's sync modes.  Fill `count` with the number of
 * sync modes.
 */
extern const BRCryptoSyncMode *
cryptoNetworkGetSupportedSyncModes (BRCryptoNetwork network,
                                    BRCryptoCount *count);

/**
 * Check if network support `mode`
 */
extern BRCryptoBoolean
cryptoNetworkSupportsSyncMode (BRCryptoNetwork network,
                               BRCryptoSyncMode mode);

/**
 * Check if network requires migration (of persistent storage).
 */
extern BRCryptoBoolean
cryptoNetworkRequiresMigration (BRCryptoNetwork network);

// MARK: - Account Initialization

/**
 * Check if `account` is initialized on the network
 */
extern BRCryptoBoolean
cryptoNetworkIsAccountInitialized (BRCryptoNetwork network,
                                   BRCryptoAccount account);

/**
 * Get a newly-allocated byte array of the data needed to initialize `account` on the network.
 * Fill `bytesCount` with the number of bytes in the initializtion data.
 */

extern uint8_t *
cryptoNetworkGetAccountInitializationData (BRCryptoNetwork network,
                                           BRCryptoAccount account,
                                           size_t *bytesCount);

/**
 * Initialize `account` on the network with `data`.  The `initialzation data`, from
 * `cryptoNetworkGetAccountInitializationData()` is used to derive `data`.
 */
extern void
cryptoNetworkInitializeAccount (BRCryptoNetwork network,
                                BRCryptoAccount account,
                                const uint8_t *bytes,
                                size_t bytesCount);

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoNetwork, cryptoNetwork);

/**
 * Find the built-in network with `uids` and `isMainnet.
 */
extern BRCryptoNetwork
cryptoNetworkFindBuiltin (const char *uids,
                          bool isMainnet);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoNetwork_h */
