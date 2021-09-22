//
//  WKHBAR.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKHBAR_h
#define WKHBAR_h

#include "walletkit/WKHandlersP.h"

#include "hedera/BRHederaBase.h"
#include "hedera/BRHederaAddress.h"
#include "hedera/BRHederaTransaction.h"
#include "hedera/BRHederaAccount.h"
#include "hedera/BRHederaToken.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct WKAddressHBARRecord {
    struct WKAddressRecord base;
    BRHederaAddress addr;
} *WKAddressHBAR;

extern WKAddress
wkAddressCreateAsHBAR (BRHederaAddress addr);

extern WKAddress
wkAddressCreateFromStringAsHBAR (const char *string);

private_extern BRHederaAddress
wkAddressAsHBAR (WKAddress address);

// MARK: - Network

typedef struct WKNetworkHBARRecord {
    struct WKNetworkRecord base;
    // Nothing more needed
} *WKNetworkHBAR;

// MARK: - Transfer

typedef struct WKTransferHBARRecord {
    struct WKTransferRecord base;

    BRHederaTransaction hbarTransaction;
} *WKTransferHBAR;

extern WKTransferHBAR
wkTransferCoerceHBAR (WKTransfer transfer);

extern WKTransfer
wkTransferCreateAsHBAR (WKTransferListener listener,
                            const char *uids,
                            WKUnit unit,
                            WKUnit unitForFee,
                            WKTransferState state,
                            BRHederaAccount hbarAccount,
                            BRHederaTransaction hbarTransaction);

// MARK: - Wallet

typedef struct WKWalletHBARRecord {
    struct WKWalletRecord base;
    BRHederaAccount hbarAccount;
    BRHederaToken token;
} *WKWalletHBAR;

extern WKWalletHandlers wkWalletHandlersHBAR;

private_extern WKWallet
wkWalletCreateAsHBAR (WKWalletListener listener,
                          WKUnit unit,
                          WKUnit unitForFee,
                          BRHederaAccount hbarAccount,
                          BRHederaToken token);

private_extern WKHash
wkHashCreateAsHBAR (BRHederaTransactionHash hash);

private_extern BRHederaTransactionHash
hederaHashCreateFromString (const char *string);

private_extern BRHederaTransactionHash
wkHashAsHBAR (WKHash hash);

// MARK: - Wallet Manager

typedef struct WKWalletManagerHBARRecord {
    struct WKWalletManagerRecord base;

    BRSetOf(BRHederaToken) tokens;

} *WKWalletManagerHBAR;

extern WKWalletManagerHandlers wkWalletManagerHandlersHBAR;

// MARK: - Fee Basis

typedef struct WKFeeBasisHBARRecord {
    struct WKFeeBasisRecord base;
    BRHederaFeeBasis hbarFeeBasis;
} *WKFeeBasisHBAR;

private_extern WKFeeBasis
wkFeeBasisCreateAsHBAR (WKUnit unit,
                            BRHederaFeeBasis hbarFeeBasis);

private_extern BRHederaFeeBasis
wkFeeBasisAsHBAR (WKFeeBasis feeBasis);

private_extern WKFeeBasisHBAR
wkFeeBasisCoerceHBAR (WKFeeBasis feeBasis);

// MARK: - Support

#define TRANSFER_ATTRIBUTE_MEMO_TAG         "Memo"

private_extern int // 1 if equal, 0 if not.
hederaCompareAttribute (const char *t1, const char *t2);

private_extern WKAmount
wkAmountCreateAsHBAR (WKUnit unit,
                          WKBoolean isNegative,
                          BRHederaUnitTinyBar value);

private_extern const char **
hederaWalletGetTransactionAttributeKeys (BRHederaAddress address,
                                         int asRequired,
                                         size_t *count);

// MARK: - Misc

#define HWM_INITIAL_SET_SIZE_DEFAULT  (10)


#ifdef __cplusplus
}
#endif

#endif // WKHBAR_h
