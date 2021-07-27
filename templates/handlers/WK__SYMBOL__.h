//
//  WK__SYMBOL__.h
//  WalletKitCore
//
//  Created by __USER__ on __DATE__.
//  Copyright © __YEAR__ Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WK__SYMBOL___h
#define WK__SYMBOL___h

#include <assert.h>

#include "walletkit/WKAmountP.h"
#include "walletkit/WKHandlersP.h"

#include "__name__/BR__Name__.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Amount

// Assumes sizeof(BR__Name__Amount) <= sizeof(uint64_t)
static inline WKAmount
wkAmountCreateAs__SYMBOL__ (WKUnit unit,
                            WKBoolean isNegative,
                            BR__Name__Amount value) { // value is positive
    return wkAmountCreate (unit, isNegative, uint256Create ((uint64_t)value));
}

static inline BR__Name__Amount
wkAmountAs__SYMBOL__ (WKAmount amount,
                      WKBoolean *isNegative) {
    WKBoolean overflow = WK_FALSE;

    BR__Name__Amount __symbol__Amount = (BR__Name__Amount) wkAmountGetIntegerRaw (amount, &overflow);
    assert (WK_FALSE == overflow);

    *isNegative = wkAmountIsNegative (amount);
    return __symbol__Amount;
}

// MARK: - Hash

static inline WKHash
wkHashCreateAs__SYMBOL__ (BR__Name__Hash hash) {
    return wkHashCreateInternal (__name__HashSetValue (&hash),
                                 __NAME___HASH_BYTES,
                                 hash.bytes,
                                 WK_NETWORK_TYPE___SYMBOL__);
}

static inline WKHash
wkHashCreateFromStringAs__SYMBOL__(const char *input) {
    return wkHashCreateAs__SYMBOL__ (__name__HashFromString (input));
}

static inline BR__Name__Hash
wkHashAs__SYMBOL__ (WKHash hash) {
    assert (__NAME___HASH_BYTES == hash->bytesCount);

    BR__Name__Hash __symbol__;
    memcpy (__symbol__.bytes, hash->bytes, __NAME___HASH_BYTES);

    return __symbol__;
}


// MARK: - Address

typedef struct WKAddress__SYMBOL__Record {
    struct WKAddressRecord base;
    BR__Name__Address addr;
} *WKAddress__SYMBOL__;

static inline WKAddress__SYMBOL__
wkAddressCoerce__SYMBOL__ (WKAddress address) {
    assert (WK_NETWORK_TYPE___SYMBOL__ == address->type);
    return (WKAddress__SYMBOL__) address;
}

extern WKAddress
wkAddressCreateAs__SYMBOL__ (BR__Name__Address addr);

extern WKAddress
wkAddressCreateFromStringAs__SYMBOL__ (const char *string);

private_extern BR__Name__Address
wkAddressAs__SYMBOL__ (WKAddress address);

// MARK: - Network

typedef struct WKNetwork__SYMBOL__Record {
    struct WKNetworkRecord base;
    // Nothing more needed
} *WKNetwork__SYMBOL__;

static inline WKNetwork__SYMBOL__
wkNetworkCoerce__SYMBOL__ (WKNetwork network) {
    assert (WK_NETWORK_TYPE___SYMBOL__ == network->type);
    return (WKNetwork__SYMBOL__) network;
}

// MARK: - Account

static inline BR__Name__Account
wkAccountGetAs__SYMBOL__ (WKAccount account) {
    BR__Name__Account __symbol__Account = (BR__Name__Account) wkAccountAs (account, WK_NETWORK_TYPE___SYMBOL__);
    assert (NULL != __symbol__Account);
    return __symbol__Account;
}

// MARK: - Fee Basis

typedef struct WKFeeBasis__SYMBOL__Record {
    struct WKFeeBasisRecord base;
    BR__Name__FeeBasis __symbol__FeeBasis;
} *WKFeeBasis__SYMBOL__;

static inline WKFeeBasis__SYMBOL__
wkFeeBasisCoerce__SYMBOL__ (WKFeeBasis feeBasis) {
    assert (WK_NETWORK_TYPE___SYMBOL__ == feeBasis->type);
    return (WKFeeBasis__SYMBOL__) feeBasis;
}

private_extern WKFeeBasis
wkFeeBasisCreateAs__SYMBOL__ (WKUnit unit,
                              BR__Name__FeeBasis __symbol__FeeBasis);

static inline BR__Name__FeeBasis
wkFeeBasisAs__SYMBOL__ (WKFeeBasis feeBasis) {
    WKFeeBasis__SYMBOL__ feeBasis__SYMBOL__ = wkFeeBasisCoerce__SYMBOL__(feeBasis);
    return feeBasis__SYMBOL__->__symbol__FeeBasis;

}


// MARK: - Transfer

typedef struct WKTransfer__SYMBOL__Record {
    struct WKTransferRecord base;

    BR__Name__Transaction __symbol__Transaction;
} *WKTransfer__SYMBOL__;

static inline WKTransfer__SYMBOL__
wkTransferCoerce__SYMBOL__ (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE___SYMBOL__ == transfer->type);
    return (WKTransfer__SYMBOL__) transfer;
}

extern WKTransfer
wkTransferCreateAs__SYMBOL__ (WKTransferListener listener,
                           const char *uids,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKTransferState state,
                           BR__Name__Account __symbol__Account,
                           BR__Name__Transaction __symbol__Transaction);

// MARK: - Wallet

typedef struct WKWallet__SYMBOL__Record {
    struct WKWalletRecord base;

    // Typically the BR__Name__Account
    BR__Name__Account __symbol__Account;
} *WKWallet__SYMBOL__;

extern WKWalletHandlers wkWalletHandlers__SYMBOL__;

private_extern WKWallet
wkWalletCreateAs__SYMBOL__ (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BR__Name__Account __symbol__Account);

extern WKTransfer
wkWalletCreateTransfer__SYMBOL__ (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee);

private_extern bool
wkWalletNeedsReveal__SYMBOL__ (WKWallet wallet);

private_extern WKTransfer
wkWalletGetTransferByHashOrUIDSAndTarget__SYMBOL__ (WKWallet wallet,
                                             WKHash hashToMatch,
                                             const char *uids,
                                             WKAddress targetToMatch);

// MARK: - Wallet Manager

typedef struct WKWalletManager__SYMBOL__Record {
    struct WKWalletManagerRecord base;

    // BR__Name__Acount  __symbol__Account;
    // BR__Name__Network __symbol__Network;
} *WKWalletManager__SYMBOL__;

extern WKWalletManagerHandlers wkWalletManagerHandlers__SYMBOL__;

#ifdef __cplusplus
}
#endif

#endif // WK__SYMBOL___h
