
#include "BRCryptoBTC.h"
#include "crypto/BRCryptoAmountP.h"
#include "ethereum/util/BRUtilMath.h"

static BRCryptoTransferDirection
cryptoTransferDirectionFromBTC (uint64_t send, uint64_t recv, uint64_t fee);

static BRCryptoAmount
cryptoTransferAmountFromBTC (BRCryptoTransferDirection direction,
                             BRCryptoUnit unit,
                             uint64_t send,
                             uint64_t recv,
                             uint64_t fee);

extern BRCryptoTransferBTC
cryptoTransferCoerceBTC (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_BTC == transfer->type ||
            CRYPTO_NETWORK_TYPE_BCH == transfer->type);
    return (BRCryptoTransferBTC) transfer;
}

private_extern OwnershipKept BRTransaction *
cryptoTransferAsBTC (BRCryptoTransfer transferBase) {
    BRCryptoTransferBTC transfer = cryptoTransferCoerceBTC(transferBase);
    return transfer->tid;
}

private_extern BRCryptoBoolean
cryptoTransferHasBTC (BRCryptoTransfer transferBase,
                      BRTransaction *btc) {
    BRCryptoTransferBTC transfer = cryptoTransferCoerceBTC(transferBase);
    return AS_CRYPTO_BOOLEAN (BRTransactionEq (btc, transfer->tid));
}

extern BRCryptoTransfer
cryptoTransferCreateAsBTC (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           OwnershipKept  BRWallet *wid,
                           OwnershipGiven BRTransaction *tid,
                           BRCryptoBlockChainType type) {
    uint64_t fee  = BRWalletFeeForTx (wid, tid);
    uint64_t send = BRWalletAmountReceivedFromTx (wid, tid);
    uint64_t recv = BRWalletAmountSentByTx (wid, tid);
    
    BRAddressParams  addressParams = BRWalletGetAddressParams (wid);

    BRCryptoTransferDirection direction = cryptoTransferDirectionFromBTC (send, recv, fee);
    
    BRCryptoAmount amount = cryptoTransferAmountFromBTC (direction, unit, send, recv, fee);
    
    BRCryptoTransfer transferBase = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferBTCRecord),
                                                                type,
                                                                unit,
                                                                unitForFee,
                                                                amount,
                                                                direction);
    BRCryptoTransferBTC transfer = cryptoTransferCoerceBTC(transferBase);

    transfer->tid  = tid;
    transfer->isResolved = BRWalletTransactionIsResolved (wid, tid);
    transfer->isDeleted  = false;
    
    // cache the values that require the wallet
    transfer->fee  = fee;
    transfer->recv = send;
    transfer->send = recv;

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
                    transferBase->sourceAddress =
                    cryptoAddressCreateAsBTC (type, BRAddressFill (addressParams, address));
                    break;
                }
            }
        }
    }

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
                    transferBase->targetAddress =
                    cryptoAddressCreateAsBTC (type, BRAddressFill (addressParams, address));
                    break;
                }
            }
        }
    }

    //
    // Currently this function, cryptoTransferCreateAsBTC(), is only called in various CWM
    // event handlers based on BTC events.  Thus for a newly created BTC transfer, the
    // BRCryptoFeeBasis is long gone.  The best we can do is reconstruct the feeBasis from the
    // BRTransaction itself.
    //
    uint32_t feePerKB = 0;  // assume not our transaction (fee == UINT64_MAX)
    uint32_t sizeInByte = (uint32_t) BRTransactionVSize (tid);

    if (UINT64_MAX != fee) {
        // round to nearest satoshi per kb
        feePerKB = (uint32_t) (((1000 * fee) + (sizeInByte/2)) / sizeInByte);
    }

    transferBase->feeBasisEstimated = cryptoFeeBasisCreateAsBTC (transferBase->unitForFee, feePerKB, sizeInByte);

    return (BRCryptoTransfer) transfer;
}

static void
cryptoTransferReleaseBTC (BRCryptoTransfer transferBase) {
    BRCryptoTransferBTC transfer = cryptoTransferCoerceBTC(transferBase);
    BRTransactionFree (transfer->tid);
}

static BRCryptoHash
cryptoTransferGetHashBTC (BRCryptoTransfer transferBase) {
    BRCryptoTransferBTC transfer = cryptoTransferCoerceBTC(transferBase);

    return (1 == UInt256IsZero(transfer->tid->txHash)
            ? NULL
            : cryptoHashCreateAsBTC (transfer->tid->txHash));
}

extern uint8_t *
cryptoTransferSerializeForSubmissionBTC (BRCryptoTransfer transferBase,
                                         BRCryptoNetwork  network,
                                         size_t *serializationCount) {
    BRTransaction *tid = cryptoTransferAsBTC     (transferBase);

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
    return BRTransactionEq (t1->tid, t2->tid);
}


static BRCryptoAmount
cryptoTransferAmountFromBTC (BRCryptoTransferDirection direction,
                             BRCryptoUnit unit,
                             uint64_t send,
                             uint64_t recv,
                             uint64_t fee) {
    if (UINT64_MAX == fee) fee = 0;
    
    switch (direction) {
        case CRYPTO_TRANSFER_RECOVERED:
            return cryptoAmountCreate (unit,
                                       CRYPTO_FALSE,
                                       uint256Create(send));
            
        case CRYPTO_TRANSFER_SENT:
            return cryptoAmountCreate (unit,
                                       CRYPTO_FALSE,
                                       uint256Create(send - fee - recv));
            
        case CRYPTO_TRANSFER_RECEIVED:
            return cryptoAmountCreate (unit,
                                       CRYPTO_FALSE,
                                       uint256Create(recv));
            break;
            
        default:
            assert(0);
            return cryptoAmountCreate (unit,
                                       CRYPTO_FALSE,
                                       UINT256_ZERO);
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


BRCryptoTransferHandlers cryptoTransferHandlersBTC = {
    cryptoTransferReleaseBTC,
    cryptoTransferGetHashBTC,
    cryptoTransferSerializeForSubmissionBTC,
    cryptoTransferIsEqualBTC
};

BRCryptoTransferHandlers cryptoTransferHandlersBCH = {
    cryptoTransferReleaseBTC,
    cryptoTransferGetHashBTC,
    cryptoTransferSerializeForSubmissionBTC,
    cryptoTransferIsEqualBTC
};
