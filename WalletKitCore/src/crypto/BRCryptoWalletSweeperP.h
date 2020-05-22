//
//  BRCryptoWalletSweeperP.h
//  BRCore
//
//  Created by Ehsan Rezaie on 5/22/20
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletSweeperP_h
#define BRCryptoWalletSweeperP_h

#include "BRCryptoWalletSweeper.h"


#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Sweeper Handlers

typedef void
(*BRCryptoWalletSweeperReleaseHandler) (BRCryptoWalletSweeper sweeper);

typedef BRCryptoWalletSweeperStatus
(*BRCryptoWalletSweeperAddTransactionFromBundleHandler) (BRCryptoWalletSweeper sweeper,
                                                         OwnershipKept BRCryptoClientTransactionBundle bundle);

typedef void
(*BRCryptoWalletSweeperEstimateFeeBasisForWalletSweep) (BRCryptoWalletManager cwm,
                                                        BRCryptoWallet wallet,
                                                        BRCryptoCookie cookie,
                                                        BRCryptoWalletSweeper sweeper,
                                                        BRCryptoNetworkFee fee);

typedef struct {
    BRCryptoWalletSweeperReleaseHandler release;
    BRCryptoWalletSweeperAddTransactionFromBundleHandler addTranactionFromBundle;
    BRCryptoWalletSweeperEstimateFeeBasisForWalletSweep estimateFeeBasis;
} BRCryptoWalletSweeperHandlers;

// MARK: - Sweeper

struct BRCryptoWalletSweeperRecord {
    BRCryptoBlockChainType type;
    const BRCryptoWalletSweeperHandlers *handlers;
    BRCryptoKey key;
    BRCryptoUnit unit;
};

private_extern BRCryptoWalletSweeper
cryptoWalletSweeperAllocAndInit (size_t sizeInBytes,
                                 BRCryptoBlockChainType type,
                                 BRCryptoKey key,
                                 BRCryptoUnit unit);


#endif /* BRCryptoWalletSweeperP_h */
