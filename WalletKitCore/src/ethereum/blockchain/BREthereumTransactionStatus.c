//
//  BREthereumTransactionStatus.c
//  WalletKitCore
//
//  Created by Ed Gamble on 5/15/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdlib.h>
#include <assert.h>
#include "BREthereumTransactionStatus.h"

extern const char *
ethTransactionGetStatusTypeName (BREthereumTransactionStatusType type) {
    static const char *names[] = {
        "unknown",
        "queued",
        "pending",
        "included",
        "errored"
    };

    return names[type];
}

const char *
ethTransactionGetErrorName (BREthereumTransactionErrorType type) {
    static const char *names[] = {
        "* invalid signature",
        "* nonce too low",
        "* balance too low",
        "* gas price too low",
        "* gas too low",
        "* replacement under-pricesd",
        "* dropped",
        "* already known",
        "* unknown"
    };

    return names[type];
}

extern BREthereumTransactionStatus
ethTransactionStatusCreate (BREthereumTransactionStatusType type) {
    assert (TRANSACTION_STATUS_INCLUDED != type && TRANSACTION_STATUS_ERRORED != type);
    BREthereumTransactionStatus status;
    status.type = type;
    return status;
}

extern BREthereumTransactionStatus
ethTransactionStatusCreateIncluded (BREthereumHash blockHash,
                                 uint64_t blockNumber,
                                 uint64_t transactionIndex,
                                 uint64_t blockTimestamp,
                                 BREthereumGas gasUsed,
                                 uint64_t success) {
    return (BREthereumTransactionStatus) {
        TRANSACTION_STATUS_INCLUDED,
        blockHash,
        blockNumber,
        transactionIndex,
        blockTimestamp,
        gasUsed,
        success
    };
}

extern BREthereumTransactionStatus
ethTransactionStatusCreateErrored (BREthereumTransactionErrorType type,
                                const char *detail) {
    BREthereumTransactionStatus status;
    status.type = TRANSACTION_STATUS_ERRORED;
    status.u.errored.type = type;
    strlcpy (status.u.errored.detail, detail, ETHEREUM_TRANSACTION_STATUS_DETAIL_BYTES);
    return status;
}

extern int
ethTransactionStatusExtractIncluded(const BREthereumTransactionStatus *status,
                                    BREthereumHash *blockHash,
                                    uint64_t *blockNumber,
                                    uint64_t *blockTransactionIndex,
                                    uint64_t *blockTimestamp,
                                    BREthereumGas *gas,
                                    uint64_t *success) {
    if (status->type != TRANSACTION_STATUS_INCLUDED)
        return 0;

    if (NULL != blockHash) *blockHash = status->u.included.blockHash;
    if (NULL != blockNumber) *blockNumber = status->u.included.blockNumber;
    if (NULL != blockTransactionIndex) *blockTransactionIndex = status->u.included.transactionIndex;
    if (NULL != blockTimestamp) *blockTimestamp = status->u.included.blockTimestamp;
    if (NULL != gas) *gas = status->u.included.gasUsed;
    if (NULL != success) *success = status->u.included.success;
    
    return 1;
}

extern int
ethTransactionStatusExtractErrored (const BREthereumTransactionStatus *status,
                                    BREthereumTransactionErrorType *type,
                                    char **detail) {
    if (status->type != TRANSACTION_STATUS_ERRORED)
        return 0;

    if (NULL != type) *type = status->u.errored.type;
    if (NULL != detail) *detail = strdup (status->u.errored.detail);

    return 1;
}

extern BREthereumBoolean
ethTransactionStatusEqual (BREthereumTransactionStatus ts1,
                        BREthereumTransactionStatus ts2) {
    return AS_ETHEREUM_BOOLEAN(ts1.type == ts2.type &&
                               ((TRANSACTION_STATUS_INCLUDED != ts1.type && TRANSACTION_STATUS_ERRORED != ts1.type) ||
                                (TRANSACTION_STATUS_INCLUDED == ts1.type &&
                                 ETHEREUM_COMPARISON_EQ == ethGasCompare(ts1.u.included.gasUsed, ts2.u.included.gasUsed) &&
                                 ETHEREUM_BOOLEAN_IS_TRUE(ethHashEqual(ts1.u.included.blockHash, ts2.u.included.blockHash)) &&
                                 ts1.u.included.blockNumber == ts2.u.included.blockNumber &&
                                 ts1.u.included.transactionIndex == ts2.u.included.transactionIndex) ||
                                (TRANSACTION_STATUS_ERRORED == ts1.type &&
                                 ts1.u.errored.type == ts2.u.errored.type &&
                                 0 == strcmp (ts1.u.errored.detail, ts2.u.errored.detail))));
}

extern BREthereumComparison
ethTransactionStatusCompare (const BREthereumTransactionStatus *ts1,
                          const BREthereumTransactionStatus *ts2) {
    int t1Blocked = ts1->type == TRANSACTION_STATUS_INCLUDED;
    int t2Blocked = ts2->type == TRANSACTION_STATUS_INCLUDED;

    if (t1Blocked && t2Blocked)
        return (ts1->u.included.blockNumber < ts2->u.included.blockNumber
                ? ETHEREUM_COMPARISON_LT
                : (ts1->u.included.blockNumber > ts2->u.included.blockNumber
                   ? ETHEREUM_COMPARISON_GT
                   : (ts1->u.included.transactionIndex < ts2->u.included.transactionIndex
                      ? ETHEREUM_COMPARISON_LT
                      : (ts1->u.included.transactionIndex > ts2->u.included.transactionIndex
                         ? ETHEREUM_COMPARISON_GT
                         : ETHEREUM_COMPARISON_EQ))));

    else if (!t1Blocked && t2Blocked)
        return ETHEREUM_COMPARISON_GT;

    else if (t1Blocked && !t2Blocked)
        return ETHEREUM_COMPARISON_LT;

    else
        return ETHEREUM_COMPARISON_EQ;
}

extern BREthereumTransactionErrorType
lookupTransactionErrorType (const char *reasons[],
                            const char *reason) {
    if (NULL != reasons)
        for (BREthereumTransactionErrorType type = TRANSACTION_ERROR_INVALID_SIGNATURE;
             type <= TRANSACTION_ERROR_UNKNOWN;
             type++) {
            if (0 == strncmp (reason, reasons[type], strlen(reasons[type])))
                return type;
        }
    return TRANSACTION_ERROR_UNKNOWN;
}

extern BREthereumTransactionStatus
ethTransactionStatusRLPDecode (BRRlpItem item,
                            const char *reasons[],
                            BRRlpCoder coder) {
    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);
    assert (3 == itemsCount); // [type, [blockHash, blockNumber, txIndex], error]

    // We have seen (many) cases where the `type` is `unknown` but there is an `error`.  That
    // appears to violate the LES specfication.  Anyways, if we see an `error` we'll force the
    // type to be TRANSACTION_STATUS_ERRORED.
    char *reason = rlpDecodeString(coder, items[2]);
    if (NULL != reason && 0 != strcmp (reason, "") && 0 != strcmp (reason, "0x")) {
        BREthereumTransactionErrorType type = lookupTransactionErrorType (reasons, reason);
        BREthereumTransactionStatus status = ethTransactionStatusCreateErrored (type, reason);
        free (reason);
        // CORE-264: We always consider an 'already known' error as 'pending'
        return TRANSACTION_ERROR_ALREADY_KNOWN != type ? status : ethTransactionStatusCreate(TRANSACTION_STATUS_PENDING);
    }
    if (NULL != reason) free (reason);

    BREthereumTransactionStatusType type = (BREthereumTransactionStatusType) rlpDecodeUInt64(coder, items[0], 0);
    switch (type) {
        case TRANSACTION_STATUS_UNKNOWN:
        case TRANSACTION_STATUS_QUEUED:
        case TRANSACTION_STATUS_PENDING:
            // assert: [] == item[1], "" == item[2]
            return ethTransactionStatusCreate(type);

        case TRANSACTION_STATUS_INCLUDED: {
            size_t othersCount;
            const BRRlpItem *others = rlpDecodeList(coder, items[1], &othersCount);

            // The 'encode' function provides '6' others.  However, the value of others has
            // varied over time:
            //   3: baseline
            //   5: Add - included timestamp and included gasUsed
            //   6: Add - included success
            assert (6 == othersCount || 5 == othersCount || 3 == othersCount);

            return ethTransactionStatusCreateIncluded (ethHashRlpDecode(others[0], coder),
                                                    rlpDecodeUInt64(coder, others[1], 0),
                                                    rlpDecodeUInt64(coder, others[2], 0),
                                                    (3 == othersCount
                                                     ? ETHEREUM_TRANSACTION_STATUS_BLOCK_TIMESTAMP_UNKNOWN
                                                     : rlpDecodeUInt64(coder, others[3], 0)),
                                                    (3 == othersCount
                                                     ? ethGasCreate(0)
                                                     : ethGasRlpDecode(others[4], coder)),
                                                    (3 == othersCount || 5 == othersCount
                                                     ? 1
                                                     : rlpDecodeUInt64(coder, others[5], 0)));
        }
        
        case TRANSACTION_STATUS_ERRORED: {
            // We should not be here....
            BREthereumTransactionErrorType type = (BREthereumTransactionErrorType) rlpDecodeUInt64 (coder, items[2], 0);
            return (TRANSACTION_ERROR_ALREADY_KNOWN != type
                    ? ethTransactionStatusCreateErrored (type, ethTransactionGetErrorName (type))
                    : ethTransactionStatusCreate(TRANSACTION_STATUS_PENDING));
        }
    }
}

extern BRRlpItem
ethTransactionStatusRLPEncode (BREthereumTransactionStatus status,
                            BRRlpCoder coder) {
    BRRlpItem items[3];

    items[0] = rlpEncodeUInt64(coder, status.type, 0);

    switch (status.type) {
        case TRANSACTION_STATUS_UNKNOWN:
        case TRANSACTION_STATUS_QUEUED:
        case TRANSACTION_STATUS_PENDING:
            items[1] = rlpEncodeList(coder, 0);
            items[2] = rlpEncodeString(coder, "");
            break;

        case TRANSACTION_STATUS_INCLUDED:
            items[1] = rlpEncodeList (coder, 6,
                                      ethHashRlpEncode(status.u.included.blockHash, coder),
                                      rlpEncodeUInt64(coder, status.u.included.blockNumber, 0),
                                      rlpEncodeUInt64(coder, status.u.included.transactionIndex, 0),
                                      rlpEncodeUInt64(coder, status.u.included.blockTimestamp, 0),
                                      ethGasRlpEncode(status.u.included.gasUsed, coder),
                                      rlpEncodeUInt64(coder, status.u.included.success, 0));
            items[2] = rlpEncodeString(coder, "");

            break;

        case TRANSACTION_STATUS_ERRORED:
            items[1] = rlpEncodeList(coder, 0);
            items[2] = rlpEncodeUInt64 (coder, status.u.errored.type, 0);
            break;
    }

    return rlpEncodeListItems(coder, items, 3);
}

extern BRArrayOf (BREthereumTransactionStatus)
ethTransactionStatusDecodeList (BRRlpItem item,
                             const char *reasons[],
                             BRRlpCoder coder) {
    size_t itemCount;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemCount);

    BRArrayOf (BREthereumTransactionStatus) stati;
    array_new (stati, itemCount);
    for (size_t index = 0; index < itemCount; index++)
        array_add (stati, ethTransactionStatusRLPDecode (items[index], reasons, coder));

    return stati;
}

/* GETH TxStatus
 ETH: TxtStatus: L  3: [
 ETH: TxtStatus:   I  0: 0x
 ETH: TxtStatus:   I  4: 0x11e19aa2
 ETH: TxtStatus:   L  1: [
 ETH: TxtStatus:     L  3: [
 ETH: TxtStatus:       I  0: 0x         # status: unknown (0)
 ETH: TxtStatus:       L  0: []         # [blockHash, blockNumber, transactionIndex]: []
 ETH: TxtStatus:       I  0: 0x         # error: ""
 ETH: TxtStatus:     ]
 ETH: TxtStatus:   ]
 ETH: TxtStatus: ]
 */
