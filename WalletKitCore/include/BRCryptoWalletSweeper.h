//
//  BRCryptoWalletSweeper.h
//  BRCore
//
//  Created by Ehsan Rezaie on 5/22/20
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletSweeper_h
#define BRCryptoWalletSweeper_h

#include "BRCryptoClient.h"
#include "BRCryptoKey.h"
#include "BRCryptoAmount.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct BRCryptoWalletSweeperRecord *BRCryptoWalletSweeper;

    // MARK: Wallet Sweeper Status

    typedef enum {
        CRYPTO_WALLET_SWEEPER_SUCCESS,
        CRYPTO_WALLET_SWEEPER_UNSUPPORTED_CURRENCY,
        CRYPTO_WALLET_SWEEPER_INVALID_KEY,
        CRYPTO_WALLET_SWEEPER_INVALID_ARGUMENTS,
        CRYPTO_WALLET_SWEEPER_INVALID_TRANSACTION,
        CRYPTO_WALLET_SWEEPER_INVALID_SOURCE_WALLET,
        CRYPTO_WALLET_SWEEPER_NO_TRANSFERS_FOUND,
        CRYPTO_WALLET_SWEEPER_INSUFFICIENT_FUNDS,
        CRYPTO_WALLET_SWEEPER_UNABLE_TO_SWEEP,
        
        // calling a sweeper function for the wrong type
        CRYPTO_WALLET_SWEEPER_ILLEGAL_OPERATION,
    } BRCryptoWalletSweeperStatus;

    extern void
    cryptoWalletSweeperRelease (BRCryptoWalletSweeper sweeper);

    extern BRCryptoWalletSweeperStatus
    cryptoWalletSweeperAddTransactionFromBundle (BRCryptoWalletSweeper sweeper,
                                                 OwnershipKept BRCryptoClientTransactionBundle bundle);

    extern void
    cryptoWalletManagerEstimateFeeBasisForWalletSweep (BRCryptoWalletSweeper sweeper,
                                                       BRCryptoWalletManager manager,
                                                       BRCryptoWallet wallet,
                                                       BRCryptoCookie cookie,
                                                       BRCryptoNetworkFee fee);

    extern BRCryptoTransfer
    cryptoWalletSweeperCreateTransferForWalletSweep (BRCryptoWalletSweeper sweeper,
                                                     BRCryptoWalletManager manager,
                                                     BRCryptoWallet wallet,
                                                     BRCryptoFeeBasis estimatedFeeBasis);

    extern BRCryptoKey
    cryptoWalletSweeperGetKey (BRCryptoWalletSweeper sweeper);

    extern BRCryptoAddress
    cryptoWalletSweeperGetAddress (BRCryptoWalletSweeper sweeper);

    extern BRCryptoAmount
    cryptoWalletSweeperGetBalance (BRCryptoWalletSweeper sweeper);

    extern BRCryptoWalletSweeperStatus
    cryptoWalletSweeperValidate (BRCryptoWalletSweeper sweeper);

#ifdef __cplusplus
}
#endif

#endif
