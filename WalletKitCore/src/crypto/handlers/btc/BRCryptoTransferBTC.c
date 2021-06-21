//
//  BRCryptoTransferBTC.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoBTC.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtilMath.h"

static BRCryptoTransferDirection
cryptoTransferDirectionFromBTC (uint64_t send, uint64_t recv, uint64_t fee);

static uint64_t
cryptoTransferComputeAmountBTC (BRCryptoTransferDirection direction,
                             uint64_t send,
                             uint64_t recv,
                             uint64_t fee);

extern BRCryptoTransferBTC
cryptoTransferCoerceBTC (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_BTC == transfer->type ||
            CRYPTO_NETWORK_TYPE_BCH == transfer->type ||
            CRYPTO_NETWORK_TYPE_BSV == transfer->type);
    return (BRCryptoTransferBTC) transfer;
}

private_extern OwnershipKept BRTransaction *
cryptoTransferAsBTC (BRCryptoTransfer transfer) {
    BRCryptoTransferBTC transferBTC = cryptoTransferCoerceBTC(transfer);
    return transferBTC->tid;
}

private_extern BRCryptoBoolean
cryptoTransferHasBTC (BRCryptoTransfer transfer,
                      BRTransaction *btc) {
    BRCryptoTransferBTC transferBTC = cryptoTransferCoerceBTC(transfer);
    return AS_CRYPTO_BOOLEAN (BRTransactionEq (btc, transferBTC->tid));
}

typedef struct {
    BRTransaction *tid;

    bool isDeleted;

    uint64_t fee;
    uint64_t send;
    uint64_t recv;

} BRCryptoTransferCreateContextBTC;

static void
cryptoTransferCreateCallbackBTC (BRCryptoTransferCreateContext context,
                                    BRCryptoTransfer transfer) {
    BRCryptoTransferCreateContextBTC *contextBTC = (BRCryptoTransferCreateContextBTC*) context;
    BRCryptoTransferBTC transferBTC = cryptoTransferCoerceBTC (transfer);

    transferBTC->tid  = contextBTC->tid;

    transferBTC->isDeleted  = contextBTC->isDeleted;

    // cache the values that require the wallet
    transferBTC->fee  = contextBTC->fee;
    transferBTC->send = contextBTC->send;
    transferBTC->recv = contextBTC->recv;
}

extern BRCryptoTransfer
cryptoTransferCreateAsBTC (BRCryptoTransferListener listener,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           OwnershipKept  BRWallet *wid,
                           OwnershipGiven BRTransaction *tid,
                           BRCryptoBlockChainType type) {
    uint64_t fee  = BRWalletFeeForTx (wid, tid);
    uint64_t recv = BRWalletAmountReceivedFromTx (wid, tid);
    uint64_t send = BRWalletAmountSentByTx (wid, tid);
    
    BRAddressParams  addressParams = BRWalletGetAddressParams (wid);

    BRCryptoTransferDirection direction = cryptoTransferDirectionFromBTC (send, recv, fee);
    
    BRCryptoAmount amount = cryptoAmountCreate (unit,
                                                CRYPTO_FALSE,
                                                uint256Create (cryptoTransferComputeAmountBTC (direction, send, recv, fee)));

    BRCryptoAddress sourceAddress = NULL;
    {
        size_t     inputsCount = tid->inCount;
        BRTxInput *inputs      = tid->inputs;

        // If we receive the transfer, then we won't be the source address.
        int inputsContain = (CRYPTO_TRANSFER_RECEIVED != direction);

        for (size_t index = 0; index < inputsCount; index++) {
            size_t addressSize = BRTxInputAddress (&inputs[index], NULL, 0, addressParams);

            // ensure address fits in a BRAddress struct, which adds a nul-terminator
            assert (addressSize < sizeof (BRAddress));
            if (0 != addressSize && addressSize < sizeof (BRAddress)) {
                char address [addressSize + 1];
                BRTxInputAddress (&inputs[index], address, addressSize, addressParams);
                address [addressSize] = '\0'; // ensure address is nul-terminated

                if (inputsContain == BRWalletContainsAddress(wid, address)) {
                    sourceAddress =
                    cryptoAddressCreateAsBTC (type, BRAddressFill (addressParams, address));
                    break;
                }
            }
        }
    }

    BRCryptoAddress targetAddress = NULL;
    {
        size_t      outputsCount = tid->outCount;
        BRTxOutput *outputs      = tid->outputs;

        // If we sent the transfer, then we won't be the target address.
        int outputsContain = (CRYPTO_TRANSFER_SENT != direction);

        for (size_t index = 0; index < outputsCount; index++) {
            size_t addressSize = BRTxOutputAddress (&outputs[index], NULL, 0, addressParams);

            // ensure address fits in a BRAddress struct, which adds a nul-terminator
            assert (addressSize < sizeof (BRAddress));
            if (0 != addressSize && addressSize < sizeof (BRAddress)) {
                // There will be no targetAddress if we send the amount to ourselves.  In that
                // case `outputsContain = 0` and every output is our own address and thus 1 is always
                // returned by `BRWalletContainsAddress()`
                char address [addressSize + 1];
                BRTxOutputAddress (&outputs[index], address, addressSize, addressParams);
                address [addressSize] = '\0'; // ensure address is nul-terminated

                if (outputsContain == BRWalletContainsAddress(wid, address)) {
                    targetAddress =
                    cryptoAddressCreateAsBTC (type, BRAddressFill (addressParams, address));
                    break;
                }
            }
        }
    }


    // Currently this function, cryptoTransferCreateAsBTC(), is only called in various CWM
    // event handlers based on BTC events.  Thus for a newly created BTC transfer, the
    // BRCryptoFeeBasis is long gone.  The best we can do is reconstruct the feeBasis from the
    // BRTransaction itself.

    BRCryptoFeeBasis feeBasisEstimated =
    cryptoFeeBasisCreateAsBTC (unitForFee,
                               (fee == UINT64_MAX ? CRYPTO_FEE_BASIS_BTC_FEE_UNKNOWN        : fee),
                               (fee == UINT64_MAX ? 0                                       : CRYPTO_FEE_BASIS_BTC_FEE_PER_KB_UNKNOWN),
                               (uint32_t) BRTransactionVSize (tid));

    BRCryptoTransferCreateContextBTC contextBTC = {
        tid,
        false,
        fee,
        send,
        recv
    };

    BRCryptoTransferState state = cryptoTransferInitializeStateBTC (tid,
                                                                    tid->blockHeight,
                                                                    tid->timestamp,
                                                                    feeBasisEstimated);

    char *uids = NULL;
    if (BRTransactionIsSigned(tid)) asprintf (&uids, "%s:0", u256hex(tid->txHash));

    BRCryptoTransfer transfer = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferBTCRecord),
                                                            type,
                                                            listener,
                                                            uids,
                                                            unit,
                                                            unitForFee,
                                                            feeBasisEstimated,
                                                            amount,
                                                            direction,
                                                            sourceAddress,
                                                            targetAddress,
                                                            state,
                                                            &contextBTC,
                                                            cryptoTransferCreateCallbackBTC);

    if (NULL != uids) free (uids);
    cryptoTransferStateGive (state);
    cryptoFeeBasisGive (feeBasisEstimated);
    cryptoAmountGive  (amount);
    cryptoAddressGive (sourceAddress);
    cryptoAddressGive (targetAddress);

    return transfer;
}

static void
cryptoTransferReleaseBTC (BRCryptoTransfer transfer) {
    BRCryptoTransferBTC transferBTC = cryptoTransferCoerceBTC(transfer);
    BRTransactionFree (transferBTC->tid);
}

private_extern BRCryptoBoolean
cryptoTransferChangedAmountBTC (BRCryptoTransfer transfer,
                                BRWallet *wid) {
    BRCryptoTransferBTC transferBTC = cryptoTransferCoerceBTC(transfer);
    BRTransaction *tid = transferBTC->tid;

    uint64_t fee  = BRWalletFeeForTx (wid, tid);
    uint64_t send = BRWalletAmountSentByTx (wid, tid);
    uint64_t recv = BRWalletAmountReceivedFromTx (wid, tid);

    // amount (and direction) are 100% determined by { fee, recv, send}.  See
    // cryptoTransferDirectionFromBTC() and cryptoTransferComputeAmountBTC()
    return AS_CRYPTO_BOOLEAN (fee  != transferBTC->fee  ||
                              send != transferBTC->send ||
                              recv != transferBTC->recv) ;
}

static BRCryptoHash
cryptoTransferGetHashBTC (BRCryptoTransfer transfer) {
    BRCryptoTransferBTC transferBTC = cryptoTransferCoerceBTC(transfer);

    return (1 == UInt256IsZero(transferBTC->tid->txHash)
            ? NULL
            : cryptoHashCreateAsBTC (transferBTC->tid->txHash));
}

extern uint8_t *
cryptoTransferSerializeBTC (BRCryptoTransfer transfer,
                            BRCryptoNetwork  network,
                            BRCryptoBoolean  requireSignature,
                            size_t *serializationCount) {
    assert (CRYPTO_TRUE == requireSignature);
    BRTransaction *tid = cryptoTransferAsBTC     (transfer);

    if (NULL == tid) { *serializationCount = 0; return NULL; }

    *serializationCount = BRTransactionSerialize (tid, NULL, 0);
    uint8_t *serialization = malloc (*serializationCount);

    BRTransactionSerialize (tid, serialization, *serializationCount);
    return serialization;
}

static int
cryptoTransferIsEqualBTC (BRCryptoTransfer tb1, BRCryptoTransfer tb2) {
    BRCryptoTransferBTC t1 = cryptoTransferCoerceBTC(tb1);
    BRCryptoTransferBTC t2 = cryptoTransferCoerceBTC(tb2);

    // This does not compare the properties of `t1` to `t2`, just the 'id-ness'.  If the properties
    // are compared, one needs to be careful about the BRTransaction's timestamp.  Two transactions
    // with an identical hash can have different timestamps depending on how the transaction
    // is identified.  Specifically P2P and API found transactions *will* have different timestamps.
    return t1 == t2 || BRTransactionEq (t1->tid, t2->tid);
}

static uint64_t
cryptoTransferComputeAmountBTC (BRCryptoTransferDirection direction,
                             uint64_t send,
                             uint64_t recv,
                             uint64_t fee) {
    if (UINT64_MAX == fee) fee = 0;

    switch (direction) {
        case CRYPTO_TRANSFER_RECOVERED:
            return send;

        case CRYPTO_TRANSFER_SENT:
            return send - fee - recv;

        case CRYPTO_TRANSFER_RECEIVED:
            return recv;

        default:
            assert(0);
            return 0;
    }
}

static BRCryptoTransferDirection
cryptoTransferDirectionFromBTC (uint64_t send, uint64_t recv, uint64_t fee) {
    if (UINT64_MAX == fee) fee = 0;

    return (0 == send
            ? CRYPTO_TRANSFER_RECEIVED
            : ((send - fee) == recv
               ? CRYPTO_TRANSFER_RECOVERED
               : ((send - fee) > recv
                  ? CRYPTO_TRANSFER_SENT
                  : CRYPTO_TRANSFER_RECEIVED)));
}

private_extern OwnershipGiven BRCryptoTransferState
cryptoTransferInitializeStateBTC (BRTransaction *tid,
                                  uint64_t blockNumber,
                                  uint64_t blockTimestamp,
                                  OwnershipKept BRCryptoFeeBasis feeBasis) {
    // Our Transfer flow is such that we want P2P and API modes to pass through the BRWallet
    // callbacks, specifically TxAdded and TxUpdated.  Those functions don't have the resolution
    // to State that BRCryptoTransfers have (rightly or wrongly).  For BTC/etc a non-TX_CONFIRMED
    // blockNumber guarantees 'INCLUDED'.  A signed transaction does not guarantee SUBMITTED but
    // in practice we sign-and-submit atomically.  I don't believe we ever get an error state for
    // BTC/etc from Blockset.  Thus besides INCLUDED and SUBMITTED, we'll use CREATED.
    return (TX_UNCONFIRMED != blockNumber
            ? cryptoTransferStateIncludedInit (blockNumber,
                                               0,
                                               blockTimestamp,
                                               feeBasis,
                                               CRYPTO_TRUE,
                                               NULL)
            : (BRTransactionIsSigned (tid)
               ? cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SUBMITTED)  // Optimistic
               : cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_CREATED)));
}

BRCryptoTransferHandlers cryptoTransferHandlersBTC = {
    cryptoTransferReleaseBTC,
    cryptoTransferGetHashBTC,
    NULL, // setHash
    NULL, // updateIdentifier
    cryptoTransferSerializeBTC,
    NULL, // getBytesForFeeEstimate
    cryptoTransferIsEqualBTC
};

BRCryptoTransferHandlers cryptoTransferHandlersBCH = {
    cryptoTransferReleaseBTC,
    cryptoTransferGetHashBTC,
    NULL, // setHash
    NULL, // updateIdentifier
    cryptoTransferSerializeBTC,
    NULL, // getBytesForFeeEstimate
    cryptoTransferIsEqualBTC
};

BRCryptoTransferHandlers cryptoTransferHandlersBSV = {
    cryptoTransferReleaseBTC,
    cryptoTransferGetHashBTC,
    NULL, // setHash
    NULL, // updateIdentifier
   cryptoTransferSerializeBTC,
    NULL, // getBytesForFeeEstimate
    cryptoTransferIsEqualBTC
};
