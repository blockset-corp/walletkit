//
//  BRCryptoWalletSweeper.h
//  BRCore
//
//  Created by Ed Gamble on 2/23/21
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoExportablePaperWallet_h
#define BRCryptoExportablePaperWallet_h

#include "BRCryptoCurrency.h"
#include "BRCryptoNetwork.h"
#include "BRCryptoKey.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: Exportable Paper Wallet

typedef struct BRCryptoExportablePaperWalletRecord *BRCryptoExportablePaperWallet;

typedef enum {
    CRYPTO_EXPORTABLE_PAPER_WALLET_SUCCESS,
    CRYPTO_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY,
    CRYPTO_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS,

    // calling a sweeper function for the wrong type
    CRYPTO_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION,
} BRCryptoExportablePaperWalletStatus;

extern BRCryptoExportablePaperWalletStatus
cryptoExportablePaperWalletValidateSupported (BRCryptoNetwork network,
                                              BRCryptoCurrency currency);

extern BRCryptoExportablePaperWallet
cryptoExportablePaperWalletCreate (BRCryptoNetwork network,
                                   BRCryptoCurrency currency);

extern void
cryptoExportablePaperWalletRelease (BRCryptoExportablePaperWallet paperWallet);

extern BRCryptoKey
cryptoExportablePaperWalletGetKey (BRCryptoExportablePaperWallet paperWallet);

extern BRCryptoAddress
cryptoExportablePaperWalletGetAddress (BRCryptoExportablePaperWallet paperWallet);

#endif /* BRCryptoExportablePaperWallet_h */
