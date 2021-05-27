//
//  BREthereumExchange.c
//  BRCore
//
//  Created by Ed Gamble on 6/9/20.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BREthereumExchange.h"

struct BREthereumExchangeRecord {
    // THIS MUST BE FIRST to support BRSet operations.

     /**
      * The hash - computed from the identifier pair {Transaction-Hash, Receipt-Index}
      */
     BREthereumHash hash;

    BREthereumAddress source;
    BREthereumAddress target;

    BREthereumAddress contract;  //  EMPTY_ADDRESS_INIT if 'Eth'
    size_t contractAssetIndex;

    UInt256 assetValue;

    /*
     * A unique identifer - derived from the transactionHash and the transactionReceiptIndex
     */
    struct {
        /**
         * The hash of the transaction producing this log.  This value *does not* depend on
         * which block records the Log.
         */
        BREthereumHash transactionHash;

        /**
         * The receipt index from the transaction's contract execution for this log.  It can't
         * possibly be the case that this number varies, can it - contract execution, regarding
         * event generating, must be deterministic?
         */
        size_t exchangeIndex;
    } identifier;

    /**
     * status
     */
    BREthereumTransactionStatus status;
};

extern BREthereumExchange
ethExchangeCreate (BREthereumAddress source,
                   BREthereumAddress target,
                   BREthereumAddress contract,  // NULL if 'Ether'
                   size_t contractAssetIndex,
                   UInt256 value) {
    BREthereumExchange exchange = calloc (1, sizeof (struct BREthereumExchangeRecord));

    exchange->hash = ethHashCreateEmpty();

    exchange->source = source;
    exchange->target = target;

    exchange->contract = contract;
    exchange->contractAssetIndex = contractAssetIndex;

    exchange->assetValue = value;

    exchange->identifier.exchangeIndex = EXCHANGE_INDEX_UNKNOWN;

    return exchange;
}


extern void
ethExchangeInitializeIdentifier (BREthereumExchange exchange,
                                 BREthereumHash transactionHash,
                                 size_t exchangeIndex) {
    exchange->identifier.transactionHash = transactionHash;
    exchange->identifier.exchangeIndex   = exchangeIndex;

    BRRlpData data = { sizeof (exchange->identifier), (uint8_t*) &exchange->identifier };
    exchange->hash = ethHashCreateFromData(data);
}

extern BREthereumBoolean
ethExchangeExtractIdentifier (BREthereumExchange exchange,
                              BREthereumHash *transactionHash,
                              size_t *exchangeIndex) {
    if (EXCHANGE_INDEX_UNKNOWN == exchange->identifier.exchangeIndex)
        return ETHEREUM_BOOLEAN_FALSE;

    if (NULL != transactionHash) *transactionHash = exchange->identifier.transactionHash;
    if (NULL != exchangeIndex)   *exchangeIndex   = exchange->identifier.exchangeIndex;

    return ETHEREUM_BOOLEAN_TRUE;
}

extern BREthereumHash
ethExchangeGetIdentifier (BREthereumExchange exchange) {
    return (EXCHANGE_INDEX_UNKNOWN == exchange->identifier.exchangeIndex
            ? EMPTY_HASH_INIT
            : exchange->identifier.transactionHash);
}


extern BREthereumHash
ethExchangeGetHash (BREthereumExchange exchange) {
    assert (EXCHANGE_INDEX_UNKNOWN != exchange->identifier.exchangeIndex);
    return exchange->hash;
}

extern BREthereumAddress
ethExchangeGetSourceAddress (BREthereumExchange exchange) {
    return exchange->source;
}

extern BREthereumAddress
ethExchangeGetTargetAddress (BREthereumExchange exchange) {
    return exchange->target;
}

extern BREthereumAddress
ethExchangeGetContract (BREthereumExchange exchange) {
    return exchange->contract;
}

extern size_t
ethExchangeGetContractAssetIndex (BREthereumExchange exchange) {
    return exchange->contractAssetIndex;
}

extern UInt256
ethExchangeGetAssetValue (BREthereumExchange exchange) {
    return exchange->assetValue;
}

static inline int
ethExchangeHasStatus (BREthereumExchange exchange,
                      BREthereumTransactionStatusType type) {
    return type == exchange->status.type;
}

extern BREthereumComparison
ethExchangeCompare (BREthereumExchange l1,
                    BREthereumExchange l2) {

    if (  l1 == l2) return ETHEREUM_COMPARISON_EQ;
    if (NULL == l2) return ETHEREUM_COMPARISON_LT;
    if (NULL == l1) return ETHEREUM_COMPARISON_GT;

    int t1Blocked = ethExchangeHasStatus(l1, TRANSACTION_STATUS_INCLUDED);
    int t2Blocked = ethExchangeHasStatus(l2, TRANSACTION_STATUS_INCLUDED);

    if (t1Blocked && t2Blocked)
        return (l1->status.u.included.blockNumber < l2->status.u.included.blockNumber
                ? ETHEREUM_COMPARISON_LT
                : (l1->status.u.included.blockNumber > l2->status.u.included.blockNumber
                   ? ETHEREUM_COMPARISON_GT
                   : (l1->status.u.included.transactionIndex < l2->status.u.included.transactionIndex
                      ? ETHEREUM_COMPARISON_LT
                      : (l1->status.u.included.transactionIndex > l2->status.u.included.transactionIndex
                         ? ETHEREUM_COMPARISON_GT
                         : (l1->identifier.exchangeIndex < l2->identifier.exchangeIndex
                            ? ETHEREUM_COMPARISON_LT
                            : (l1->identifier.exchangeIndex > l2->identifier.exchangeIndex
                               ? ETHEREUM_COMPARISON_GT
                               : ETHEREUM_COMPARISON_EQ))))));

    else if (!t1Blocked && t2Blocked)
        return ETHEREUM_COMPARISON_GT;

    else if (t1Blocked && !t2Blocked)
        return ETHEREUM_COMPARISON_LT;

    else
        return ETHEREUM_COMPARISON_EQ;
}

extern BREthereumTransactionStatus
ethExchangeGetStatus (BREthereumExchange exchange) {
    return exchange->status;
}

extern void
ethExchangeSetStatus (BREthereumExchange exchange,
                      BREthereumTransactionStatus status) {
    exchange->status = status;
}

extern BREthereumBoolean
ethExchangeIsConfirmed (BREthereumExchange exchange) {
    return AS_ETHEREUM_BOOLEAN(TRANSACTION_STATUS_INCLUDED == exchange->status.type);
}

extern BREthereumBoolean
ethExchangeIsErrored (BREthereumExchange exchange) {
    return AS_ETHEREUM_BOOLEAN(TRANSACTION_STATUS_ERRORED == exchange->status.type);
}

// Support BRSet
extern size_t
ethExchangeHashValue (const void *e) {
    assert (EXCHANGE_INDEX_UNKNOWN != ((BREthereumExchange) e)->identifier.exchangeIndex);
    return (size_t) ethHashSetValue(&((BREthereumExchange) e)->hash);
}

// Support BRSet
extern int
ethExchangeHashEqual (const void *l1, const void *l2) {
    if (l1 == l2) return 1;

    assert (EXCHANGE_INDEX_UNKNOWN != ((BREthereumExchange) l1)->identifier.exchangeIndex);
    assert (EXCHANGE_INDEX_UNKNOWN != ((BREthereumExchange) l2)->identifier.exchangeIndex);
    return ethHashSetEqual (&((BREthereumExchange) l1)->hash,
                            &((BREthereumExchange) l2)->hash);

}

extern BREthereumExchange
ethExchangeRlpDecode (BRRlpItem item,
                      BREthereumRlpType type,
                      BRRlpCoder coder) {
    BREthereumExchange exchange = calloc (1, sizeof (struct BREthereumExchangeRecord));

    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);
    assert ((5 == itemsCount && RLP_TYPE_NETWORK == type) ||
            (8 == itemsCount && RLP_TYPE_ARCHIVE == type));

    exchange->source   = ethAddressRlpDecode (items[0], coder);
    exchange->target   = ethAddressRlpDecode (items[1], coder);
    exchange->contract = ethAddressRlpDecode (items[2], coder);

    exchange->contractAssetIndex = rlpDecodeUInt64  (coder, items[3], 0);
    exchange->assetValue         = rlpDecodeUInt256 (coder, items[4], 0);

    if (RLP_TYPE_ARCHIVE == type) {
        BREthereumHash hash = ethHashRlpDecode(items[5], coder);
        size_t exchangeIndex = rlpDecodeUInt64 (coder, items[6], 0);
        ethExchangeInitializeIdentifier (exchange, hash, (size_t) exchangeIndex);

        exchange->status = transactionStatusRLPDecode (items[7], NULL, coder);
    }

    return exchange;
}
/**
 * [QUASI-INTERNAL - used by BREthereumBlock]
 */
extern BRRlpItem
ethExchangeRlpEncode(BREthereumExchange exchange,
                     BREthereumRlpType type,
                     BRRlpCoder coder) {
    BRRlpItem items[8]; // more than enough

    items[0] = ethAddressRlpEncode (exchange->source,   coder);
    items[1] = ethAddressRlpEncode (exchange->target,   coder);
    items[2] = ethAddressRlpEncode (exchange->contract, coder);
    items[3] = rlpEncodeUInt64  (coder, exchange->contractAssetIndex, 0);
    items[4] = rlpEncodeUInt256 (coder, exchange->assetValue, 0);

    if (RLP_TYPE_ARCHIVE == type) {
        items[5] = ethHashRlpEncode(exchange->identifier.transactionHash, coder);
        items[6] = rlpEncodeUInt64(coder, exchange->identifier.exchangeIndex, 0);
        items[7] = transactionStatusRLPEncode(exchange->status, coder);
    }

    return rlpEncodeListItems(coder, items, (RLP_TYPE_ARCHIVE == type ? 8 : 5));
}

extern void
ethExchangeRelease (BREthereumExchange exchange) {
    if (NULL == exchange) return;
    free (exchange);
}

extern BREthereumExchange
ethExchangeCopy (BREthereumExchange exchange) {
    BREthereumExchange copy = calloc (1, sizeof (struct BREthereumExchangeRecord));

    memcpy (copy, exchange, sizeof(struct BREthereumExchangeRecord));
    return copy;
}

