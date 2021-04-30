//
//  WKNetworkP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 11/22/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKNetworkP_h
#define WKNetworkP_h

#include <pthread.h>
#include <stdbool.h>

#include "support/BRArray.h"

#include "WKBaseP.h"
#include "WKHashP.h"
#include "WKClientP.h"
#include "WKListenerP.h"

#include "WKNetwork.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Network Fee

struct WKNetworkFeeRecord {
    uint64_t confirmationTimeInMilliseconds;
    WKAmount pricePerCostFactor;
    WKUnit   pricePerCostFactorUnit;  // Until in WKAmount
    WKRef ref;
};

private_extern WKNetworkFee
wkNetworkFeeCreate (uint64_t confirmationTimeInMilliseconds,
                        WKAmount pricePerCostFactor,
                        WKUnit   pricePerCostFactorUnit);

/// MARK: - Currency Association

typedef struct {
    WKCurrency currency;
    WKUnit baseUnit;
    WKUnit defaultUnit;
    BRArrayOf(WKUnit) units;
} WKCurrencyAssociation;

/// MARK: - Network Handlers

typedef WKNetwork
(*WKNetworkCreateHandler) (WKNetworkListener listener,
                                 const char *uids,               // bitcoin-testnet
                                 const char *name,               // Bitcoin
                                 const char *network,            // testnet
                                 bool isMainnet,                 // false
                                 uint32_t confirmationPeriodInSeconds,  // 10 * 60
                                 WKAddressScheme defaultAddressScheme,
                                 WKSyncMode defaultSyncMode,
                                 WKCurrency nativeCurrency);

typedef void
(*WKNetworkReleaseHandler) (WKNetwork network);

typedef WKAddress
(*WKNetworkCreateAddressHandler) (WKNetwork network,
                                        const char *addressAsString);

typedef WKBlockNumber
(*WKNetworkGetBlockNumberAtOrBeforeTimestampHandler) (WKNetwork network,
                                                            WKTimestamp timestamp);

typedef WKBoolean
(*WKNetworkIsAccountInitializedHandler) (WKNetwork network,
                                               WKAccount account);


typedef uint8_t *
(*WKNetworkGetAccountInitializationDataHandler) (WKNetwork network,
                                                       WKAccount account,
                                                       size_t *bytesCount);

typedef void
(*WKNetworkInitializeAccountHandler) (WKNetwork network,
                                            WKAccount account,
                                            const uint8_t *bytes,
                                            size_t bytesCount);

typedef WKHash
(*WKNetworkCreateHashFromStringHandler) (WKNetwork network,
                                               const char *string);

typedef char *
(*WKNetworkEncodeHashHandler) (WKHash hash);

typedef struct {
    WKNetworkCreateHandler create;
    WKNetworkReleaseHandler release;
    WKNetworkCreateAddressHandler createAddress;
    WKNetworkGetBlockNumberAtOrBeforeTimestampHandler getBlockNumberAtOrBeforeTimestamp;
    WKNetworkIsAccountInitializedHandler isAccountInitialized;
    WKNetworkGetAccountInitializationDataHandler getAccountInitializationData;
    WKNetworkInitializeAccountHandler initializeAccount;
    WKNetworkCreateHashFromStringHandler createHashFromString;
    WKNetworkEncodeHashHandler encodeHash;
} WKNetworkHandlers;

/// MARK: - Network

struct WKNetworkRecord {
    WKNetworkType type;
    const WKNetworkHandlers *handlers;
    WKRef ref;
    size_t sizeInBytes;
    
    pthread_mutex_t lock;

    WKNetworkListener listener;
    
    char *uids;
    char *name;
    char *desc;
    bool isMainnet;

    WKBlockNumber height;
    WKHash verifiedBlockHash;

    // Base and associated currencies.
    WKCurrency currency;
    BRArrayOf(WKCurrencyAssociation) associations;

    uint32_t confirmationPeriodInSeconds;
    uint32_t confirmationsUntilFinal;

    // Address Schemes
    BRArrayOf(WKAddressScheme) addressSchemes;
    WKAddressScheme defaultAddressScheme;

    // Sync Modes
    BRArrayOf(WKSyncMode) syncModes;
    WKSyncMode defaultSyncMode;

    // Fees
    BRArrayOf(WKNetworkFee) fees;
};

typedef void *WKNetworkCreateContext;
typedef void (*WKNetworkCreateCallback) (WKNetworkCreateContext context,
                                               WKNetwork network);

extern WKNetwork
wkNetworkAllocAndInit (size_t sizeInBytes,
                           WKNetworkType type,
                           WKNetworkListener listener,
                           const char *uids,
                           const char *name,
                           const char *desc,        // "mainnet", "testnet", "rinkeby"
                           bool isMainnet,
                           uint32_t confirmationPeriodInSeconds,
                           WKAddressScheme defaultAddressScheme,
                           WKSyncMode defaultSyncMode,
                           WKCurrency currencyNative,
                           WKNetworkCreateContext createContext,
                           WKNetworkCreateCallback createCallback);

private_extern WKNetworkType
wkNetworkGetType (WKNetwork network);

private_extern const char *
wkNetworkGetDesc (WKNetwork network);

private_extern uint32_t
wkNetworkGetConfirmationPeriodInSeconds (WKNetwork network);

private_extern void
wkNetworkAnnounce (WKNetwork network);

private_extern void
wkNetworkSetHeight (WKNetwork network,
                        WKBlockNumber height);

private_extern void
wkNetworkSetConfirmationsUntilFinal (WKNetwork network,
                                         uint32_t confirmationsUntilFinal);

private_extern void
wkNetworkAddCurrency (WKNetwork network,
                          WKCurrency currency,
                          WKUnit baseUnit,
                          WKUnit defaultUnit);

private_extern void
wkNetworkAddCurrencyUnit (WKNetwork network,
                              WKCurrency currency,
                              WKUnit unit);

private_extern void
wkNetworkAddCurrencyAssociationFromBundle (WKNetwork network,
                                               OwnershipKept WKClientCurrencyBundle bundle,
                                               WKBoolean needEvent);

private_extern void
wkNetworkAddCurrencyAssociationsFromBundles (WKNetwork network,
                                                 OwnershipKept BRArrayOf(WKClientCurrencyBundle) bundles);

private_extern void
wkNetworkAddNetworkFee (WKNetwork network,
                            WKNetworkFee fee);

private_extern void
wkNetworkSetNetworkFees (WKNetwork network,
                             const WKNetworkFee *fees,
                             size_t count);

private_extern WKNetworkType
wkNetworkGetBlockChainType (WKNetwork network);

private_extern WKBlockNumber
wkNetworkGetBlockNumberAtOrBeforeTimestamp (WKNetwork network,
                                                WKTimestamp timestamp);

private_extern WKHash
wkNetworkCreateHashFromString (WKNetwork network,
                                   const char *string);

private_extern OwnershipGiven char *
wkNetworkEncodeHash (WKHash hash);

static inline void
wkNetworkGenerateEvent (WKNetwork network,
                            WKNetworkEvent event) {
    wkListenerGenerateNetworkEvent (&network->listener, network, event);
}

/**
 * Return a newly-allocated array of the built-in networks that are on `isMainet.  The
 * `listerner` has events announced to it.  Fill `networksCount` with the number of install
 * networks.
 */
private_extern WKNetwork *
wkNetworkInstallBuiltins (size_t *networksCount,
                              WKNetworkListener listener,
                              bool isMainnet);

#ifdef __cplusplus
}
#endif

#endif /* WKNetworkP_h */
