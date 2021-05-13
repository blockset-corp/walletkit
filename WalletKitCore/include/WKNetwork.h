//
//  WKNetwork.h
//  WalletKitCore
//
//  Created by Ed Gamble on 3/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKNetwork_h
#define WKNetwork_h

#include "WKAccount.h"
#include "WKAddress.h"
#include "WKAmount.h"
#include "WKSync.h"
#include "WKListener.h"
#include "WKHash.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - (Network) Address Scheme

/**
 * An enumeration of possible network address schemes.
 */
typedef enum {
    WK_ADDRESS_SCHEME_BTC_LEGACY,
    WK_ADDRESS_SCHEME_BTC_SEGWIT,
    WK_ADDRESS_SCHEME_NATIVE,          // Default for Currency
} WKAddressScheme;

#define NUMBER_OF_ADDRESS_SCHEMES   (1 + WK_ADDRESS_SCHEME_NATIVE)

/**
 * A NetworkFee represents the possible 'pricePerCostFactors' and the price's corresponding
 * expected confirmation time.
 *
 * @discussion Generally to achieve a shorter confimation time, one must pay a higher price.
 * That is, price is a measure of miner priority.
 */
typedef struct WKNetworkFeeRecord *WKNetworkFee;

/**
 * Create a NetworkFee
 */
extern WKNetworkFee
wkNetworkFeeCreate (uint64_t confirmationTimeInMilliseconds,
                    WKAmount pricePerCostFactor,
                    WKUnit   pricePerCostFactorUnit);

/**
 * The estimated time to confirm a transfer for this network fee
 *
 * @param networkFee the network fee
 *
 * @return time in milliseconds
 */
extern uint64_t
wkNetworkFeeGetConfirmationTimeInMilliseconds (WKNetworkFee networkFee);

/**
 * Get the network fee's pricePerCostFactor
 */
extern WKAmount
wkNetworkFeeGetPricePerCostFactor (WKNetworkFee networkFee);

/**
 * Get the network fee's pricePerCostFactor unit
 */
extern WKUnit
wkNetworkFeeGetPricePerCostFactorUnit (WKNetworkFee networkFee);

/**
 * Check of two network fee's are equal.
 */
extern WKBoolean
wkNetworkFeeEqual (WKNetworkFee nf1, WKNetworkFee nf2);

DECLARE_WK_GIVE_TAKE (WKNetworkFee, wkNetworkFee);


// MARK: - Network

/**
 * Get the network's unique-identifier.  All networks, whether mainnet or testnet, will have
 * a globally unique identifier.
 */
extern const char *
wkNetworkGetUids (WKNetwork network);

/**
 * Get the network's name.  For bitcoin this is "Bitcoin"; for ethereum this is "Ethereum"
 */
extern const char *
wkNetworkGetName (WKNetwork network);

/**
 * Check of the network is a mainnet network.
 */
extern WKBoolean
wkNetworkIsMainnet (WKNetwork network);

/**
 * Get the network's type.  For bitcoin the type is `WK_NETWORK_TYPE_BTC`.
 */
extern WKNetworkType
wkNetworkGetType (WKNetwork network);

/**
 * Return the Blockchain type the network with `name` or WK_NETWORK_TYPE_UNKNOWN if
 * there is no network with `name`.
 *
 * @param name the name
 * @param isMainnet filled with true if `name` is for mainnet; false otherwise.
 */
extern WKNetworkType
wkNetworkGetTypeFromName (const char *name, WKBoolean *isMainnet);

/**
 * Returns the network's currency.  This is typically (always?) the currency used to pay
 * for network fees.
 
 @param network The network
 @return the network's currency w/ an incremented reference count (aka 'taken')
 */
extern WKCurrency
wkNetworkGetCurrency (WKNetwork network);

/**
 * Add `currency`, with the provided base and default units, to the network.
 */
extern void
wkNetworkAddCurrency (WKNetwork network,
                      WKCurrency currency,
                      WKUnit baseUnit,
                      WKUnit defaultUnit);

/**
 * Get the code for the network's native currency.
 */
extern const char *
wkNetworkGetCurrencyCode (WKNetwork network);

/**
 * Returns the currency's default unit or NULL
 *
 * @param network the network
 * @param currency the currency or NULL for the network's currency.
 *
 * @return the currency's default unit or NULL w/ an incremented reference count (aka 'taken')
 */
extern WKUnit
wkNetworkGetUnitAsDefault (WKNetwork network,
                           WKCurrency currency);

/**
 * Returns the currency's base unit or NULL
 *
 * @param network the network
 * @param currency the currency or NULL for the network's currency.
 *
 * @return the currency's base unit or NULL w/ an incremented reference count (aka 'taken')
 */
extern WKUnit
wkNetworkGetUnitAsBase (WKNetwork network,
                        WKCurrency currency);

/**
 * Add `unit` to the known units for `currency` on the network.
 */
extern void
wkNetworkAddCurrencyUnit (WKNetwork network,
                          WKCurrency currency,
                          WKUnit unit);

/**
 * Get the network's height.  The height changes as the blockchain is extended.
 */
extern WKBlockNumber
wkNetworkGetHeight (WKNetwork network);

/**
 * Set the network's height.
 */
extern void
wkNetworkSetHeight (WKNetwork network,
                    WKBlockNumber height);

/**
 * Get the network's verified block hash.
 */
extern WKHash
wkNetworkGetVerifiedBlockHash (WKNetwork network);

/**
 * Set the network's verified block hash.
 */
extern void
wkNetworkSetVerifiedBlockHash (WKNetwork network,
                               WKHash verifiedBlockHash);

/**
 * Set the network's verified block hash from the string encoding of the hash.
 */
extern void
wkNetworkSetVerifiedBlockHashAsString (WKNetwork network,
                                       const char * verifiedBlockHashString);

/**
 * Get the network's count of blocks needed to confirm a transaction.  For Bitcoin this is six.
 */
extern uint32_t
wkNetworkGetConfirmationsUntilFinal (WKNetwork network);

/**
 * Returns the number of network currencies.  This is the index exclusive limit to be used
 * in `wkNetworkGetCurrencyAt()`.
 *
 * @param network the network
 *
 * @return number of network currencies.
 */
extern size_t
wkNetworkGetCurrencyCount (WKNetwork network);

/**
 * Returns the network's currency at `index`.  The index must satisfy [0, count) otherwise
 * an assertion is signaled.
 *
 * @param network the network
 * @param index the desired currency index
 *
 * @return The currency w/ an incremented reference count (aka 'taken')
 */
extern WKCurrency
wkNetworkGetCurrencyAt (WKNetwork network,
                        size_t index);

/**
 * Return 'TRUE' is `network` has `currency`.
 *
 * @param network the network
 *@param currency the currency
 *
 *@return WK_TRUE if `network` has `currency`.
 */
extern WKBoolean
wkNetworkHasCurrency (WKNetwork network,
                      WKCurrency currency);

/**
 * Returns the network's currency with `symbol` or NULL.
 *
 * @param network the network
 * @param index the desired currency's symbol
 *
 * @return The currency w/ an incremented reference count (aka 'taken')
 */
extern WKCurrency
wkNetworkGetCurrencyForCode (WKNetwork network,
                             const char *code);

/**
 * Get the network's currency with `uids`; otherwise `NULL`
 */
extern WKCurrency
wkNetworkGetCurrencyForUids (WKNetwork network,
                             const char *uids);

/**
 * Get the network's currency with `issuer`; otherwise `NULL`
 */
extern WKCurrency
wkNetworkGetCurrencyForIssuer (WKNetwork network,
                               const char *issuer);

/**
 * Returns the number of units for network's `currency`.  This is the index exclusive limit to
 * be used in `wkNetworkGetUnitAt()`.
 *
 * @param network the network
 * @param currency the currency or NULL for the network's currency.
 *
 * @return the number of units for `currency`
 */
extern size_t
wkNetworkGetUnitCount (WKNetwork network,
                       WKCurrency currency);

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
extern WKUnit
wkNetworkGetUnitAt (WKNetwork network,
                    WKCurrency currency,
                    size_t index);

/**
 * Get the network's count of network fees.
 */
extern size_t
wkNetworkGetNetworkFeeCount (WKNetwork network);

/**
 * Get the network fee at `index` from among the network's fees
 */
extern WKNetworkFee
wkNetworkGetNetworkFeeAt (WKNetwork network,
                          size_t index);

/*
 * Get all the network's fee and fill `count` with the number of fees.
 */
extern WKNetworkFee *
wkNetworkGetNetworkFees (WKNetwork network,
                         size_t *count);

/**
 * Set the network's fee.
 */
extern void
wkNetworkSetNetworkFees (WKNetwork network,
                         const WKNetworkFee *fees,
                         size_t count);

/**
 * Add `fee` to the network's fees.
 */
extern void
wkNetworkAddNetworkFee (WKNetwork network,
                        WKNetworkFee fee);

// MARK: - Address Scheme

/**
 * Get the network's default address scheme.
 */
extern WKAddressScheme
wkNetworkGetDefaultAddressScheme (WKNetwork network);

/**
 * Get a newly allocated array of the network's address schemes.  Fill `count` with the number
 * of schemes.
 */
extern const WKAddressScheme *
wkNetworkGetSupportedAddressSchemes (WKNetwork network,
                                     WKCount *count);

/**
 * Check if network supports scheme
 */
extern WKBoolean
wkNetworkSupportsAddressScheme (WKNetwork network,
                                WKAddressScheme scheme);

// MARK: - Address

/**
 * Create a network address from the string representation of in `address`
 */
extern WKAddress
wkNetworkCreateAddress (WKNetwork network,
                        const char *address);

// MARK: - Sync Mode

/**
 * Get the network's default sync-mode
 */
extern WKSyncMode
wkNetworkGetDefaultSyncMode (WKNetwork network);

/**
 * Get a newly allocated array of the network's sync modes.  Fill `count` with the number of
 * sync modes.
 */
extern const WKSyncMode *
wkNetworkGetSupportedSyncModes (WKNetwork network,
                                WKCount *count);

/**
 * Check if network support `mode`
 */
extern WKBoolean
wkNetworkSupportsSyncMode (WKNetwork network,
                           WKSyncMode mode);

/**
 * Check if network requires migration (of persistent storage).
 */
extern WKBoolean
wkNetworkRequiresMigration (WKNetwork network);

// MARK: - Account Initialization

/**
 * Check if `account` is initialized on the network
 */
extern WKBoolean
wkNetworkIsAccountInitialized (WKNetwork network,
                               WKAccount account);

/**
 * Get a newly-allocated byte array of the data needed to initialize `account` on the network.
 * Fill `bytesCount` with the number of bytes in the initializtion data.
 */

extern uint8_t *
wkNetworkGetAccountInitializationData (WKNetwork network,
                                       WKAccount account,
                                       size_t *bytesCount);

/**
 * Initialize `account` on the network with `data`.  The `initialzation data`, from
 * `wkNetworkGetAccountInitializationData()` is used to derive `data`.
 */
extern void
wkNetworkInitializeAccount (WKNetwork network,
                            WKAccount account,
                            const uint8_t *bytes,
                            size_t bytesCount);

DECLARE_WK_GIVE_TAKE (WKNetwork, wkNetwork);

/**
 * Find the built-in network with `uids` and `isMainnet.
 */
extern WKNetwork
wkNetworkFindBuiltin (const char *uids,
                      bool isMainnet);

#ifdef __cplusplus
}
#endif

#endif /* WKNetwork_h */
