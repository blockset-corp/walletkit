//
//  WKTransferBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKBTC.h"
#include "walletkit/WKAmountP.h"
#include "support/util/BRUtilMath.h"

static WKTransferDirection
wkTransferDirectionFromBTC (uint64_t send, uint64_t recv, uint64_t fee);

static uint64_t
wkTransferComputeAmountBTC (WKTransferDirection direction,
                             uint64_t send,
                             uint64_t recv,
                             uint64_t fee);

extern WKTransferBTC
wkTransferCoerceBTC (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE_BTC == transfer->type ||
            WK_NETWORK_TYPE_BCH == transfer->type ||
            WK_NETWORK_TYPE_BSV == transfer->type);
    return (WKTransferBTC) transfer;
}

private_extern OwnershipKept BRBitcoinTransaction *
wkTransferAsBTC (WKTransfer transfer) {
    WKTransferBTC transferBTC = wkTransferCoerceBTC(transfer);
    return transferBTC->tid;
}

private_extern WKBoolean
wkTransferHasBTC (WKTransfer transfer,
                      BRBitcoinTransaction *btc) {
    WKTransferBTC transferBTC = wkTransferCoerceBTC(transfer);
    return AS_WK_BOOLEAN (btcTransactionEq (btc, transferBTC->tid));
}

typedef struct {
    BRBitcoinTransaction *tid;

    bool isDeleted;

    uint64_t fee;
    uint64_t send;
    uint64_t recv;

} WKTransferCreateContextBTC;

static void
wkTransferCreateCallbackBTC (WKTransferCreateContext context,
                                    WKTransfer transfer) {
    WKTransferCreateContextBTC *contextBTC = (WKTransferCreateContextBTC*) context;
    WKTransferBTC transferBTC = wkTransferCoerceBTC (transfer);

    transferBTC->tid  = contextBTC->tid;

    transferBTC->isDeleted  = contextBTC->isDeleted;

    // cache the values that require the wallet
    transferBTC->fee  = contextBTC->fee;
    transferBTC->send = contextBTC->send;
    transferBTC->recv = contextBTC->recv;
}

extern WKTransfer
wkTransferCreateAsBTC (WKTransferListener listener,
                           WKUnit unit,
                           WKUnit unitForFee,
                           OwnershipKept  BRBitcoinWallet *wid,
                           OwnershipGiven BRBitcoinTransaction *tid,
                           WKNetworkType type) {
    uint64_t fee  = btcWalletFeeForTx (wid, tid);
    uint64_t recv = btcWalletAmountReceivedFromTx (wid, tid);
    uint64_t send = btcWalletAmountSentByTx (wid, tid);
    
    BRAddressParams  addressParams = btcWalletGetAddressParams (wid);

    WKTransferDirection direction = wkTransferDirectionFromBTC (send, recv, fee);
    
    WKAmount amount = wkAmountCreate (unit,
                                                WK_FALSE,
                                                uint256Create (wkTransferComputeAmountBTC (direction, send, recv, fee)));

    WKAddress sourceAddress = NULL;
    {
        size_t     inputsCount = tid->inCount;
        BRBitcoinTxInput *inputs      = tid->inputs;

        // If we receive the transfer, then we won't be the source address.
        int inputsContain = (WK_TRANSFER_RECEIVED != direction);

        for (size_t index = 0; index < inputsCount; index++) {
            size_t addressSize = btcTxInputAddress (&inputs[index], NULL, 0, addressParams);

            // ensure address fits in a BRAddress struct, which adds a nul-terminator
            assert (addressSize < sizeof (BRAddress));
            if (0 != addressSize && addressSize < sizeof (BRAddress)) {
                char address [addressSize + 1];
                btcTxInputAddress (&inputs[index], address, addressSize, addressParams);
                address [addressSize] = '\0'; // ensure address is nul-terminated

                if (inputsContain == btcWalletContainsAddress(wid, address)) {
                    sourceAddress =
                    wkAddressCreateAsBTC (type, BRAddressFill (addressParams, address));
                    break;
                }
            }
        }
    }

    WKAddress targetAddress = NULL;
    {
        size_t      outputsCount = tid->outCount;
        BRBitcoinTxOutput *outputs      = tid->outputs;

        // If we sent the transfer, then we won't be the target address.
        int outputsContain = (WK_TRANSFER_SENT != direction);

        for (size_t index = 0; index < outputsCount; index++) {
            size_t addressSize = btcTxOutputAddress (&outputs[index], NULL, 0, addressParams);

            // ensure address fits in a BRAddress struct, which adds a nul-terminator
            assert (addressSize < sizeof (BRAddress));
            if (0 != addressSize && addressSize < sizeof (BRAddress)) {
                // There will be no targetAddress if we send the amount to ourselves.  In that
                // case `outputsContain = 0` and every output is our own address and thus 1 is always
                // returned by `btcWalletContainsAddress()`
                char address [addressSize + 1];
                btcTxOutputAddress (&outputs[index], address, addressSize, addressParams);
                address [addressSize] = '\0'; // ensure address is nul-terminated

                if (outputsContain == btcWalletContainsAddress(wid, address)) {
                    targetAddress =
                    wkAddressCreateAsBTC (type, BRAddressFill (addressParams, address));
                    break;
                }
            }
        }
    }


    // Currently this function, wkTransferCreateAsBTC(), is only called in various CWM
    // event handlers based on BTC events.  Thus for a newly created BTC transfer, the
    // WKFeeBasis is long gone.  The best we can do is reconstruct the feeBasis from the
    // BRBitcoinTransaction itself.

    WKFeeBasis feeBasisEstimated =
    wkFeeBasisCreateAsBTC (unitForFee,
                               (fee == UINT64_MAX ? WK_FEE_BASIS_BTC_FEE_UNKNOWN        : fee),
                               (fee == UINT64_MAX ? 0                                       : WK_FEE_BASIS_BTC_FEE_PER_KB_UNKNOWN),
                               (uint32_t) btcTransactionVSize (tid));

    WKTransferCreateContextBTC contextBTC = {
        tid,
        false,
        fee,
        send,
        recv
    };

    WKTransferState state = wkTransferInitializeStateBTC (tid,
                                                                    tid->blockHeight,
                                                                    tid->timestamp,
                                                                    feeBasisEstimated);

    WKTransfer transfer = wkTransferAllocAndInit (sizeof (struct WKTransferBTCRecord),
                                                            type,
                                                            listener,
                                                            unit,
                                                            unitForFee,
                                                            feeBasisEstimated,
                                                            amount,
                                                            direction,
                                                            sourceAddress,
                                                            targetAddress,
                                                            state,
                                                            &contextBTC,
                                                            wkTransferCreateCallbackBTC);

    wkTransferStateGive (state);
    wkFeeBasisGive (feeBasisEstimated);
    wkAmountGive  (amount);
    wkAddressGive (sourceAddress);
    wkAddressGive (targetAddress);

    return transfer;
}

static void
wkTransferReleaseBTC (WKTransfer transfer) {
    WKTransferBTC transferBTC = wkTransferCoerceBTC(transfer);
    btcTransactionFree (transferBTC->tid);
}

private_extern WKBoolean
wkTransferChangedAmountBTC (WKTransfer transfer,
                                BRBitcoinWallet *wid) {
    WKTransferBTC transferBTC = wkTransferCoerceBTC(transfer);
    BRBitcoinTransaction *tid = transferBTC->tid;

    uint64_t fee  = btcWalletFeeForTx (wid, tid);
    uint64_t send = btcWalletAmountSentByTx (wid, tid);
    uint64_t recv = btcWalletAmountReceivedFromTx (wid, tid);

    // amount (and direction) are 100% determined by { fee, recv, send}.  See
    // wkTransferDirectionFromBTC() and wkTransferComputeAmountBTC()
    return AS_WK_BOOLEAN (fee  != transferBTC->fee  ||
                              send != transferBTC->send ||
                              recv != transferBTC->recv) ;
}

static WKHash
wkTransferGetHashBTC (WKTransfer transfer) {
    WKTransferBTC transferBTC = wkTransferCoerceBTC(transfer);

    return (1 == UInt256IsZero(transferBTC->tid->txHash)
            ? NULL
            : wkHashCreateAsBTC (transferBTC->tid->txHash));
}

extern uint8_t *
wkTransferSerializeBTC (WKTransfer transfer,
                            WKNetwork  network,
                            WKBoolean  requireSignature,
                            size_t *serializationCount) {
    assert (WK_TRUE == requireSignature);
    BRBitcoinTransaction *tid = wkTransferAsBTC     (transfer);

    if (NULL == tid) { *serializationCount = 0; return NULL; }

    *serializationCount = btcTransactionSerialize (tid, NULL, 0);
    uint8_t *serialization = malloc (*serializationCount);

    btcTransactionSerialize (tid, serialization, *serializationCount);
    return serialization;
}

static int
wkTransferIsEqualBTC (WKTransfer tb1, WKTransfer tb2) {
    WKTransferBTC t1 = wkTransferCoerceBTC(tb1);
    WKTransferBTC t2 = wkTransferCoerceBTC(tb2);

    // This does not compare the properties of `t1` to `t2`, just the 'id-ness'.  If the properties
    // are compared, one needs to be careful about the BRTransaction's timestamp.  Two transactions
    // with an identical hash can have different timestamps depending on how the transaction
    // is identified.  Specifically P2P and API found transactions *will* have different timestamps.
    return t1 == t2 || btcTransactionEq (t1->tid, t2->tid);
}

static uint64_t
wkTransferComputeAmountBTC (WKTransferDirection direction,
                             uint64_t send,
                             uint64_t recv,
                             uint64_t fee) {
    if (UINT64_MAX == fee) fee = 0;

    switch (direction) {
        case WK_TRANSFER_RECOVERED:
            return send;

        case WK_TRANSFER_SENT:
            return send - fee - recv;

        case WK_TRANSFER_RECEIVED:
            return recv;

        default:
            assert(0);
            return 0;
    }
}

static WKTransferDirection
wkTransferDirectionFromBTC (uint64_t send, uint64_t recv, uint64_t fee) {
    if (UINT64_MAX == fee) fee = 0;

    return (0 == send
            ? WK_TRANSFER_RECEIVED
            : ((send - fee) == recv
               ? WK_TRANSFER_RECOVERED
               : ((send - fee) > recv
                  ? WK_TRANSFER_SENT
                  : WK_TRANSFER_RECEIVED)));
}

private_extern OwnershipGiven WKTransferState
wkTransferInitializeStateBTC (BRBitcoinTransaction *tid,
                                  uint64_t blockNumber,
                                  uint64_t blockTimestamp,
                                  OwnershipKept WKFeeBasis feeBasis) {
    // Our Transfer flow is such that we want P2P and API modes to pass through the BRBitcoinWallet
    // callbacks, specifically TxAdded and TxUpdated.  Those functions don't have the resolution
    // to State that WKTransfers have (rightly or wrongly).  For BTC/etc a non-TX_CONFIRMED
    // blockNumber guarantees 'INCLUDED'.  A signed transaction does not guarantee SUBMITTED but
    // in practice we sign-and-submit atomically.  I don't believe we ever get an error state for
    // BTC/etc from Blockset.  Thus besides INCLUDED and SUBMITTED, we'll use CREATED.
    return (TX_UNCONFIRMED != blockNumber
            ? wkTransferStateIncludedInit (blockNumber,
                                               0,
                                               blockTimestamp,
                                               feeBasis,
                                               WK_TRUE,
                                               NULL)
            : (btcTransactionIsSigned (tid)
               ? wkTransferStateInit (WK_TRANSFER_STATE_SUBMITTED)  // Optimistic
               : wkTransferStateInit (WK_TRANSFER_STATE_CREATED)));
}

WKTransferHandlers wkTransferHandlersBTC = {
    wkTransferReleaseBTC,
    wkTransferGetHashBTC,
    NULL, // setHash
    NULL, // updateIdentifier
    wkTransferSerializeBTC,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqualBTC
};

WKTransferHandlers wkTransferHandlersBCH = {
    wkTransferReleaseBTC,
    wkTransferGetHashBTC,
    NULL, // setHash
    NULL, // updateIdentifier
    wkTransferSerializeBTC,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqualBTC
};

WKTransferHandlers wkTransferHandlersBSV = {
    wkTransferReleaseBTC,
    wkTransferGetHashBTC,
    NULL, // setHash
    NULL, // updateIdentifier
   wkTransferSerializeBTC,
    NULL, // getBytesForFeeEstimate
    wkTransferIsEqualBTC
};
