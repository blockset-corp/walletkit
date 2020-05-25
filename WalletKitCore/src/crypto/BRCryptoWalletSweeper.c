//
//  BRCryptoWalletSweeper.c
//  BRCore
//
//  Created by Ehsan Rezaie on 5/22/20
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoWalletSweeperP.h"
#include "BRCryptoHandlersP.h"

extern BRCryptoWalletSweeper
cryptoWalletSweeperAllocAndInit (size_t sizeInBytes,
                                 BRCryptoBlockChainType type,
                                 BRCryptoKey key,
                                 BRCryptoUnit unit) {
    assert (sizeInBytes >= sizeof (struct BRCryptoWalletSweeperRecord));
    assert (cryptoKeyHasSecret (key));
    
    BRCryptoWalletSweeper sweeper = calloc (1, sizeInBytes);
    
    sweeper->type = type;
    sweeper->handlers = cryptoHandlersLookup(type)->sweeper;
    sweeper->key = cryptoKeyTake (key);
    sweeper->unit = cryptoUnitTake (unit);
    
    return sweeper;
}

extern void
cryptoWalletSweeperRelease (BRCryptoWalletSweeper sweeper) {
    
    sweeper->handlers->release (sweeper);
    
    cryptoKeyGive (sweeper->key);
    cryptoUnitGive (sweeper->unit);
    
    memset (sweeper, 0, sizeof(*sweeper));
    free (sweeper);
}

extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperAddTransactionFromBundle (BRCryptoWalletSweeper sweeper,
                                             OwnershipKept BRCryptoClientTransactionBundle bundle) {
    if (NULL == sweeper->handlers->addTranactionFromBundle) {
        assert(0);
        return CRYPTO_WALLET_SWEEPER_ILLEGAL_OPERATION;
    }
    
    return sweeper->handlers->addTranactionFromBundle (sweeper, bundle);
}

extern BRCryptoTransfer
cryptoWalletSweeperCreateTransferForWalletSweep (BRCryptoWalletSweeper sweeper,
                                                 BRCryptoWalletManager cwm,
                                                 BRCryptoWallet wallet,
                                                 BRCryptoFeeBasis estimatedFeeBasis) {
    return sweeper->handlers->createTransfer (cwm, wallet, sweeper, estimatedFeeBasis);
}

extern BRCryptoKey
cryptoWalletSweeperGetKey (BRCryptoWalletSweeper sweeper) {
    return cryptoKeyTake (sweeper->key);
}

extern BRCryptoAddress
cryptoWalletSweeperGetAddress (BRCryptoWalletSweeper sweeper) {
    return sweeper->handlers->getAddress (sweeper);
}

extern BRCryptoAmount
cryptoWalletSweeperGetBalance (BRCryptoWalletSweeper sweeper) {
    return sweeper->handlers->getBalance (sweeper);
}

extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperValidate (BRCryptoWalletSweeper sweeper) {
    return sweeper->handlers->validate (sweeper);
}

extern void
cryptoWalletManagerEstimateFeeBasisForWalletSweep (BRCryptoWalletManager cwm,
                                                   BRCryptoWallet wallet,
                                                   BRCryptoCookie cookie,
                                                   BRCryptoWalletSweeper sweeper,
                                                   BRCryptoNetworkFee fee) {
    sweeper->handlers->estimateFeeBasis (cwm,
                                         wallet,
                                         cookie,
                                         sweeper,
                                         fee);
}
