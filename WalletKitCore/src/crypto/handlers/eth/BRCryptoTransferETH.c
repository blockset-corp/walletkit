//
//  BRCryptoTransferETH.c
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoETH.h"
#include "support/BRInt.h"
#include "crypto/BRCryptoAmountP.h"

#include "ethereum/blockchain/BREthereumTransaction.h"
#include "ethereum/blockchain/BREthereumLog.h"

static BRCryptoTransferDirection
cryptoTransferFindDirection (BREthereumAccount account,
                             BREthereumAddress source,
                             BREthereumAddress target);

extern BRCryptoTransferETH
cryptoTransferCoerce (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_ETH == transfer->type);
    return (BRCryptoTransferETH) transfer;
}

#if 0
private_extern BRCryptoTransfer
cryptoTransferCreateAsETH (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
//                           BREthereumEWM ewm,
//                           BREthereumTransfer tid,
                           BRCryptoFeeBasis feeBasisEstimated) {
    BRCryptoTransfer transferBase = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferETHRecord),
                                                                CRYPTO_NETWORK_TYPE_ETH,
                                                                unit,
                                                                unitForFee);
    BRCryptoTransferETH transfer = cryptoTransferCoerce (transferBase);

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
#endif

extern BRCryptoTransfer
cryptoTransferCreateAsETH (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRCryptoFeeBasis feeBasisEstimated,
                           BRCryptoAmount amount,
                           BRCryptoTransferDirection direction,
                           BRCryptoAddress sourceAddress,
                           BRCryptoAddress targetAddress,
                           BREthereumAccount account,
                           BREthereumTransferBasisType type,
                           OwnershipGiven BREthereumTransaction originatingTransaction) {
    BRCryptoTransfer transferBase = cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferETHRecord),
                                                                 CRYPTO_NETWORK_TYPE_ETH,
                                                                 unit,
                                                                 unitForFee,
                                                                feeBasisEstimated,
                                                                amount,
                                                                direction,
                                                                sourceAddress,
                                                                targetAddress);
     BRCryptoTransferETH transfer = cryptoTransferCoerce (transferBase);

    transfer->account = account;
    transfer->type = type;
    transfer->originatingTransaction = originatingTransaction;

    return transferBase;
}

extern BRCryptoTransfer
cryptoTransferCreateWithTransactionAsETH (BRCryptoUnit unit,
                                          BRCryptoUnit unitForFee,
                                          BREthereumAccount account,
                                          OwnershipGiven BREthereumTransaction ethTransaction) {

    BRCryptoTransferDirection direction = cryptoTransferFindDirection (account,
                                                                       transactionGetSourceAddress (ethTransaction),
                                                                       transactionGetTargetAddress (ethTransaction));
    BREthereumEther ethAmount = transactionGetAmount(ethTransaction);
    BRCryptoAmount  amount    = cryptoAmountCreate (unit, CRYPTO_FALSE, ethEtherGetValue (ethAmount, WEI));

    BRCryptoFeeBasis estimatedFeeBasis = cryptoFeeBasisCreateAsETH (unitForFee, transactionGetFeeBasisLimit(ethTransaction));
    BRCryptoAddress  source = cryptoAddressCreateAsETH (transactionGetSourceAddress (ethTransaction));
    BRCryptoAddress  target = cryptoAddressCreateAsETH (transactionGetTargetAddress (ethTransaction));

    BRCryptoTransfer transferBase = cryptoTransferCreateAsETH (unit,
                                                               unitForFee,
                                                               estimatedFeeBasis,
                                                               amount,
                                                               direction,
                                                               source,
                                                               target,
                                                               account,
                                                               TRANSFER_BASIS_TRANSACTION,
                                                               NULL);
    BRCryptoTransferETH transfer = cryptoTransferCoerce (transferBase);

    transfer->basis.transaction = ethTransaction;

    cryptoFeeBasisGive(estimatedFeeBasis);
    cryptoAddressGive(source);
    cryptoAddressGive(target);

    return transferBase;
}

extern BRCryptoTransfer
cryptoTransferCreateWithLogAsETH (BRCryptoUnit unit,
                                  BRCryptoUnit unitForFee,
                                  BREthereumAccount account,
                                  UInt256 ethAmount,
                                  OwnershipGiven BREthereumLog ethLog) {
    BREthereumAddress ethSource = logTopicAsAddress(logGetTopic(ethLog, 1));
    BREthereumAddress ethTarget = logTopicAsAddress(logGetTopic(ethLog, 2));

    BRCryptoTransferDirection direction = cryptoTransferFindDirection (account, ethSource, ethTarget);

    BRCryptoAmount  amount    = cryptoAmountCreate (unit, CRYPTO_FALSE, ethAmount);

    BREthereumFeeBasis ethFeeBasis = ethFeeBasisCreate (ethGasCreate(0),
                                                        ethGasPriceCreate(ethEtherCreateZero()));
    BRCryptoFeeBasis estimatedFeeBasis = cryptoFeeBasisCreateAsETH(unitForFee, ethFeeBasis);

    BRCryptoAddress  source = cryptoAddressCreateAsETH (ethSource);
    BRCryptoAddress  target = cryptoAddressCreateAsETH (ethTarget);

    BRCryptoTransfer transferBase = cryptoTransferCreateAsETH (unit,
                                                               unitForFee,
                                                               estimatedFeeBasis,
                                                               amount,
                                                               direction,
                                                               source,
                                                               target,
                                                               account,
                                                               TRANSFER_BASIS_LOG,
                                                               NULL);
    BRCryptoTransferETH transfer = cryptoTransferCoerce (transferBase);

    transfer->basis.log = ethLog;

    //        // Only at this point do we know that log->data is a number.
    //        BRRlpItem  item  = rlpDataGetItem (coder, logGetDataShared(log));
    //        UInt256 value = rlpDecodeUInt256(coder, item, 1);
    //        rlpItemRelease (coder, item);
    //
    //        BREthereumAmount  amount = ethAmountCreateToken (ethTokenQuantityCreate(token, value));

    cryptoAddressGive(source);
    cryptoAddressGive(target);

    return transferBase;
}

static void
cryptoTransferReleaseETH (BRCryptoTransfer transferBase) {
    BRCryptoTransferETH transfer = cryptoTransferCoerce (transferBase);


    if (NULL != transfer->originatingTransaction)
        transactionRelease(transfer->originatingTransaction);

    switch (transfer->type) {
        case TRANSFER_BASIS_TRANSACTION:
            transactionRelease (transfer->basis.transaction);
            break;

        case TRANSFER_BASIS_LOG:
            logRelease (transfer->basis.log);
            break;
    }
}

static BRCryptoTransferDirection
cryptoTransferFindDirection (BREthereumAccount account,
                             BREthereumAddress source,
                             BREthereumAddress target) {
    BREthereumBoolean accountIsSource = ethAccountHasAddress (account, source);
    BREthereumBoolean accountIsTarget = ethAccountHasAddress (account, target);

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

static BREthereumHash
cryptoTransferGetEthHash (BRCryptoTransfer transferBase) {
    BRCryptoTransferETH transfer = cryptoTransferCoerce(transferBase);

    return (NULL != transfer->originatingTransaction
            ? transactionGetHash (transfer->originatingTransaction)
            : (TRANSFER_BASIS_TRANSACTION == transfer->type
               ? (NULL == transfer->basis.transaction ? EMPTY_HASH_INIT : transactionGetHash (transfer->basis.transaction))
               : (NULL == transfer->basis.log         ? EMPTY_HASH_INIT : logGetIdentifier   (transfer->basis.log))));
}

static BRCryptoHash
cryptoTransferGetHashETH (BRCryptoTransfer transferBase) {
    BREthereumHash ethHash = cryptoTransferGetEthHash (transferBase);
    return (ETHEREUM_BOOLEAN_TRUE == ethHashEqual (ethHash, EMPTY_HASH_INIT)
            ? NULL
            : cryptoHashCreateAsETH (ethHash));
}

extern const BREthereumHash
cryptoTransferGetIdentifierETH (BRCryptoTransferETH transfer) {
    switch (transfer->type) {
        case TRANSFER_BASIS_TRANSACTION:
            return (NULL == transfer->basis.transaction ? EMPTY_HASH_INIT : transactionGetHash(transfer->basis.transaction));
        case TRANSFER_BASIS_LOG:
            return (NULL == transfer->basis.log         ? EMPTY_HASH_INIT : logGetHash(transfer->basis.log));
    }
}

#if 0
static BREthereumHash
transferBasisGetHash (BREthereumTransferBasis *basis) {
    switch (basis->type) {
        case TRANSFER_BASIS_TRANSACTION: {
            if (NULL == basis->u.transaction) return EMPTY_HASH_INIT;

            return transactionGetHash (basis->u.transaction);
        }

        case TRANSFER_BASIS_LOG: {
            if (NULL == basis->u.log) return EMPTY_HASH_INIT;

            BREthereumHash hash = EMPTY_HASH_INIT;
            logExtractIdentifier(basis->u.log, &hash, NULL);
            return hash;
        }
    }
}
#endif

extern const BREthereumHash
cryptoTransferGetOriginatingTransactionHashETH (BRCryptoTransferETH transfer) {
    // If we have an originatingTransaction - becasue we created the transfer - then return its
    // hash.  Otherwise use the transfer's basis to get the hash
    return  (NULL != transfer->originatingTransaction
             ? transactionGetHash (transfer->originatingTransaction)
             : (TRANSFER_BASIS_TRANSACTION == transfer->type
                 ? (NULL == transfer->basis.transaction ? EMPTY_HASH_INIT : transactionGetHash (transfer->basis.transaction))
                 : (NULL == transfer->basis.log         ? EMPTY_HASH_INIT : logGetIdentifier   (transfer->basis.log))));
}

#if 0
extern uint64_t
transferGetNonce (BREthereumTransfer transfer) {
    return (NULL != transfer->originatingTransaction
            ? transactionGetNonce (transfer->originatingTransaction)
            : (TRANSFER_BASIS_TRANSACTION == transfer->basis.type && NULL != transfer->basis.u.transaction
               ? transactionGetNonce(transfer->basis.u.transaction)
               : TRANSACTION_NONCE_IS_NOT_ASSIGNED));
}
#endif

extern uint8_t *
cryptoTransferSerializeETH (BRCryptoTransfer transferBase,
                                         BRCryptoNetwork  network,
                                         BRCryptoBoolean  requireSignature,
                                         size_t *serializationCount) {
    BRCryptoTransferETH transfer = cryptoTransferCoerce (transferBase);

    if (NULL == transfer->originatingTransaction ||
        (CRYPTO_TRUE == requireSignature &&
        ETHEREUM_BOOLEAN_FALSE == transactionIsSigned (transfer->originatingTransaction))) {
        *serializationCount = 0;
        return NULL;
    }

    BRRlpData data = transactionGetRlpData (transfer->originatingTransaction,
                                            cryptoNetworkAsETH(network),
                                            (CRYPTO_TRUE == requireSignature
                                             ? RLP_TYPE_TRANSACTION_SIGNED
                                             : RLP_TYPE_TRANSACTION_UNSIGNED));

    *serializationCount = data.bytesCount;
    return data.bytes;
}

static int
cryptoTransferEqualAsETH (BRCryptoTransfer tb1, BRCryptoTransfer tb2) {
    if (tb1 == tb2) return 1;

    BREthereumHash h1 = cryptoTransferGetEthHash (tb1);
    BREthereumHash h2 = cryptoTransferGetEthHash (tb2);

    return (ETHEREUM_BOOLEAN_FALSE != ethHashEqual (h1, EMPTY_HASH_INIT) &&
            ETHEREUM_BOOLEAN_TRUE  == ethHashEqual (h1, h2));
}

BRCryptoTransferHandlers cryptoTransferHandlersETH = {
    cryptoTransferReleaseETH,
    cryptoTransferGetHashETH,
    cryptoTransferSerializeETH,
    cryptoTransferEqualAsETH
};
