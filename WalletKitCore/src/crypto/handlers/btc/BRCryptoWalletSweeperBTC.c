//
//  BRCryptoWalletSweeperBTC.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 5/22/20
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoBTC.h"
#include "crypto/BRCryptoWalletSweeperP.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtilMath.h"

// MARK: Forward Declarations

static BRCryptoWalletSweeperStatus
btcWalletSweeperEstimateFee (BRCryptoWalletSweeperBTC sweeper,
                            BRBitcoinWallet * wallet,
                            uint64_t feePerKb,
                            uint64_t *feeEstimate);

static BRCryptoWalletSweeperStatus
btcWalletSweeperCreateTransaction (BRCryptoWalletSweeperBTC sweeper,
                                  BRBitcoinWallet * wallet,
                                  uint64_t feePerKb,
                                  BRBitcoinTransaction **transaction);

static uint64_t
btcWalletSweeperGetBalance (BRCryptoWalletSweeperBTC sweeper);

// MARK: - Handlers

static BRCryptoWalletSweeperBTC
cryptoWalletSweeperCoerce (BRCryptoWalletSweeper sweeper) {
    assert (CRYPTO_NETWORK_TYPE_BTC == sweeper->type ||
            CRYPTO_NETWORK_TYPE_BCH == sweeper->type ||
            CRYPTO_NETWORK_TYPE_BSV == sweeper->type);
    return (BRCryptoWalletSweeperBTC) sweeper;
}

private_extern BRCryptoAddress
cryptoWalletSweeperGetAddressBTC (BRCryptoWalletSweeper sweeper) {
    BRCryptoWalletSweeperBTC sweeperBTC = cryptoWalletSweeperCoerce (sweeper);
    switch (sweeper->type) {
        case CRYPTO_NETWORK_TYPE_BTC:
            return cryptoAddressCreateFromStringAsBTC (sweeperBTC->addrParams, sweeperBTC->sourceAddress);
        case CRYPTO_NETWORK_TYPE_BSV:
            return cryptoAddressCreateFromStringAsBSV (sweeperBTC->addrParams, sweeperBTC->sourceAddress);
            
        case CRYPTO_NETWORK_TYPE_BCH:
            return cryptoAddressCreateFromLegacyStringAsBCH (sweeperBTC->addrParams, sweeperBTC->sourceAddress);
            
        default:
            assert (0);
            return NULL;
    }
}

private_extern BRCryptoAmount
cryptoWalletSweeperGetBalanceBTC (BRCryptoWalletSweeper sweeper) {
    BRCryptoWalletSweeperBTC sweeperBTC = cryptoWalletSweeperCoerce (sweeper);
    UInt256 value = uint256Create (btcWalletSweeperGetBalance (sweeperBTC));
    return cryptoAmountCreate (sweeper->unit, CRYPTO_FALSE, value);
}

private_extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperAddTransactionFromBundleBTC (BRCryptoWalletSweeper sweeper,
                                                OwnershipKept BRCryptoClientTransactionBundle bundle) {
    BRCryptoWalletSweeperStatus status = CRYPTO_WALLET_SWEEPER_SUCCESS;
    BRCryptoWalletSweeperBTC sweeperBTC = cryptoWalletSweeperCoerce (sweeper);

    BRBitcoinTransaction * txn = btcTransactionParse (bundle->serialization, bundle->serializationCount);
    if (NULL != txn) {
        array_add (sweeperBTC->txns, txn);
    } else {
        status = CRYPTO_WALLET_SWEEPER_INVALID_TRANSACTION;
    }

    return status;
}

private_extern BRCryptoFeeBasis
cryptoWalletSweeperEstimateFeeBasisForWalletSweepBTC (BRCryptoWalletManager cwm,
                                                      BRCryptoWallet wallet,
                                                      BRCryptoCookie cookie,
                                                      BRCryptoWalletSweeper sweeper,
                                                      BRCryptoNetworkFee networkFee) {
    BRBitcoinWallet *wid = cryptoWalletAsBTC (wallet);
    
    BRCryptoWalletSweeperBTC sweeperBTC = cryptoWalletSweeperCoerce (sweeper);
    
    uint64_t feePerKb = 1000 * cryptoNetworkFeeAsBTC (networkFee);
    
    // TODO(fix): We should move this, along with BRWalletManagerEstimateFeeForTransfer, to
    //            a model where they return a status code. We are currently providing no
    //            context to the caller.
    uint64_t fee = 0;
    btcWalletSweeperEstimateFee (sweeperBTC, wid, feePerKb, &fee);
    
    return cryptoFeeBasisCreateAsBTC (wallet->unitForFee, fee, feePerKb, CRYPTO_FEE_BASIS_BTC_SIZE_UNKNOWN);
}

static BRBitcoinTransaction *
cryptoWalletSweeperTransactionForSweepAsBTC (BRCryptoWalletManager manager,
                                             BRBitcoinWallet *wallet,
                                             BRCryptoWalletSweeperBTC sweeper,
                                             uint64_t feePerKb) {
    assert (wallet == cryptoWalletAsBTC (manager->wallet));

    pthread_mutex_lock (&manager->lock);

    // TODO(fix): We should move this, along with BRWalletManagerCreateTransaction, to
    //            a model where they return a status code. We are currently providing no
    //            context to the caller.
    BRBitcoinTransaction *transaction = NULL;
    btcWalletSweeperCreateTransaction (sweeper, wallet, feePerKb, &transaction);

    return transaction;
}

private_extern BRCryptoTransfer
cryptoWalletSweeperCreateTransferForWalletSweepBTC (BRCryptoWalletManager cwm,
                                                    BRCryptoWallet wallet,
                                                    BRCryptoWalletSweeper sweeper,
                                                    BRCryptoFeeBasis estimatedFeeBasis) {
    BRCryptoUnit unit       = cryptoWalletGetUnit (wallet);
    BRCryptoUnit unitForFee = cryptoWalletGetUnitForFee(wallet);
    
    BRCryptoWalletBTC walletBTC = (BRCryptoWalletBTC) wallet;
    BRBitcoinWallet *wid = walletBTC->wid;
    
    BRCryptoWalletSweeperBTC sweeperBTC = cryptoWalletSweeperCoerce (sweeper);
    
    BRBitcoinTransaction *tid = cryptoWalletSweeperTransactionForSweepAsBTC (cwm,
                                                                      wid,
                                                                      sweeperBTC,
                                                                      cryptoFeeBasisAsBTC(estimatedFeeBasis));
    return (NULL != tid
            ? cryptoTransferCreateAsBTC (wallet->listenerTransfer,
                                         unit,
                                         unitForFee,
                                         wid,
                                         tid,
                                         cwm->type)
            : NULL);
}

private_extern BRCryptoWalletSweeperStatus
cryptoWalletSweeperValidateBTC (BRCryptoWalletSweeper sweeper) {
    BRCryptoWalletSweeperBTC sweeperBTC = cryptoWalletSweeperCoerce (sweeper);
    
    if (0 == array_count (sweeperBTC->txns)) {
        return CRYPTO_WALLET_SWEEPER_NO_TRANSFERS_FOUND;
    }

    if (0 == btcWalletSweeperGetBalance (sweeperBTC)) {
        return CRYPTO_WALLET_SWEEPER_INSUFFICIENT_FUNDS;
    }

    return CRYPTO_WALLET_SWEEPER_SUCCESS;
}

private_extern void
cryptoWalletSweeperReleaseBTC (BRCryptoWalletSweeper sweeper) {
    BRCryptoWalletSweeperBTC sweeperBTC = cryptoWalletSweeperCoerce (sweeper);
    
    free (sweeperBTC->sourceAddress);
    for (size_t index = 0; index < array_count(sweeperBTC->txns); index++) {
        btcTransactionFree (sweeperBTC->txns[index]);
    }
    array_free (sweeperBTC->txns);
}

// MARK: - Support

typedef struct {
    UInt256 txHash;
    uint32_t utxoIndex;
    uint8_t *script;
    size_t scriptLen;
    uint64_t amount;
} BRWalletSweeperUTXO;

inline static size_t btcWalletSweeperUTXOHash(const void *utxo)
{
    // (hash xor n)*FNV_PRIME, lifted from BRWallet's BRUTXOHash
    return (size_t)((((const BRWalletSweeperUTXO *)utxo)->txHash.u32[0] ^ ((const BRWalletSweeperUTXO *)utxo)->utxoIndex)*0x01000193);
}

inline static int btcWalletSweeperUTXOEq(const void *utxo, const void *otherUtxo)
{
    // lifted from BRWallet's BRUTXOEq
    return (utxo == otherUtxo || (UInt256Eq(((const BRWalletSweeperUTXO *)utxo)->txHash, ((const BRWalletSweeperUTXO *)otherUtxo)->txHash) &&
                                  ((const BRWalletSweeperUTXO *)utxo)->utxoIndex == ((const BRWalletSweeperUTXO *)otherUtxo)->utxoIndex));
}

inline static uint64_t btcWalletSweeperCalculateFee(uint64_t feePerKb, size_t size)
{
    // lifted from BRWallet's _txFee
    uint64_t standardFee = size*TX_FEE_PER_KB/1000,       // standard fee based on tx size
             fee = (((size*feePerKb/1000) + 99)/100)*100; // fee using feePerKb, rounded up to nearest 100 satoshi

    return (fee > standardFee) ? fee : standardFee;
}

inline static uint64_t btcWalletSweeperCalculateMinOutputAmount(uint64_t feePerKb)
{
    // lifted from BRWallet's BRWalletMinOutputAmount
    uint64_t amount = (TX_MIN_OUTPUT_AMOUNT*feePerKb + MIN_FEE_PER_KB - 1)/MIN_FEE_PER_KB;
    return (amount > TX_MIN_OUTPUT_AMOUNT) ? amount : TX_MIN_OUTPUT_AMOUNT;
}

inline static int btcWalletSweeperIsSourceInput(BRBitcoinTxInput *input, BRAddressParams addrParams, char * sourceAddress) {
    size_t addressLength = btcTxInputAddress (input, NULL, 0, addrParams);
    char * address = malloc (addressLength + 1);
    btcTxInputAddress (input, address, addressLength, addrParams);
    address[addressLength] = '\0';

    int match = 0 == strcmp (sourceAddress, address);

    free (address);
    return match;
}

inline static int btcWalletSweeperIsSourceOutput(BRBitcoinTxOutput *output, BRAddressParams addrParams, char * sourceAddress) {
    size_t addressLength = btcTxOutputAddress (output, NULL, 0, addrParams);
    char * address = malloc (addressLength + 1);
    btcTxOutputAddress (output, address, addressLength, addrParams);
    address[addressLength] = '\0';

    int match = 0 == strcmp (sourceAddress, address);

    free (address);
    return match;
}

static BRSetOf(BRWalletSweeperUTXO *)
btcWalletSweeperGetUTXOs (BRCryptoWalletSweeperBTC sweeper) {
    BRSet * utxos = BRSetNew(btcWalletSweeperUTXOHash, btcWalletSweeperUTXOEq, 100);

    // TODO(fix): This is horrible; we should be building up this knowledge as transactions are added

    // loop through and add all the unspent outputs
    for (size_t index = 0; index < array_count (sweeper->txns); index++) {
        BRBitcoinTransaction *txn = sweeper->txns[index];

        for (uint32_t i = 0; i < txn->outCount; i++) {
            if (btcWalletSweeperIsSourceOutput (&txn->outputs[i], sweeper->addrParams, sweeper->sourceAddress)) {
                BRWalletSweeperUTXO * utxo = malloc (sizeof(BRWalletSweeperUTXO));
                utxo->txHash = txn->txHash;
                utxo->utxoIndex = i;
                utxo->amount = txn->outputs[i].amount;
                utxo->script = txn->outputs[i].script;
                utxo->scriptLen = txn->outputs[i].scriptLen;

                utxo = BRSetAdd (utxos, utxo);
                if (NULL != utxo) {
                    free (utxo);
                }
            }
        }
    }

    // loop through and remove all the unspent outputs
    for (size_t index = 0; index < array_count (sweeper->txns); index++) {
        BRBitcoinTransaction *txn = sweeper->txns[index];

        for (uint32_t i = 0; i < txn->inCount; i++) {
            if (btcWalletSweeperIsSourceInput (&txn->inputs[i], sweeper->addrParams, sweeper->sourceAddress)) {
                BRWalletSweeperUTXO value = {0};
                BRWalletSweeperUTXO * utxo = &value;
                value.txHash = txn->inputs[i].txHash;
                value.utxoIndex = txn->inputs[i].index;
                // other values are not used during lookup

                utxo = BRSetRemove (utxos, utxo);
                if (NULL != utxo) {
                    free (utxo);
                }
            }
        }
    }

    return utxos;
}

static BRCryptoWalletSweeperStatus
btcWalletSweeperBuildTransaction (BRCryptoWalletSweeperBTC sweeper,
                                 BRBitcoinWallet * wallet,
                                 uint64_t feePerKb,
                                 BRBitcoinTransaction **transactionOut,
                                 uint64_t *feeAmountOut,
                                 uint64_t *balanceAmountOut) {
    uint64_t balanceAmount = 0;
    BRBitcoinTransaction *transaction = btcTransactionNew ();

    // based on BRWallet's BRWalletCreateTxForOutputs

    BRSetOf(BRWalletSweeperUTXO *) outputs = btcWalletSweeperGetUTXOs (sweeper);
    FOR_SET (BRWalletSweeperUTXO *, utxo, outputs) {
        btcTransactionAddInput(transaction,
                              utxo->txHash,
                              utxo->utxoIndex,
                              utxo->amount,
                              utxo->script,
                              utxo->scriptLen,
                              NULL,
                              0,
                              NULL,
                              0,
                              TXIN_SEQUENCE);
        balanceAmount += utxo->amount;
    }
    BRSetFreeAll (outputs, free);

    size_t txnSize = btcTransactionVSize(transaction) + TX_OUTPUT_SIZE;
    if (txnSize > TX_MAX_SIZE) {
        btcTransactionFree (transaction);
        if (transactionOut) *transactionOut = NULL;
        if (feeAmountOut) *feeAmountOut = 0;
        if (balanceAmountOut) *balanceAmountOut = 0;
        return CRYPTO_WALLET_SWEEPER_UNABLE_TO_SWEEP;
    }

    if (0 == balanceAmount) {
        btcTransactionFree (transaction);
        if (transactionOut) *transactionOut = NULL;
        if (feeAmountOut) *feeAmountOut = 0;
        if (balanceAmountOut) *balanceAmountOut = 0;
        return CRYPTO_WALLET_SWEEPER_INSUFFICIENT_FUNDS;
    }

    uint64_t feeAmount = btcWalletSweeperCalculateFee(feePerKb, txnSize);
    uint64_t minAmount = btcWalletSweeperCalculateMinOutputAmount(feePerKb);
    if ((feeAmount + minAmount) > balanceAmount) {
        btcTransactionFree (transaction);
        if (transactionOut) *transactionOut = NULL;
        if (feeAmountOut) *feeAmountOut = 0;
        if (balanceAmountOut) *balanceAmountOut = 0;
        return CRYPTO_WALLET_SWEEPER_INSUFFICIENT_FUNDS;
    }

    BRAddress addr = sweeper->isSegwit ? btcWalletReceiveAddress(wallet) : btcWalletLegacyAddress (wallet);
    BRBitcoinTxOutput o = BR_TX_OUTPUT_NONE;
    btcTxOutputSetAddress(&o, sweeper->addrParams, addr.s);
    btcTransactionAddOutput (transaction, balanceAmount - feeAmount, o.script, o.scriptLen);

    if (transactionOut) {
        *transactionOut = transaction;
    } else {
        btcTransactionFree (transaction);
    }

    if (feeAmountOut) {
        *feeAmountOut = feeAmount;
    }

    if (balanceAmountOut) {
        *balanceAmountOut = balanceAmount;
    }

    return CRYPTO_WALLET_SWEEPER_SUCCESS;
}

static BRCryptoWalletSweeperStatus
btcWalletSweeperCreateTransaction (BRCryptoWalletSweeperBTC sweeper,
                                  BRBitcoinWallet * wallet,
                                  uint64_t feePerKb,
                                  BRBitcoinTransaction **transaction) {
    return btcWalletSweeperBuildTransaction (sweeper, wallet, feePerKb, transaction, NULL, NULL);
}

static BRCryptoWalletSweeperStatus
btcWalletSweeperEstimateFee (BRCryptoWalletSweeperBTC sweeper,
                            BRBitcoinWallet * wallet,
                            uint64_t feePerKb,
                            uint64_t *feeEstimate) {
    return btcWalletSweeperBuildTransaction (sweeper, wallet, feePerKb, NULL, feeEstimate, NULL);
}

static uint64_t
btcWalletSweeperGetBalance (BRCryptoWalletSweeperBTC sweeper) {
    uint64_t balance = 0;

    BRSetOf(BRWalletSweeperUTXO *) outputs = btcWalletSweeperGetUTXOs (sweeper);
    FOR_SET (BRWalletSweeperUTXO *, utxo, outputs) {
        balance += utxo->amount;
    }
    BRSetFreeAll (outputs, free);

    return balance;
}


// MARK: -

BRCryptoWalletSweeperHandlers cryptoWalletSweeperHandlersBTC = {
    cryptoWalletSweeperReleaseBTC,
    cryptoWalletSweeperGetAddressBTC,
    cryptoWalletSweeperGetBalanceBTC,
    cryptoWalletSweeperAddTransactionFromBundleBTC,
    cryptoWalletSweeperEstimateFeeBasisForWalletSweepBTC,
    cryptoWalletSweeperCreateTransferForWalletSweepBTC,
    cryptoWalletSweeperValidateBTC
};

BRCryptoWalletSweeperHandlers cryptoWalletSweeperHandlersBCH = {
    cryptoWalletSweeperReleaseBTC,
    cryptoWalletSweeperGetAddressBTC,
    cryptoWalletSweeperGetBalanceBTC,
    cryptoWalletSweeperAddTransactionFromBundleBTC,
    cryptoWalletSweeperEstimateFeeBasisForWalletSweepBTC,
    cryptoWalletSweeperCreateTransferForWalletSweepBTC,
    cryptoWalletSweeperValidateBTC
};

BRCryptoWalletSweeperHandlers cryptoWalletSweeperHandlersBSV = {
    cryptoWalletSweeperReleaseBTC,
    cryptoWalletSweeperGetAddressBTC,
    cryptoWalletSweeperGetBalanceBTC,
    cryptoWalletSweeperAddTransactionFromBundleBTC,
    cryptoWalletSweeperEstimateFeeBasisForWalletSweepBTC,
    cryptoWalletSweeperCreateTransferForWalletSweepBTC,
    cryptoWalletSweeperValidateBTC
};
