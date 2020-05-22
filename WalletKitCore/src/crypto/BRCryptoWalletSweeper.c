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

extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperHandleTransactionAsBTC (BRCryptoWalletSweeper sweeper,
                                           OwnershipKept uint8_t *transaction,
                                           size_t transactionLen) {
    BRCryptoWalletSweeperStatus status = CRYPTO_WALLET_SWEEPER_ILLEGAL_OPERATION;

    //TODO:SWEEPER
#ifdef REFACTOR
    status = BRWalletSweeperStatusToCrypto (
                                            BRWalletSweeperHandleTransaction (sweeper->u.btc.sweeper,
                                                                              transaction, transactionLen)
                                            );
#endif
    return status;
}

extern BRCryptoKey
cryptoWalletSweeperGetKey (BRCryptoWalletSweeper sweeper) {
    return cryptoKeyTake (sweeper->key);
}

extern BRCryptoAddress
cryptoWalletSweeperGetAddress (BRCryptoWalletSweeper sweeper) {
    //TODO:SWEEPER
#ifdef REFACTOR
    char * address = NULL;

    switch (sweeper->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            address = BRWalletSweeperGetLegacyAddress (sweeper->u.btc.sweeper);
            break;
        }
        default:
            assert (0);
            break;
    }

    return address;
#endif
    return NULL;
}

extern BRCryptoAmount
cryptoWalletSweeperGetBalance (BRCryptoWalletSweeper sweeper) {
    BRCryptoAmount amount = NULL;
#ifdef REFACTOR
    switch (sweeper->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            UInt256 value = uint256Create (BRWalletSweeperGetBalance (sweeper->u.btc.sweeper));
            amount = cryptoAmountCreate (sweeper->unit, CRYPTO_FALSE, value);
            break;
        }
        default:
            assert (0);
            break;
    }
#endif
    return amount;
}

extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperValidate (BRCryptoWalletSweeper sweeper) {
    BRCryptoWalletSweeperStatus status = CRYPTO_WALLET_SWEEPER_ILLEGAL_OPERATION;
#ifdef REFACTOR
    switch (sweeper->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            status = BRWalletSweeperStatusToCrypto (
                BRWalletSweeperValidate(sweeper->u.btc.sweeper)
            );
            break;
        }
        default:
            assert (0);
            break;
    }
#endif
    return status;
}

extern void
cryptoWalletManagerEstimateFeeBasisForWalletSweep (BRCryptoWalletManager cwm,
                                                   BRCryptoWallet wallet,
                                                   BRCryptoCookie cookie,
                                                   BRCryptoWalletSweeper sweeper,
                                                   BRCryptoNetworkFee fee) {
#ifdef REFACTOR//TODO:SWEEP
    switch (cwm->type) {
        case BLOCK_CHAIN_TYPE_BTC: {
            BRWalletManager bwm = cwm->u.btc;
            BRWallet *wid = cryptoWalletAsBTC (wallet);
            uint64_t feePerKB = 1000 * cryptoNetworkFeeAsBTC (fee);

            BRWalletManagerEstimateFeeForSweep (bwm,
                                                wid,
                                                cookie,
                                                cryptoWalletSweeperAsBTC(sweeper),
                                                feePerKB);
            break;
        }
        default:
            assert (0);
            break;
    }
#endif
}

#ifdef REFACTOR
private_extern BRWalletSweeper
cryptoWalletSweeperAsBTC (BRCryptoWalletSweeper sweeper) {
    assert (BLOCK_CHAIN_TYPE_BTC == sweeper->type);
    return sweeper->u.btc.sweeper;
}
#endif
