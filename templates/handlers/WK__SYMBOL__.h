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

#include "walletkit/WKHandlersP.h"

#include "__name__/BR__Name__.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct WKAddress__SYMBOL__Record {
    struct WKAddressRecord base;
    BR__Name__Address addr;
} *WKAddress__SYMBOL__;

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

// MARK: - Transfer

typedef struct WKTransfer__SYMBOL__Record {
    struct WKTransferRecord base;

    BR__Name__Transfer __symbol__Transfer;
} *WKTransfer__SYMBOL__;

extern WKTransfer__SYMBOL__
wkTransferCoerce__SYMBOL__ (WKTransfer transfer);

extern WKTransfer
wkTransferCreateAs__SYMBOL__ (WKTransferListener listener,
                           const char *uids,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKTransferState state,
                           BR__Name__Account __symbol__Account,
                           BR__Name__Transfer __symbol__Transfer);

// MARK: - Wallet

typedef struct WKWallet__SYMBOL__Record {
    struct WKWalletRecord base;
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
} *WKWalletManager__SYMBOL__;

extern WKWalletManagerHandlers wkWalletManagerHandlers__SYMBOL__;

// MARK: - Fee Basis

typedef struct WKFeeBasis__SYMBOL__Record {
    struct WKFeeBasisRecord base;
    BR__Name__FeeBasis __symbol__FeeBasis;
} *WKFeeBasis__SYMBOL__;

private_extern WKFeeBasis
wkFeeBasisCreateAs__SYMBOL__ (WKUnit unit,
                           BR__Name__FeeBasis __symbol__FeeBasis);

private_extern WKFeeBasis__SYMBOL__
wkFeeBasisCoerce__SYMBOL__ (WKFeeBasis feeBasis);

private_extern BR__Name__FeeBasis
wkFeeBasisAs__SYMBOL__ (WKFeeBasis feeBasis);

// MARK: - Support

#define FIELD_OPTION_DELEGATION_OP          "DelegationOp"
#define FIELD_OPTION_DELEGATE               "delegate"
#define FIELD_OPTION_OPERATION_TYPE         "type"

private_extern WKAmount
wkAmountCreateAs__SYMBOL__ (WKUnit unit,
                         WKBoolean isNegative,
                         BR__Name__Amount value);

private_extern BR__Name__Amount
__name__MutezCreate (WKAmount amount);

private_extern WKHash
wkHashCreateAs__SYMBOL__ (BR__Name__Hash hash);

private_extern WKHash
wkHashCreateFromStringAs__SYMBOL__(const char *input);

private_extern BR__Name__Hash
wkHashAs__SYMBOL__ (WKHash hash);

private_extern const char **
__name__GetTransactionAttributeKeys (int asRequired,
                                  size_t *count);


#ifdef __cplusplus
}
#endif

#endif // WK__SYMBOL___h
