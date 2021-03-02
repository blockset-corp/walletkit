//
//  BRCryptoExportablePaperWalletP.h
//  BRCore
//
//  Created by Ed Gamble on 2/23/21
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoExportablePaperWalletP_h
#define BRCryptoExportablePaperWalletP_h

#include "BRCryptoExportablePaperWallet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef BRCryptoExportablePaperWallet
(*BRCryptoExportablePaperWalletCreateHandler) (BRCryptoNetwork network,
                                               BRCryptoCurrency currency);

typedef void
(*BRCryptoExportablePaperWalletReleaseHandler) (BRCryptoExportablePaperWallet wallet);

typedef BRCryptoExportablePaperWalletStatus
(*BRCryptoExportablePaperWalletValidateSupportedHandler) (BRCryptoNetwork network,
                                                          BRCryptoCurrency currency);

typedef struct {
    BRCryptoExportablePaperWalletCreateHandler create;
    BRCryptoExportablePaperWalletReleaseHandler release;
    BRCryptoExportablePaperWalletValidateSupportedHandler validateSupported;
} BRCryptoExportablePaperWalletHandlers;

// MARK: - Sweeper

struct BRCryptoExportablePaperWalletRecord {
    BRCryptoBlockChainType type;
    const BRCryptoExportablePaperWalletHandlers *handlers;
    size_t sizeInBytes;

    BRCryptoNetwork network;
    BRCryptoAddress address;
    BRCryptoKey key;
};


private_extern BRCryptoExportablePaperWallet
cryptoExportablePaperWalletAllocAndInit (size_t sizeInBytes,
                                         BRCryptoBlockChainType type,
                                         BRCryptoNetwork network,
                                         BRCryptoAddress address,
                                         BRCryptoKey key);

private_extern void
cryptoExportablePaperWalletReleaseInternal (BRCryptoExportablePaperWallet wallet);

#endif /* BRCryptoExportablePaperWalletP_h */
