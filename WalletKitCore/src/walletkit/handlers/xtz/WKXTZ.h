//
//  WKXTZ.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-08-27.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKXTZ_h
#define WKXTZ_h

#include "walletkit/WKHandlersP.h"

#include "tezos/BRTezos.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct WKAddressXTZRecord {
    struct WKAddressRecord base;
    BRTezosAddress addr;
} *WKAddressXTZ;

extern WKAddress
wkAddressCreateAsXTZ (OwnershipGiven BRTezosAddress addr);

extern WKAddress
wkAddressCreateFromStringAsXTZ (const char *string);

private_extern BRTezosAddress
wkAddressAsXTZ (WKAddress address);

// MARK: - Network

typedef struct WKNetworkXTZRecord {
    struct WKNetworkRecord base;
    // Nothing more needed
} *WKNetworkXTZ;

// MARK: - Transfer

typedef struct WKTransferXTZRecord {
    struct WKTransferRecord base;

    BRTezosHash hash;
    BRTezosUnitMutez amount;

    BRTezosTransaction originatingTransaction;
} *WKTransferXTZ;

extern WKTransferXTZ
wkTransferCoerceXTZ (WKTransfer transfer);

extern WKTransfer
wkTransferCreateAsXTZ (WKTransferListener listener,
                       const char *uids,
                       WKUnit unit,
                       WKUnit unitForFee,
                       WKFeeBasis feeBasisEstimated,
                       WKAmount   amount,
                       WKAddress  source,
                       WKAddress  target,
                       WKTransferState state,
                       OwnershipKept BRTezosAccount xtzAccount,
                       BRTezosHash xtzHash,
                       OwnershipGiven BRTezosTransaction xtzTransaction);
// MARK: - Wallet

typedef struct WKWalletXTZRecord {
    struct WKWalletRecord base;
    BRTezosAccount xtzAccount;
} *WKWalletXTZ;

extern WKWalletHandlers wkWalletHandlersXTZ;

private_extern WKWallet
wkWalletCreateAsXTZ (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BRTezosAccount xtzAccount);

extern WKTransfer
wkWalletCreateTransferXTZ (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee);

private_extern bool
wkWalletNeedsRevealXTZ (WKWallet wallet);

private_extern WKTransfer
wkWalletGetTransferByHashOrUIDSAndTargetXTZ (WKWallet wallet,
                                             WKHash hashToMatch,
                                             const char *uids,
                                             WKAddress targetToMatch);

// MARK: - Wallet Manager

typedef struct WKWalletManagerXTZRecord {
    struct WKWalletManagerRecord base;
} *WKWalletManagerXTZ;

extern WKWalletManagerHandlers wkWalletManagerHandlersXTZ;

// MARK: - Fee Basis

typedef struct WKFeeBasisXTZRecord {
    struct WKFeeBasisRecord base;
    BRTezosFeeBasis xtzFeeBasis;
} *WKFeeBasisXTZ;

private_extern WKFeeBasis
wkFeeBasisCreateAsXTZ (WKUnit unit,
                           BRTezosFeeBasis xtzFeeBasis);

private_extern WKFeeBasisXTZ
wkFeeBasisCoerceXTZ (WKFeeBasis feeBasis);

private_extern BRTezosFeeBasis
wkFeeBasisAsXTZ (WKFeeBasis feeBasis);

// MARK: - Support

#define FIELD_OPTION_DELEGATION_OP          "DelegationOp"
#define FIELD_OPTION_DELEGATE               "delegate"
#define FIELD_OPTION_OPERATION_TYPE         "type"

private_extern WKAmount
wkAmountCreateAsXTZ (WKUnit unit,
                         WKBoolean isNegative,
                         BRTezosUnitMutez value);

private_extern BRTezosUnitMutez
tezosMutezCreate (WKAmount amount);

private_extern WKHash
wkHashCreateAsXTZ (BRTezosHash hash);

private_extern WKHash
wkHashCreateFromStringAsXTZ(const char *input);

private_extern BRTezosHash
wkHashAsXTZ (WKHash hash);

private_extern const char **
tezosGetTransactionAttributeKeys (int asRequired,
                                  size_t *count);


#ifdef __cplusplus
}
#endif

#endif // WKXTZ_h
