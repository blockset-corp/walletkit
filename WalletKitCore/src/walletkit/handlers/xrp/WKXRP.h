//
//  WKXRP.h
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKXRP_h
#define WKXRP_h

#include "../WKHandlersExport.h"
#include "walletkit/WKFeeBasisP.h"

#include "ripple/BRRipple.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct WKAddressXRPRecord {
    struct WKAddressRecord base;
    BRRippleAddress addr;
} *WKAddressXRP;

extern WKAddress
wkAddressCreateAsXRP (BRRippleAddress addr);

extern WKAddress
wkAddressCreateFromStringAsXRP (const char *string);

private_extern BRRippleAddress
wkAddressAsXRP (WKAddress address);

// MARK: - Network

typedef struct WKNetworkXRPRecord {
    struct WKNetworkRecord base;
    // Nothing more needed
} *WKNetworkXRP;

// MARK: - Transfer

typedef struct WKTransferXRPRecord {
    struct WKTransferRecord base;

    BRRippleTransaction xrpTransaction;
} *WKTransferXRP;

extern WKTransferXRP
wkTransferCoerceXRP (WKTransfer transfer);

extern WKTransfer
wkTransferCreateAsXRP (WKTransferListener listener,
                           const char *uids,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKTransferState state,
                           BRRippleAccount xrpAccount,
                           BRRippleTransaction xrpTransaction);

extern BRRippleTransaction
wkTransferAsXRP (WKTransfer transfer);

// MARK: - Wallet

typedef struct WKWalletXRPRecord {
    struct WKWalletRecord base;
    BRRippleAccount xrpAccount;
} *WKWalletXRP;

extern WKWalletHandlers wkWalletHandlersXRP;

private_extern WKWallet
wkWalletCreateAsXRP (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BRRippleAccount xrpAccount);

private_extern WKHash
wkHashCreateAsXRP (BRRippleTransactionHash hash);

private_extern BRRippleTransactionHash
rippleHashCreateFromString (const char *string);

// MARK: - Wallet Manager

typedef struct WKWalletManagerXRPRecord {
    struct WKWalletManagerRecord base;
} *WKWalletManagerXRP;

extern WKWalletManagerHandlers wkWalletManagerHandlersXRP;

// MARK: - Fee Basis

typedef struct WKFeeBasisXRPRecord {
    struct WKFeeBasisRecord base;
    BRRippleFeeBasis xrpFeeBasis;
} *WKFeeBasisXRP;

private_extern WKFeeBasis
wkFeeBasisCreateAsXRP (WKUnit unit,
                           BRRippleUnitDrops fee);

private_extern BRRippleFeeBasis
wkFeeBasisAsXRP (WKFeeBasis feeBasis);

// MARK: - Support


#define FIELD_OPTION_DESTINATION_TAG        "DestinationTag"
#define FIELD_OPTION_INVOICE_ID             "InvoiceId"

private_extern int // 1 if equal, 0 if not.
rippleCompareFieldOption (const char *t1, const char *t2);

private_extern WKAmount
wkAmountCreateAsXRP (WKUnit unit,
                         WKBoolean isNegative,
                         BRRippleUnitDrops value);

private_extern const char **
rippleAddressGetTransactionAttributeKeys (BRRippleAddress address,
                                          int asRequired,
                                          size_t *count);

#ifdef __cplusplus
}
#endif

#endif // WKXRP_h
