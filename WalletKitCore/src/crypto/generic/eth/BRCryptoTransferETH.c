#include "BRCryptoETH.h"
#include "support/BRInt.h"

#include "ethereum/blockchain/BREthereumTransaction.h"
#include "ethereum/blockchain/BREthereumLog.h"

typedef enum  {
    TRANSFER_BASIS_TRANSACTION,
    TRANSFER_BASIS_LOG
} BREthereumTransferBasisType;

typedef struct {
    BREthereumTransferBasisType type;
    union {
        BREthereumTransaction transaction;
        BREthereumLog log;
    } u;
} BREthereumTransferBasis;


/// A ETH Transfer
struct BRCryptoTransferETHRecord {
    struct BRCryptoTransferRecord base;
//    BREthereumEWM ewm;
//    BREthereumTransfer tid;
    BREthereumAddress accountAddress;


//    BREthereumAddress sourceAddress;
//    BREthereumAddress targetAddress;
//    BREthereumAmount amount;
//    BREthereumFeeBasis feeBasis;
    BREthereumGas gasEstimate;
    BREthereumTransaction originatingTransaction;
    BREthereumTransferBasis basis;
    BREthereumTransferStatus status;

};

static BRCryptoTransferETH
cryptoTransferCoerce (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_ETH == transfer->type);
    return (BRCryptoTransferETH) transfer;
}

private_extern BRCryptoTransfer
cryptoTransferCreateAsETH (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
//                           BREthereumEWM ewm,
 //                          BREthereumTransfer tid,
                           BRCryptoFeeBasis feeBasisEstimated) {
    BRCryptoTransferETH transfer = (BRCryptoTransferETH)
    cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferETHRecord),
                                CRYPTO_NETWORK_TYPE_ETH,
                                unit,
                                unitForFee);

//    transfer->ewm = ewm;
//    transfer->tid = tid;
//
//    transfer->sourceAddress = cryptoAddressCreateAsETH (ewmTransferGetSource (ewm, tid));
//    transfer->targetAddress = cryptoAddressCreateAsETH (ewmTransferGetTarget (ewm, tid));

    // cache the values that require the ewm
    BREthereumAccount account = ewmGetAccount (ewm);
    transfer->accountAddress = ethAccountGetPrimaryAddress (account);

    // This function `cryptoTransferCreateAsETH()` includes an argument as
    // `BRCryptoFeeBasis feeBasisEstimated` whereas the analogous function
    // `cryptoTransferCreateAsBTC` does not.  Why is that?  For BTC the fee basis can be derived
    // 100% reliably from the BRTransaction; both the 'estimated' and 'confirmed' fee basises are
    // identical.  For ETH, the 'estimated' and the 'confirmed' basises may differ.  The difference
    // being the distinction between ETH `gasLimit` (the 'estimate') and `gasUsed` (the
    // 'confirmed').
    //
    // The EWM interface does not make this distinction clear.  It should.
    // TODO: In EWM expose 'getEstimatedFeeBasis' and 'getConfirmedFeeBasis' functions.
    //
    // Turns out that this function is called in two contexts - when Crypto creates a transfer (in
    // response to User input) and when EWM has a transfer announced (like when found in a
    // blockchain).  When Crypto creates the transfer we have the `feeBasisEstimated` and it is used
    // to create the EWM transfer.  Then EWM finds the transfer (see `cwmTransactionEventAsETH()`)
    // we don't have the estimated fee - if we did nothing the `transfer->feeBasisEstimated` field
    // would be NULL.
    //
    // Problem is we *require* one of 'estimated' or 'confirmed'.  See Transfer.swift at
    // `public var fee: Amount { ... guard let feeBasis = confirmedFeeBasis ?? estimatedFeeBasis }`
    // The 'confirmed' value is *ONLY SET* when a transfer is actually included in the blockchain;
    // therefore we need an estimated fee basis.
    //
    // Thus: if `feeBasisEstimated` is NULL, we'll take the ETH fee basis (as the best we have).

    // Get the ETH feeBasis, in the event that we need it.
    BREthereumFeeBasis ethFeeBasis = ewmTransferGetFeeBasis (ewm, tid);

    transfer->feeBasisEstimated = (NULL == feeBasisEstimated
                                   ? cryptoFeeBasisCreateAsETH (unitForFee,
                                                                ethFeeBasis.u.gas.limit,
                                                                ethFeeBasis.u.gas.price)
                                   : cryptoFeeBasisTake(feeBasisEstimated));

    return (BRCryptoTransfer) transfer;
}

static void
cryptoTransferReleaseETH (BRCryptoTransfer transfer) {
}

static BRCryptoAmount
cryptoTransferGetAmountAsSignETH (BRCryptoTransfer transferBase, BRCryptoBoolean isNegative) {
    BRCryptoTransferETH transfer = cryptoTransferCoerce(transferBase);

    uint64_t fee = transfer->u.btc.fee;
    if (UINT64_MAX == fee) fee = 0;

    uint64_t recv = transfer->u.btc.recv;
    uint64_t send = transfer->u.btc.send;

    switch (cryptoTransferGetDirection(transferBase)) {
        case CRYPTO_TRANSFER_RECOVERED:
            return cryptoAmountCreate (transferBase->unit,
                                       isNegative,
                                       uint256Create(send));


        case CRYPTO_TRANSFER_SENT:
            return cryptoAmountCreate (transferBase->unit,
                                       isNegative,
                                       uint256Create(send - fee - recv));

        case CRYPTO_TRANSFER_RECEIVED:
            return cryptoAmountCreate (transferBase->unit,
                                       isNegative,
                                       uint256Create(recv));

        default: assert(0); return UINT256_ZERO;
    }
}

static BRCryptoTransferDirection
cryptoTransferGetDirectionETH (BRCryptoTransfer transferBase) {
    BRCryptoTransferETH transfer = cryptoTransferCoerce(transferBase);

    BREthereumEWM      ewm = transfer->ewm;
    BREthereumTransfer tid = transfer->tid;

    BREthereumAddress source = ewmTransferGetSource (ewm, tid);
    BREthereumAddress target = ewmTransferGetTarget (ewm, tid);

    BREthereumBoolean accountIsSource = ethAddressEqual (source, transfer->accountAddress);
    BREthereumBoolean accountIsTarget = ethAddressEqual (target, transfer->accountAddress);

    if (accountIsSource == ETHEREUM_BOOLEAN_TRUE && accountIsTarget == ETHEREUM_BOOLEAN_TRUE) {
        return CRYPTO_TRANSFER_RECOVERED;
    } else if (accountIsSource == ETHEREUM_BOOLEAN_TRUE && accountIsTarget == ETHEREUM_BOOLEAN_FALSE) {
        return CRYPTO_TRANSFER_SENT;
    } else if (accountIsSource == ETHEREUM_BOOLEAN_FALSE && accountIsTarget == ETHEREUM_BOOLEAN_TRUE) {
        return CRYPTO_TRANSFER_RECEIVED;
    } else {
        assert(0);
    }
}

extern BRCryptoHash
cryptoTransferGetHashETH (BRCryptoTransfer transferBase) {
    BRCryptoTransferETH transfer = cryptoTransferCoerce(transferBase);
    BREthereumEWM      ewm = transfer->ewm;
    BREthereumTransfer tid = transfer->tid;

    BREthereumHash hash = ewmTransferGetOriginatingTransactionHash (ewm, tid);
    return (ETHEREUM_BOOLEAN_TRUE == ethHashEqual(hash, ethHashCreateEmpty())
            ? NULL
            : cryptoHashCreateAsETH (hash));
}

private_extern BREthereumTransfer
cryptoTransferAsETH (BRCryptoTransfer transferBase) {
    BRCryptoTransferETH transfer = cryptoTransferCoerce(transferBase);
    return transfer->tid;
}

private_extern BRCryptoBoolean
cryptoTransferHasETH (BRCryptoTransfer transferBase,
                      BREthereumTransfer eth) {
    BRCryptoTransferETH transfer = cryptoTransferCoerce(transferBase);
    return AS_CRYPTO_BOOLEAN (BLOCK_CHAIN_TYPE_ETH == transfer->type && eth == transfer->tid);
}
static int
cryptoTransferEqualAsETH (BRCryptoTransfer tb1, BRCryptoTransfer tb2) {
    BRCryptoTransferETH t1 = cryptoTransferCoerce(tb1);
    BRCryptoTransferETH t2 = cryptoTransferCoerce(tb2);

    return t1->tid == t2->tid;
}

