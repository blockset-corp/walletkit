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
#include "crypto/BRCryptoFeeBasisP.h"

#include "hedera/BRHederaBase.h"
#include "hedera/BRHederaAddress.h"
#include "hedera/BRHederaTransaction.h"
#include "hedera/BRHederaAccount.h"

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

typedef struct BRCryptoNetworkHBARRecord {
    struct BRCryptoNetworkRecord base;
    // Nothing more needed
} *BRCryptoNetworkHBAR;

// MARK: - Transfer

typedef struct BRCryptoTransferHBARRecord {
    struct BRCryptoTransferRecord base;

    BRHederaTransaction hbarTransaction;
} *BRCryptoTransferHBAR;

extern BRCryptoTransferHBAR
cryptoTransferCoerceHBAR (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsHBAR (BRCryptoTransferListener listener,
                            const char *uids,
                            BRCryptoUnit unit,
                            BRCryptoUnit unitForFee,
                            BRCryptoTransferState state,
                            BRHederaAccount hbarAccount,
                            BRHederaTransaction hbarTransaction);

// MARK: - Wallet

typedef struct BRCryptoWalletHBARRecord {
    struct BRCryptoWalletRecord base;
    BRHederaAccount hbarAccount;
} *BRCryptoWalletHBAR;

extern BRCryptoWalletHandlers cryptoWalletHandlersHBAR;

private_extern BRCryptoWallet
cryptoWalletCreateAsHBAR (BRCryptoWalletListener listener,
                          BRCryptoUnit unit,
                          BRCryptoUnit unitForFee,
                          BRHederaAccount hbarAccount);

private_extern BRCryptoHash
cryptoHashCreateAsHBAR (BRHederaTransactionHash hash);

private_extern BRHederaTransactionHash
hederaHashCreateFromString (const char *string);

private_extern BRHederaTransactionHash
cryptoHashAsHBAR (BRCryptoHash hash);

// MARK: - Wallet Manager

typedef struct BRCryptoWalletManagerHBARRecord {
    struct BRCryptoWalletManagerRecord base;
} *BRCryptoWalletManagerHBAR;

extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersHBAR;

// MARK: - Fee Basis

typedef struct BRCryptoFeeBasisHBARRecord {
    struct BRCryptoFeeBasisRecord base;
    BRHederaFeeBasis hbarFeeBasis;
} *BRCryptoFeeBasisHBAR;

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsHBAR (BRCryptoUnit unit,
                            BRHederaFeeBasis hbarFeeBasis);

private_extern BRHederaFeeBasis
cryptoFeeBasisAsHBAR (BRCryptoFeeBasis feeBasis);

private_extern BRCryptoFeeBasisHBAR
cryptoFeeBasisCoerceHBAR (BRCryptoFeeBasis feeBasis);

// MARK: - Support

#define TRANSFER_ATTRIBUTE_MEMO_TAG         "Memo"

private_extern int // 1 if equal, 0 if not.
hederaCompareAttribute (const char *t1, const char *t2);

private_extern BRCryptoAmount
cryptoAmountCreateAsHBAR (BRCryptoUnit unit,
                          BRCryptoBoolean isNegative,
                          BRHederaUnitTinyBar value);

private_extern const char **
hederaWalletGetTransactionAttributeKeys (BRHederaAddress address,
                                         int asRequired,
                                         size_t *count);


#ifdef __cplusplus
}
#endif

#endif // BRCryptoHBAR_h
