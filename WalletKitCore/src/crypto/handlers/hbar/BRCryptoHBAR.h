//
//  BRCryptoHBAR.h
//  Core
//
//  Created by Ehsan Rezaie on 2020-05-19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRCryptoHBAR_h
#define BRCryptoHBAR_h

#include "../BRCryptoHandlersExport.h"

#include "hedera/BRHederaBase.h"
#include "hedera/BRHederaAddress.h"
#include "hedera/BRHederaTransaction.h"
#include "hedera/BRHederaAccount.h"
#include "hedera/BRHederaWallet.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct BRCryptoAddressHBARRecord {
    struct BRCryptoAddressRecord base;
    BRHederaAddress addr;
} *BRCryptoAddressHBAR;

extern BRCryptoAddress
cryptoAddressCreateAsHBAR (BRHederaAddress addr);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsHBAR (const char *string);

private_extern BRHederaAddress
cryptoAddressAsHBAR (BRCryptoAddress address);

// MARK: - Network

//TODO:HBAR needed?
//typedef struct BRCryptoNetworkHBARRecord {
//    struct BRCryptoNetworkRecord base;
//    // ...
//} *BRCryptoNetworkHBAR;

// MARK: - Transfer

typedef struct BRCryptoTransferHBARRecord {
    struct BRCryptoTransferRecord base;

    BRHederaTransaction hbarTransaction;
} *BRCryptoTransferHBAR;

extern BRCryptoTransferHBAR
cryptoTransferCoerceHBAR (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsHBAR (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRHederaWallet wallet,
                           BRHederaTransaction hederaTx);

// MARK: - Wallet

typedef struct BRCryptoWalletHBARRecord {
    struct BRCryptoWalletRecord base;
    BRHederaWallet wid;
} *BRCryptoWalletHBAR;

extern BRCryptoWalletHandlers cryptoWalletHandlersHBAR;

private_extern BRHederaWallet
cryptoWalletAsHBAR (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsHBAR (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRHederaWallet wid);


//TODO:HBAR needed?
private_extern BRCryptoHash
cryptoHashCreateAsHBAR (BRHederaTransactionHash hash);

// MARK: - Wallet Manager

typedef struct BRCryptoWalletManagerHBARRecord {
    struct BRCryptoWalletManagerRecord base;

    int ignoreTBD;
} *BRCryptoWalletManagerHBAR;

extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersHBAR;

// MARK: - Events

// MARK: - Support

#define TRANSFER_ATTRIBUTE_MEMO_TAG         "Memo"

private_extern int // 1 if equal, 0 if not.
hederaCompareAttribute (const char *t1, const char *t2);

private_extern BRCryptoAmount
cryptoAmountCreateAsHBAR (BRCryptoUnit unit,
                          BRCryptoBoolean isNegative,
                          BRHederaUnitTinyBar value);

private_extern const char **
hederaWalletGetTransactionAttributeKeys (BRHederaWallet wallet,
                                         BRHederaAddress address,
                                         int asRequired,
                                         size_t *count);


#ifdef __cplusplus
}
#endif

#endif // BRCryptoHBAR_h
