//
//  WKSTELLAR.h
//  WalletKitCore
//
//  Created by Carl Cherry on 2021-05-19.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKSTELLAR_h
#define WKSTELLAR_h

#include "../WKHandlersExport.h"
#include "walletkit/WKFeeBasisP.h"

#include "stellar/BRStellar.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct WKAddressXLMRecord {
    struct WKAddressRecord base;
    BRStellarAddress addr;
} *WKAddressXLM;

extern WKAddress
wkAddressCreateAsXLM (BRStellarAddress addr);

extern WKAddress
wkAddressCreateFromStringAsXLM (const char *string);

private_extern BRStellarAddress
wkAddressAsXLM (WKAddress address);

// MARK: - Network

typedef struct WKNetworkXLMRecord {
    struct WKNetworkRecord base;
    // Nothing more needed
} *WKNetworkXLM;

// MARK: - Transfer

typedef struct WKTransferXLMRecord {
    struct WKTransferRecord base;

    BRStellarTransaction xlmTransaction;
} *WKTransferXLM;

extern WKTransferXLM
wkTransferCoerceXLM (WKTransfer transfer);

extern WKTransfer
wkTransferCreateAsXLM (WKTransferListener listener,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKTransferState state,
                           BRStellarAccount xlmAccount,
                           BRStellarTransaction xlmTransaction);

extern BRStellarTransaction
wkTransferAsXLM (WKTransfer transfer);

// MARK: - Wallet

typedef struct WKWalletXLMRecord {
    struct WKWalletRecord base;
    BRStellarAccount xlmAccount;
} *WKWalletXLM;

extern WKWalletHandlers wkWalletHandlersXLM;

private_extern WKWallet
wkWalletCreateAsXLM (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BRStellarAccount xlmAccount);

private_extern WKHash
wkHashCreateAsXLM (BRStellarTransactionHash hash);

private_extern BRStellarTransactionHash
stellarHashCreateFromString (const char *string);

// MARK: - Wallet Manager

typedef struct WKWalletManagerXLMRecord {
    struct WKWalletManagerRecord base;
} *WKWalletManagerXLM;

extern WKWalletManagerHandlers wkWalletManagerHandlersXLM;

// MARK: - Fee Basis

typedef struct WKFeeBasisXLMRecord {
    struct WKFeeBasisRecord base;
    BRStellarFeeBasis xlmFeeBasis;
} *WKFeeBasisXLM;

private_extern WKFeeBasis
wkFeeBasisCreateAsXLM (WKUnit unit, BRStellarAmount fee);

private_extern BRStellarFeeBasis
wkFeeBasisAsXLM (WKFeeBasis feeBasis);

// MARK: - Support

#define FIELD_OPTION_DESTINATION_TAG        "DestinationTag"
#define FIELD_OPTION_INVOICE_ID             "InvoiceId"

private_extern int // 1 if equal, 0 if not.
stellarCompareFieldOption (const char *t1, const char *t2);

private_extern WKAmount
wkAmountCreateAsXLM (WKUnit unit, WKBoolean isNegative, BRStellarAmount value);

private_extern const char **
stellarAddressGetTransactionAttributeKeys (BRStellarAddress address, int asRequired, size_t *count);

#ifdef __cplusplus
}
#endif

#endif WKSTELLAR_h
