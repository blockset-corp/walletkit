//
//  WKTransferETH.c
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKETH.h"
#include "support/BRInt.h"
#include "walletkit/WKAmountP.h"

#include "ethereum/blockchain/BREthereumTransaction.h"
#include "ethereum/blockchain/BREthereumLog.h"
#include "ethereum/contract/BREthereumExchange.h"

static WKTransferDirection
wkTransferFindDirection (BREthereumAccount account,
                             BREthereumAddress source,
                             BREthereumAddress target);

extern WKTransferETH
wkTransferCoerceETH (WKTransfer transfer) {
    assert (WK_NETWORK_TYPE_ETH == transfer->type);
    return (WKTransferETH) transfer;
}

extern WKTransferState
wkTransferDeriveStateETH (BREthereumTransactionStatus status,
                              WKFeeBasis feeBasis) {
    switch (status.type) {
        case TRANSACTION_STATUS_UNKNOWN:
            return wkTransferStateInit (WK_TRANSFER_STATE_CREATED);
        case TRANSACTION_STATUS_QUEUED:
        case TRANSACTION_STATUS_PENDING:
            return wkTransferStateInit (WK_TRANSFER_STATE_SUBMITTED);

        case TRANSACTION_STATUS_INCLUDED:
            return wkTransferStateIncludedInit (status.u.included.blockNumber,
                                                    status.u.included.transactionIndex,
                                                    status.u.included.blockTimestamp,
                                                    wkFeeBasisTake (feeBasis),
                                                    AS_WK_BOOLEAN (status.u.included.success),
                                                    NULL);
            break;

        case TRANSACTION_STATUS_ERRORED:
            return wkTransferStateErroredInit((WKTransferSubmitError) {
                WK_TRANSFER_SUBMIT_ERROR_UNKNOWN
            });
    }
}

typedef struct {
    WKHash hash;
    BREthereumAccount account;
    BREthereumTransaction originatingTransaction;
    uint64_t nonce;
} WKTransferCreateContextETH;

static void
wkTransferCreateCallbackETH (WKTransferCreateContext context,
                                 WKTransfer transfer) {
    WKTransferCreateContextETH *contextETH = (WKTransferCreateContextETH*) context;
    WKTransferETH transferETH = wkTransferCoerceETH (transfer);

    transferETH->hash    = wkHashTake(contextETH->hash);
    transferETH->account = contextETH->account;
    transferETH->nonce   = contextETH->nonce;
    transferETH->originatingTransaction = contextETH->originatingTransaction;
}

extern WKTransfer
wkTransferCreateAsETH (WKTransferListener listener,
                           const char *uids,
                           WKHash hash,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKFeeBasis feeBasisEstimated,
                           WKAmount amount,
                           WKAddress sourceAddress,
                           WKAddress targetAddress,
                           WKTransferState transferState,
                           BREthereumAccount account,
                           uint64_t nonce,
                           OwnershipGiven BREthereumTransaction originatingTransaction) {
    assert (NULL  == originatingTransaction ||
            nonce == ethTransactionGetNonce (originatingTransaction));

    WKTransferCreateContextETH contextETH = {
        hash,
        account,
        originatingTransaction,
        nonce
    };

    WKTransferDirection direction = wkTransferFindDirection (account,
                                                                       wkAddressAsETH (sourceAddress),
                                                                       wkAddressAsETH (targetAddress));

    return wkTransferAllocAndInit (sizeof (struct WKTransferETHRecord),
                                       WK_NETWORK_TYPE_ETH,
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
                                       wkTransferCreateCallbackETH);
}

static void
wkTransferReleaseETH (WKTransfer transfer) {
    WKTransferETH transferETH = wkTransferCoerceETH (transfer);

    wkHashGive(transferETH->hash);

    if (NULL != transferETH->originatingTransaction)
        ethTransactionRelease(transferETH->originatingTransaction);
}

extern uint64_t
wkTransferGetNonceETH (WKTransferETH transfer) {
    return transfer->nonce;
}

extern void
wkTransferSetNonceETH (WKTransferETH transfer,
                       uint64_t nonce) {
    transfer->nonce = nonce;
    if (NULL != transfer->originatingTransaction)
        ethTransactionSetNonce (transfer->originatingTransaction, nonce);
}

static WKTransferDirection
wkTransferFindDirection (BREthereumAccount account,
                             BREthereumAddress source,
                             BREthereumAddress target) {
    BREthereumBoolean accountIsSource = ethAccountHasAddress (account, source);
    BREthereumBoolean accountIsTarget = ethAccountHasAddress (account, target);

    if (accountIsSource == ETHEREUM_BOOLEAN_TRUE && accountIsTarget == ETHEREUM_BOOLEAN_TRUE) {
        return WK_TRANSFER_RECOVERED;
    } else if (accountIsSource == ETHEREUM_BOOLEAN_TRUE && accountIsTarget == ETHEREUM_BOOLEAN_FALSE) {
        return WK_TRANSFER_SENT;
    } else if (accountIsSource == ETHEREUM_BOOLEAN_FALSE && accountIsTarget == ETHEREUM_BOOLEAN_TRUE) {
        return WK_TRANSFER_RECEIVED;
    } else {
        assert(0);
    }
}


extern const BREthereumHash
wkTransferGetIdentifierETH (WKTransferETH transfer) {
    return (NULL != transfer->hash
            ? wkHashAsETH(transfer->hash)
            : ETHEREUM_EMPTY_HASH_INIT);
}

static WKHash
wkTransferGetHashETH (WKTransfer transfer) {
    WKTransferETH transferETH = wkTransferCoerceETH (transfer);
    return wkHashTake (transferETH->hash);
}

extern const BREthereumHash
wkTransferGetOriginatingTransactionHashETH (WKTransferETH transfer) {
    // If we have an originatingTransaction - becasue we created the transfer - then return its
    // hash.  Otherwise use tEMPTY_HASH_INIT
    return  (NULL != transfer->originatingTransaction
             ? ethTransactionGetHash (transfer->originatingTransaction)
             : ETHEREUM_EMPTY_HASH_INIT);
}

extern OwnershipGiven uint8_t *
wkTransferSerializeETH (WKTransfer transfer,
                            WKNetwork  network,
                            WKBoolean  requireSignature,
                            size_t *serializationCount) {
    WKTransferETH transferETH = wkTransferCoerceETH (transfer);

    if (NULL == transferETH->originatingTransaction ||
        (WK_TRUE == requireSignature &&
         ETHEREUM_BOOLEAN_FALSE == ethTransactionIsSigned (transferETH->originatingTransaction))) {
        *serializationCount = 0;
        return NULL;
    }

    BRRlpData data = ethTransactionGetRlpData (transferETH->originatingTransaction,
                                            wkNetworkAsETH(network),
                                            (WK_TRUE == requireSignature
                                             ? RLP_TYPE_TRANSACTION_SIGNED
                                             : RLP_TYPE_TRANSACTION_UNSIGNED));

    *serializationCount = data.bytesCount;
    return data.bytes;
}

extern OwnershipGiven uint8_t *
wkTransferGetBytesForFeeEstimateETH (WKTransfer transfer,
                                         WKNetwork  network,
                                         size_t *bytesCount) {
    WKTransferETH transferETH = wkTransferCoerceETH (transfer);
    BREthereumTransaction ethTransaction = transferETH->originatingTransaction;

    if (NULL == ethTransaction) { *bytesCount = 0; return NULL; }

    BRRlpData data = ethTransactionGetRlpData (ethTransaction,
                                            wkNetworkAsETH(network),
                                            RLP_TYPE_TRANSACTION_UNSIGNED);
    BREthereumAddress ethSource = ethTransactionGetSourceAddress (ethTransaction);

    *bytesCount = ETHEREUM_ADDRESS_BYTES + data.bytesCount;
    uint8_t *bytes = malloc (*bytesCount);
    memcpy (&bytes[0],             ethSource.bytes, ETHEREUM_ADDRESS_BYTES);
    memcpy (&bytes[ETHEREUM_ADDRESS_BYTES], data.bytes,      data.bytesCount);

    rlpDataRelease(data);

    return bytes;
}

static int
wkTransferEqualAsETH (WKTransfer tb1, WKTransfer tb2) {
    if (tb1 == tb2) return 1;

    WKTransferETH te1 = wkTransferCoerceETH (tb1);
    WKTransferETH te2 = wkTransferCoerceETH (tb2);

    return (NULL != te1->hash &&
            NULL != te2->hash &&
            WK_TRUE == wkHashEqual(te1->hash, te2->hash));
}

WKTransferHandlers wkTransferHandlersETH = {
    wkTransferReleaseETH,
    wkTransferGetHashETH,
    NULL, // setHash
    NULL, // updateIdentifier
    wkTransferSerializeETH,
    wkTransferGetBytesForFeeEstimateETH,
    wkTransferEqualAsETH
};
