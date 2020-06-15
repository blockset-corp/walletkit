//
//  BREthereumExchange.h
//  BRCore
//
//  Created by Ed Gamble on 6/9/20.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Exchange_h
#define BR_Ethereum_Exchange_h

#include "ethereum/base/BREthereumBase.h"
#include "ethereum/blockchain/BREthereumTransactionStatus.h"
#include "BREthereumContract.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * An Ethereum Exchange ...
 *
 */
typedef struct BREthereumExchangeRecord *BREthereumExchange;

extern BREthereumExchange
ethExchangeCreate (BREthereumAddress source,
                   BREthereumAddress target,
                   BREthereumAddress contract,  // NULL if 'Ether'
                   size_t contractdAssetIndex,
                   UInt256 value);


extern void
ethExchangeInitializeIdentifier (BREthereumExchange exchange,
                                 BREthereumHash transactionHash,
                                 size_t exchangeIndex);

/**
 * An identifier for an unknown exchange index index.
 */
#define EXCHANGE_INDEX_UNKNOWN       (SIZE_MAX)

/**
 * Extract the log's identifier components.  A Log is identified by the transaction hash that
 * originated the log and the index of the log in the block's transaction receipts array.
 *
 * If the log has not been recorded in a block, then FALSE is returned.  Otherwise TRUE is returned
 * and `transactionHash` and `transacetionReceiptIndex` will be filled.
 *
 * @param log the log
 * @param transactionHash a hash pointer; if non-NULL will be filled with the transaction's hash
 * @param exchangeIndex an index pointer, if non-NULL will be filled with the logs
 *    index in the block's transaction receipts array.
 *
 * @return TRUE if recorded in a block, FALSE otherwise.
 */
extern BREthereumBoolean
ethExchangeExtractIdentifier (BREthereumExchange exchange,
                              BREthereumHash *transactionHash,
                              size_t *exchangeIndex);

// Will be EMPTY_HASH_INIT if no identifier
extern BREthereumHash
ethExchangeGetIdentifier (BREthereumExchange exchange);

extern BREthereumHash
ethExchangeGetHash (BREthereumExchange exchange);

extern BREthereumAddress
ethExchangeGetSourceAddress (BREthereumExchange exchange);

extern BREthereumAddress
ethExchangeGetTargetAddress (BREthereumExchange exchange);

extern BREthereumAddress
ethExchangeGetContract (BREthereumExchange exchange);

extern size_t
ethExchangeGetContractAssetIndex (BREthereumExchange exchange);

extern UInt256
ethExchangeGetAssetValue (BREthereumExchange exchange);

extern BREthereumComparison
ethExchangeCompare (BREthereumExchange e1,
                    BREthereumExchange e2) ;

extern BREthereumTransactionStatus
ethExchangeGetStatus (BREthereumExchange exchange);

extern void
ethExchangeSetStatus (BREthereumExchange exchange,
                      BREthereumTransactionStatus status);

extern BREthereumBoolean
ethExchangeIsConfirmed (BREthereumExchange exchange);

extern BREthereumBoolean
ethExchangeIsErrored (BREthereumExchange exchange);

// Support BRSet
extern size_t
ethExchangeHashValue (const void *h);

// Support BRSet
extern int
ethExchangeHashEqual (const void *h1, const void *h2);

extern BREthereumExchange
ethExchangeRlpDecode (BRRlpItem item,
                      BREthereumRlpType type,
                      BRRlpCoder coder);
/**
 * [QUASI-INTERNAL - used by BREthereumBlock]
 */
extern BRRlpItem
ethExchangeRlpEncode(BREthereumExchange exchange,
                     BREthereumRlpType type,
                     BRRlpCoder coder);

extern void
ethExchangeRelease (BREthereumExchange exchange);

extern BREthereumExchange
ethExchangeCopy (BREthereumExchange exchange);

#if 0
extern void
logsRelease (BRArrayOf(BREthereumLog) logs);

extern void
logReleaseForSet (void *ignore, void *item);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Exchange_h */
