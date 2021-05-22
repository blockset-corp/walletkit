//
//  WKWalletSweeperBTC.c
//  WalletKitCore
//
//  Created by Ehsan Rezaie on 5/22/20
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKBTC.h"
#include "walletkit/WKWalletSweeperP.h"
#include "walletkit/WKAmountP.h"
#include "support/util/BRUtilMath.h"

// MARK: Forward Declarations

static WKWalletSweeperStatus
btcWalletSweeperEstimateFee (WKWalletSweeperBTC sweeper,
                            BRBitcoinWallet * wallet,
                            uint64_t feePerKb,
                            uint64_t *feeEstimate);

static WKWalletSweeperStatus
btcWalletSweeperCreateTransaction (WKWalletSweeperBTC sweeper,
                                  BRBitcoinWallet * wallet,
                                  uint64_t feePerKb,
                                  BRBitcoinTransaction **transaction);

static uint64_t
btcWalletSweeperGetBalance (WKWalletSweeperBTC sweeper);

// MARK: - Handlers

static WKWalletSweeperBTC
wkWalletSweeperCoerce (WKWalletSweeper sweeper) {
    assert (WK_NETWORK_TYPE_BTC == sweeper->type ||
            WK_NETWORK_TYPE_BCH == sweeper->type ||
            WK_NETWORK_TYPE_BSV == sweeper->type ||
            WK_NETWORK_TYPE_LTC == sweeper->type);
    return (WKWalletSweeperBTC) sweeper;
}

private_extern WKAddress
wkWalletSweeperGetAddressBTC (WKWalletSweeper sweeper) {
    WKWalletSweeperBTC sweeperBTC = wkWalletSweeperCoerce (sweeper);
    switch (sweeper->type) {
        case WK_NETWORK_TYPE_BTC:
            return wkAddressCreateFromStringAsBTC (sweeperBTC->addrParams, sweeperBTC->sourceAddress);
        case WK_NETWORK_TYPE_BSV:
            return wkAddressCreateFromStringAsBSV (sweeperBTC->addrParams, sweeperBTC->sourceAddress);
        case WK_NETWORK_TYPE_BCH:
            return wkAddressCreateFromLegacyStringAsBCH (sweeperBTC->addrParams, sweeperBTC->sourceAddress);
        case WK_NETWORK_TYPE_LTC:
            return wkAddressCreateFromStringAsLTC(sweeperBTC->addrParams, sweeperBTC->sourceAddress);
        default:
            assert (0);
            return NULL;
    }
}

private_extern WKAmount
wkWalletSweeperGetBalanceBTC (WKWalletSweeper sweeper) {
    WKWalletSweeperBTC sweeperBTC = wkWalletSweeperCoerce (sweeper);
    UInt256 value = uint256Create (btcWalletSweeperGetBalance (sweeperBTC));
    return wkAmountCreate (sweeper->unit, WK_FALSE, value);
}

private_extern WKWalletSweeperStatus
wkWalletSweeperAddTransactionFromBundleBTC (WKWalletSweeper sweeper,
                                                OwnershipKept WKClientTransactionBundle bundle) {
    WKWalletSweeperStatus status = WK_WALLET_SWEEPER_SUCCESS;
    WKWalletSweeperBTC sweeperBTC = wkWalletSweeperCoerce (sweeper);

    BRBitcoinTransaction * txn = btcTransactionParse (bundle->serialization, bundle->serializationCount);
    if (NULL != txn) {
        array_add (sweeperBTC->txns, txn);
    } else {
        status = WK_WALLET_SWEEPER_INVALID_TRANSACTION;
    }

    return status;
}

private_extern WKFeeBasis
wkWalletSweeperEstimateFeeBasisForWalletSweepBTC (WKWalletManager cwm,
                                                      WKWallet wallet,
                                                      WKCookie cookie,
                                                      WKWalletSweeper sweeper,
                                                      WKNetworkFee networkFee) {
    BRBitcoinWallet *wid = wkWalletAsBTC (wallet);
    
    WKWalletSweeperBTC sweeperBTC = wkWalletSweeperCoerce (sweeper);
    
    uint64_t feePerKb = 1000 * wkNetworkFeeAsBTC (networkFee);
    
    // TODO(fix): We should move this, along with BRWalletManagerEstimateFeeForTransfer, to
    //            a model where they return a status code. We are currently providing no
    //            context to the caller.
    uint64_t fee = 0;
    btcWalletSweeperEstimateFee (sweeperBTC, wid, feePerKb, &fee);
    
    return wkFeeBasisCreateAsBTC (wallet->unitForFee, fee, feePerKb, WK_FEE_BASIS_BTC_SIZE_UNKNOWN);
}

static BRBitcoinTransaction *
wkWalletSweeperTransactionForSweepAsBTC (WKWalletManager manager,
                                             BRBitcoinWallet *wallet,
                                             WKWalletSweeperBTC sweeper,
                                             uint64_t feePerKb) {
    assert (wallet == wkWalletAsBTC (manager->wallet));

    pthread_mutex_lock (&manager->lock);

    // TODO(fix): We should move this, along with BRWalletManagerCreateTransaction, to
    //            a model where they return a status code. We are currently providing no
    //            context to the caller.
    BRBitcoinTransaction *transaction = NULL;
    btcWalletSweeperCreateTransaction (sweeper, wallet, feePerKb, &transaction);

    return transaction;
}

private_extern WKTransfer
wkWalletSweeperCreateTransferForWalletSweepBTC (WKWalletManager cwm,
                                                    WKWallet wallet,
                                                    WKWalletSweeper sweeper,
                                                    WKFeeBasis estimatedFeeBasis) {
    WKUnit unit       = wkWalletGetUnit (wallet);
    WKUnit unitForFee = wkWalletGetUnitForFee(wallet);
    
    WKWalletBTC walletBTC = (WKWalletBTC) wallet;
    BRBitcoinWallet *wid = walletBTC->wid;
    
    WKWalletSweeperBTC sweeperBTC = wkWalletSweeperCoerce (sweeper);
    
    BRBitcoinTransaction *tid = wkWalletSweeperTransactionForSweepAsBTC (cwm,
                                                                      wid,
                                                                      sweeperBTC,
                                                                      wkFeeBasisAsBTC(estimatedFeeBasis));
    return (NULL != tid
            ? wkTransferCreateAsBTC (wallet->listenerTransfer,
                                         unit,
                                         unitForFee,
                                         wid,
                                         tid,
                                         cwm->type)
            : NULL);
}

private_extern WKWalletSweeperStatus
wkWalletSweeperValidateBTC (WKWalletSweeper sweeper) {
    WKWalletSweeperBTC sweeperBTC = wkWalletSweeperCoerce (sweeper);
    
    if (0 == array_count (sweeperBTC->txns)) {
        return WK_WALLET_SWEEPER_NO_TRANSFERS_FOUND;
    }

    if (0 == btcWalletSweeperGetBalance (sweeperBTC)) {
        return WK_WALLET_SWEEPER_INSUFFICIENT_FUNDS;
    }

    return WK_WALLET_SWEEPER_SUCCESS;
}

private_extern void
wkWalletSweeperReleaseBTC (WKWalletSweeper sweeper) {
    WKWalletSweeperBTC sweeperBTC = wkWalletSweeperCoerce (sweeper);
    
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
btcWalletSweeperGetUTXOs (WKWalletSweeperBTC sweeper) {
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

static WKWalletSweeperStatus
btcWalletSweeperBuildTransaction (WKWalletSweeperBTC sweeper,
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
        return WK_WALLET_SWEEPER_UNABLE_TO_SWEEP;
    }

    if (0 == balanceAmount) {
        btcTransactionFree (transaction);
        if (transactionOut) *transactionOut = NULL;
        if (feeAmountOut) *feeAmountOut = 0;
        if (balanceAmountOut) *balanceAmountOut = 0;
        return WK_WALLET_SWEEPER_INSUFFICIENT_FUNDS;
    }

    uint64_t feeAmount = btcWalletSweeperCalculateFee(feePerKb, txnSize);
    uint64_t minAmount = btcWalletSweeperCalculateMinOutputAmount(feePerKb);
    if ((feeAmount + minAmount) > balanceAmount) {
        btcTransactionFree (transaction);
        if (transactionOut) *transactionOut = NULL;
        if (feeAmountOut) *feeAmountOut = 0;
        if (balanceAmountOut) *balanceAmountOut = 0;
        return WK_WALLET_SWEEPER_INSUFFICIENT_FUNDS;
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

    return WK_WALLET_SWEEPER_SUCCESS;
}

static WKWalletSweeperStatus
btcWalletSweeperCreateTransaction (WKWalletSweeperBTC sweeper,
                                  BRBitcoinWallet * wallet,
                                  uint64_t feePerKb,
                                  BRBitcoinTransaction **transaction) {
    return btcWalletSweeperBuildTransaction (sweeper, wallet, feePerKb, transaction, NULL, NULL);
}

static WKWalletSweeperStatus
btcWalletSweeperEstimateFee (WKWalletSweeperBTC sweeper,
                            BRBitcoinWallet * wallet,
                            uint64_t feePerKb,
                            uint64_t *feeEstimate) {
    return btcWalletSweeperBuildTransaction (sweeper, wallet, feePerKb, NULL, feeEstimate, NULL);
}

static uint64_t
btcWalletSweeperGetBalance (WKWalletSweeperBTC sweeper) {
    uint64_t balance = 0;

    BRSetOf(BRWalletSweeperUTXO *) outputs = btcWalletSweeperGetUTXOs (sweeper);
    FOR_SET (BRWalletSweeperUTXO *, utxo, outputs) {
        balance += utxo->amount;
    }
    BRSetFreeAll (outputs, free);

    return balance;
}


// MARK: -

WKWalletSweeperHandlers wkWalletSweeperHandlersBTC = {
    wkWalletSweeperReleaseBTC,
    wkWalletSweeperGetAddressBTC,
    wkWalletSweeperGetBalanceBTC,
    wkWalletSweeperAddTransactionFromBundleBTC,
    wkWalletSweeperEstimateFeeBasisForWalletSweepBTC,
    wkWalletSweeperCreateTransferForWalletSweepBTC,
    wkWalletSweeperValidateBTC
};

WKWalletSweeperHandlers wkWalletSweeperHandlersBCH = {
    wkWalletSweeperReleaseBTC,
    wkWalletSweeperGetAddressBTC,
    wkWalletSweeperGetBalanceBTC,
    wkWalletSweeperAddTransactionFromBundleBTC,
    wkWalletSweeperEstimateFeeBasisForWalletSweepBTC,
    wkWalletSweeperCreateTransferForWalletSweepBTC,
    wkWalletSweeperValidateBTC
};

WKWalletSweeperHandlers wkWalletSweeperHandlersBSV = {
    wkWalletSweeperReleaseBTC,
    wkWalletSweeperGetAddressBTC,
    wkWalletSweeperGetBalanceBTC,
    wkWalletSweeperAddTransactionFromBundleBTC,
    wkWalletSweeperEstimateFeeBasisForWalletSweepBTC,
    wkWalletSweeperCreateTransferForWalletSweepBTC,
    wkWalletSweeperValidateBTC
};

WKWalletSweeperHandlers wkWalletSweeperHandlersLTC = {
    wkWalletSweeperReleaseBTC,
    wkWalletSweeperGetAddressBTC,
    wkWalletSweeperGetBalanceBTC,
    wkWalletSweeperAddTransactionFromBundleBTC,
    wkWalletSweeperEstimateFeeBasisForWalletSweepBTC,
    wkWalletSweeperCreateTransferForWalletSweepBTC,
    wkWalletSweeperValidateBTC
};
