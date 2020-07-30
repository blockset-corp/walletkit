//
//  BRCryptoXRP.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRCryptoXRP_h
#define BRCryptoXRP_h

#include "../BRCryptoHandlersExport.h"

#include "ripple/BRRippleBase.h"
#include "ripple/BRRippleAddress.h"
#include "ripple/BRRippleTransfer.h"
#include "ripple/BRRippleAccount.h"
#include "ripple/BRRippleWallet.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct BRCryptoAddressXRPRecord {
    struct BRCryptoAddressRecord base;
    BRRippleAddress addr;
} *BRCryptoAddressXRP;

extern BRCryptoAddress
cryptoAddressCreateAsXRP (BRRippleAddress addr);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsXRP (const char *string);

private_extern BRRippleAddress
cryptoAddressAsXRP (BRCryptoAddress address);

// MARK: - Network

typedef struct BRCryptoNetworkXRPRecord {
    struct BRCryptoNetworkRecord base;
    // Nothing more needed
} *BRCryptoNetworkXRP;

// MARK: - Transfer

typedef struct BRCryptoTransferXRPRecord {
    struct BRCryptoTransferRecord base;

    BRRippleTransfer xrpTransfer;
} *BRCryptoTransferXRP;

extern BRCryptoTransferXRP
cryptoTransferCoerceXRP (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsXRP (BRCryptoTransferListener listener,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRRippleWallet wallet,
                           BRRippleTransfer xrpTransfer);

// MARK: - Wallet

typedef struct BRCryptoWalletXRPRecord {
    struct BRCryptoWalletRecord base;
    BRRippleWallet wid;
} *BRCryptoWalletXRP;

extern BRCryptoWalletHandlers cryptoWalletHandlersXRP;

private_extern BRRippleWallet
cryptoWalletAsXRP (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsXRP (BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRRippleWallet wid);


//TODO:XRP needed?
private_extern BRCryptoHash
cryptoHashCreateAsXRP (BRRippleTransactionHash hash);

// MARK: - Wallet Manager

typedef struct BRCryptoWalletManagerXRPRecord {
    struct BRCryptoWalletManagerRecord base;

    int ignoreTBD;
} *BRCryptoWalletManagerXRP;

extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersXRP;

// MARK: - Events

// MARK: - Support


#define FIELD_OPTION_DESTINATION_TAG        "DestinationTag"
#define FIELD_OPTION_INVOICE_ID             "InvoiceId"

private_extern int // 1 if equal, 0 if not.
rippleCompareFieldOption (const char *t1, const char *t2);

private_extern BRCryptoAmount
cryptoAmountCreateAsXRP (BRCryptoUnit unit,
                         BRCryptoBoolean isNegative,
                         BRRippleUnitDrops value);

private_extern const char **
rippleWalletGetTransactionAttributeKeys (BRRippleWallet wallet,
                                         BRRippleAddress address,
                                         int asRequired,
                                         size_t *count);


#ifdef __cplusplus
}
#endif

#endif // BRCryptoXRP_h
