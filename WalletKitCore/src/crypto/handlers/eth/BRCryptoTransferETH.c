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
#include "ethereum/contract/BREthereumExchange.h"

static BRCryptoTransferDirection
cryptoTransferFindDirection (BREthereumAccount account,
                             BREthereumAddress source,
                             BREthereumAddress target);

extern BRCryptoTransferETH
cryptoTransferCoerceETH (BRCryptoTransfer transfer) {
    assert (CRYPTO_NETWORK_TYPE_ETH == transfer->type);
    return (BRCryptoTransferETH) transfer;
}

extern BRCryptoTransferState
cryptoTransferDeriveStateETH (BREthereumTransactionStatus status,
                              BRCryptoFeeBasis feeBasis) {
    switch (status.type) {
        case TRANSACTION_STATUS_UNKNOWN:
            return cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_CREATED);
        case TRANSACTION_STATUS_QUEUED:
        case TRANSACTION_STATUS_PENDING:
            return cryptoTransferStateInit (CRYPTO_TRANSFER_STATE_SUBMITTED);

        case TRANSACTION_STATUS_INCLUDED:
            return cryptoTransferStateIncludedInit (status.u.included.blockNumber,
                                                    status.u.included.transactionIndex,
                                                    status.u.included.blockTimestamp,
                                                    cryptoFeeBasisTake (feeBasis),
                                                    AS_CRYPTO_BOOLEAN (status.u.included.success),
                                                    NULL);
            break;

        case TRANSACTION_STATUS_ERRORED:
            return cryptoTransferStateErroredInit((BRCryptoTransferSubmitError) {
                CRYPTO_TRANSFER_SUBMIT_ERROR_UNKNOWN
            });
    }
}

typedef struct {
    BRCryptoHash hash;
    BREthereumAccount account;
    BREthereumTransaction originatingTransaction;
    uint64_t nonce;
} BRCryptoTransferCreateContextETH;

static void
cryptoTransferCreateCallbackETH (BRCryptoTransferCreateContext context,
                                 BRCryptoTransfer transfer) {
    BRCryptoTransferCreateContextETH *contextETH = (BRCryptoTransferCreateContextETH*) context;
    BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (transfer);

    transferETH->hash    = cryptoHashTake(contextETH->hash);
    transferETH->account = contextETH->account;
    transferETH->nonce   = contextETH->nonce;
    transferETH->originatingTransaction = contextETH->originatingTransaction;
}

extern BRCryptoTransfer
cryptoTransferCreateAsETH (BRCryptoTransferListener listener,
                           const char *uids,
                           BRCryptoHash hash,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRCryptoFeeBasis feeBasisEstimated,
                           BRCryptoAmount amount,
                           BRCryptoAddress sourceAddress,
                           BRCryptoAddress targetAddress,
                           BRCryptoTransferState transferState,
                           BREthereumAccount account,
                           uint64_t nonce,
                           OwnershipGiven BREthereumTransaction originatingTransaction) {
    assert (NULL  == originatingTransaction ||
            nonce == transactionGetNonce (originatingTransaction));

    BRCryptoTransferCreateContextETH contextETH = {
        hash,
        account,
        originatingTransaction,
        nonce
    };

    BRCryptoTransferDirection direction = cryptoTransferFindDirection (account,
                                                                       cryptoAddressAsETH (sourceAddress),
                                                                       cryptoAddressAsETH (targetAddress));

    return cryptoTransferAllocAndInit (sizeof (struct BRCryptoTransferETHRecord),
                                       CRYPTO_NETWORK_TYPE_ETH,
                                       listener,
                                       uids,
                                       unit,
                                       unitForFee,
                                       feeBasisEstimated,
                                       amount,
                                       direction,
                                       sourceAddress,
                                       targetAddress,
                                       transferState,
                                       &contextETH,
                                       cryptoTransferCreateCallbackETH);
}

static void
cryptoTransferReleaseETH (BRCryptoTransfer transfer) {
    BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (transfer);

    cryptoHashGive(transferETH->hash);

    if (NULL != transferETH->originatingTransaction)
        transactionRelease(transferETH->originatingTransaction);
}

extern uint64_t
cryptoTransferGetNonceETH (BRCryptoTransferETH transfer) {
    return transfer->nonce;
}

extern void
cryptoTransferSetNonceETH (BRCryptoTransferETH transfer,
                           uint64_t nonce) {
    transfer->nonce = nonce;
    if (NULL != transfer->originatingTransaction)
        transactionSetNonce (transfer->originatingTransaction, nonce);
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


extern const BREthereumHash
cryptoTransferGetIdentifierETH (BRCryptoTransferETH transfer) {
    return (NULL != transfer->hash
            ? cryptoHashAsETH(transfer->hash)
            : EMPTY_HASH_INIT);
}

static BRCryptoHash
cryptoTransferGetHashETH (BRCryptoTransfer transfer) {
    BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (transfer);
    return cryptoHashTake (transferETH->hash);
}

extern const BREthereumHash
cryptoTransferGetOriginatingTransactionHashETH (BRCryptoTransferETH transfer) {
    // If we have an originatingTransaction - becasue we created the transfer - then return its
    // hash.  Otherwise use tEMPTY_HASH_INIT
    return  (NULL != transfer->originatingTransaction
             ? transactionGetHash (transfer->originatingTransaction)
             : EMPTY_HASH_INIT);
}

extern uint8_t *
cryptoTransferSerializeETH (BRCryptoTransfer transfer,
                            BRCryptoNetwork  network,
                            BRCryptoBoolean  requireSignature,
                            size_t *serializationCount) {
    BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (transfer);

    if (NULL == transferETH->originatingTransaction ||
        (CRYPTO_TRUE == requireSignature &&
         ETHEREUM_BOOLEAN_FALSE == transactionIsSigned (transferETH->originatingTransaction))) {
        *serializationCount = 0;
        return NULL;
    }

    BRRlpData data = transactionGetRlpData (transferETH->originatingTransaction,
                                            cryptoNetworkAsETH(network),
                                            (CRYPTO_TRUE == requireSignature
                                             ? RLP_TYPE_TRANSACTION_SIGNED
                                             : RLP_TYPE_TRANSACTION_UNSIGNED));

    *serializationCount = data.bytesCount;
    return data.bytes;
}

extern uint8_t *
cryptoTransferGetBytesForFeeEstimateETH (BRCryptoTransfer transfer,
                                         BRCryptoNetwork  network,
                                         size_t *bytesCount) {
    BRCryptoTransferETH transferETH = cryptoTransferCoerceETH (transfer);
    BREthereumTransaction ethTransaction = transferETH->originatingTransaction;

    if (NULL == ethTransaction) { *bytesCount = 0; return NULL; }

    BRRlpData data = transactionGetRlpData (ethTransaction,
                                            cryptoNetworkAsETH(network),
                                            RLP_TYPE_TRANSACTION_UNSIGNED);
    BREthereumAddress ethSource = transactionGetSourceAddress (ethTransaction);

    *bytesCount = ADDRESS_BYTES + data.bytesCount;
    uint8_t *bytes = malloc (*bytesCount);
    memcpy (&bytes[0],             ethSource.bytes, ADDRESS_BYTES);
    memcpy (&bytes[ADDRESS_BYTES], data.bytes,      data.bytesCount);

    rlpDataRelease(data);

    return bytes;
}

static int
cryptoTransferEqualAsETH (BRCryptoTransfer tb1, BRCryptoTransfer tb2) {
    if (tb1 == tb2) return 1;

    BRCryptoTransferETH te1 = cryptoTransferCoerceETH (tb1);
    BRCryptoTransferETH te2 = cryptoTransferCoerceETH (tb2);

    return (NULL != te1->hash &&
            NULL != te2->hash &&
            CRYPTO_TRUE == cryptoHashEqual(te1->hash, te2->hash));
}

BRCryptoTransferHandlers cryptoTransferHandlersETH = {
    cryptoTransferReleaseETH,
    cryptoTransferGetHashETH,
    NULL, // setHash
    NULL, // updateIdentifier
    cryptoTransferSerializeETH,
    cryptoTransferGetBytesForFeeEstimateETH,
    cryptoTransferEqualAsETH
};
