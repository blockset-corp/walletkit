//
//  WKAVAX.h
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKAVAX_h
#define WKAVAX_h

#include <assert.h>

#include "walletkit/WKAmountP.h"
#include "walletkit/WKHandlersP.h"

#include "avalanche/BRAvalanche.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Amount

// Assumes sizeof(BRAvalancheAmount) <= sizeof(uint64_t)
static inline WKAmount
wkAmountCreateAsAVAX (WKUnit unit,
                      WKBoolean isNegative,
                      BRAvalancheAmount value) { // value is positive
    return wkAmountCreate (unit, isNegative, uint256Create ((uint64_t)value));
}

static inline BRAvalancheAmount
wkAmountAsAVAX (WKAmount amount,
                WKBoolean *isNegative) {
    WKBoolean overflow = WK_FALSE;

    BRAvalancheAmount avaxAmount = (BRAvalancheAmount) wkAmountGetIntegerRaw (amount, &overflow);
    assert (WK_FALSE == overflow);

    *isNegative = wkAmountIsNegative (amount);
    return avaxAmount;
}

// MARK: - Hash

static inline WKHash
wkHashCreateAsAVAX (BRAvalancheHash hash) {
    return wkHashCreateInternal ((uint32_t) avalancheHashSetValue (&hash),
                                 AVALANCHE_HASH_BYTES,
                                 hash.bytes,
                                 WK_NETWORK_TYPE_AVAX);
}

static inline WKHash
wkHashCreateFromStringAsAVAX(const char *input) {
    return wkHashCreateAsAVAX (avalancheHashFromString (input));
}

static inline BRAvalancheHash
wkHashAsAVAX (WKHash hash) {
    assert (AVALANCHE_HASH_BYTES == hash->bytesCount);

    BRAvalancheHash avax;
    memcpy (avax.bytes, hash->bytes, AVALANCHE_HASH_BYTES);

    return avax;
}

// MARK: - Address

typedef struct WKAddressAVAXRecord {
    struct WKAddressRecord base;

    // The BRAvalanceAddress are, essentially, the AVALANCHE_ADDRESS_BYTES_{X,C,P} bytes
    BRAvalancheAddress avaxAddress;

    // The BRAvalanceNetwork is required to string-ify the address
    BRAvalancheNetwork avaxNetwork;
} *WKAddressAVAX;

static inline WKAddressAVAX
wkAddressCoerceAVAX (WKAddress address) {
    assert (WK_NETWORK_TYPE_AVAX == address->type);
    return (WKAddressAVAX) address;
}

extern WKAddress
wkAddressCreateAsAVAX (BRAvalancheAddress addr,
                       BRAvalancheNetwork network);

private_extern BRAvalancheAddress
wkAddressAsAVAX (WKAddress address);

// MARK: - Network

typedef struct WKNetworkAVAXRecord {
    struct WKNetworkRecord base;
    const BRAvalancheNetwork avaxNetwork;
} *WKNetworkAVAX;

static inline WKNetworkAVAX
wkNetworkCoerceAVAX (WKNetwork network) {
    assert (WK_NETWORK_TYPE_AVAX == network->type);
    return (WKNetworkAVAX) network;
}

private_extern BRAvalancheNetwork
wkNetworkAsAVAX (WKNetwork network);

// MARK: - Account

static inline BRAvalancheAccount
wkAccountGetAsAVAX (WKAccount account) {
    BRAvalancheAccount avaxAccount = (BRAvalancheAccount) wkAccountAs (account, WK_NETWORK_TYPE_AVAX);
    assert (NULL != avaxAccount);
    return avaxAccount;
}

// MARK: - Fee Basis

typedef struct WKFeeBasisAVAXRecord {
    struct WKFeeBasisRecord base;
    BRAvalancheFeeBasis avaxFeeBasis;
} *WKFeeBasisAVAX;

static inline WKFeeBasisAVAX
wkFeeBasisCoerceAVAX (WKFeeBasis feeBasis) {
    assert (WK_NETWORK_TYPE_AVAX == feeBasis->type);
    return (WKFeeBasisAVAX) feeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAsAVAX (WKUnit unit,
                        BRAvalancheFeeBasis avaxFeeBasis);

static inline BRAvalancheFeeBasis
wkFeeBasisAsAVAX (WKFeeBasis feeBasis) {
    WKFeeBasisAVAX feeBasisAVAX = wkFeeBasisCoerceAVAX(feeBasis);
    return feeBasisAVAX->avaxFeeBasis;

}


// MARK: - Transfer

typedef struct WKTransferAVAXRecord {
    struct WKTransferRecord base;

    BRAvalancheTransaction avaxTransaction;
} *WKTransferAVAX;

static inline WKTransferAVAX
wkTransferCoerceAVAX (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE_AVAX == transfer->type);
    return (WKTransferAVAX) transfer;
}

extern WKTransfer
wkTransferCreateAsAVAX (WKTransferListener listener,
                        const char *uids,
                        WKUnit unit,
                        WKUnit unitForFee,
                        WKTransferState state,
                        BRAvalancheAccount     avaxAccount,
                        BRAvalancheNetwork     avaxNetwork,
                        BRAvalancheTransaction avaxTransaction);

// MARK: - Wallet

typedef struct WKWalletAVAXRecord {
    struct WKWalletRecord base;
    
    // Typically the BRAvalancheAccount
    BRAvalancheAccount avaxAccount;
    BRAvalancheNetwork avaxNetwork;
} *WKWalletAVAX;

extern WKWalletHandlers wkWalletHandlersAVAX;

private_extern WKWallet
wkWalletCreateAsAVAX (WKWalletListener listener,
                      WKUnit unit,
                      WKUnit unitForFee,
                      BRAvalancheAccount avaxAccount,
                      BRAvalancheNetwork avaxNetwork);

extern WKTransfer
wkWalletCreateTransferAVAX (WKWallet  wallet,
                            WKAddress target,
                            WKAmount  amount,
                            WKFeeBasis estimatedFeeBasis,
                            size_t attributesCount,
                            OwnershipKept WKTransferAttribute *attributes,
                            WKCurrency currency,
                            WKUnit unit,
                            WKUnit unitForFee);

private_extern bool
wkWalletNeedsRevealAVAX (WKWallet wallet);

private_extern WKTransfer
wkWalletGetTransferByHashOrUIDSAndTargetAVAX (WKWallet wallet,
                                              WKHash hashToMatch,
                                              const char *uids,
                                              WKAddress targetToMatch);

// MARK: - Wallet Manager

typedef struct WKWalletManagerAVAXRecord {
    struct WKWalletManagerRecord base;

    // BRAvalancheAcount  avaxAccount;
    // BRAvalancheNetwork avaxNetwork;
} *WKWalletManagerAVAX;

extern WKWalletManagerHandlers wkWalletManagerHandlersAVAX;

#ifdef __cplusplus
}
#endif

#endif // WKAVAX_h
