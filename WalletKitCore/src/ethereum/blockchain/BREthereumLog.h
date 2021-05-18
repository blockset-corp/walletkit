//
//  BREthereumLog.h
//  WalletKitCore
//
//  Created by Ed Gamble on 5/10/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Log_h
#define BR_Ethereum_Log_h

#include "ethereum/base/BREthereumBase.h"
#include "BREthereumBloomFilter.h"
#include "BREthereumTransactionStatus.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Log Topic

#define ETHEREUM_LOG_TOPIC_BYTES_COUNT   32

/**
 * An Ethereum Log Topic is 32 bytes of arbitary data.
 */
typedef struct {
    uint8_t bytes[ETHEREUM_LOG_TOPIC_BYTES_COUNT];
} BREthereumLogTopic;

/**
 * Create a LogTopic from a 0x-prefaces, 67 (1 + 2 + 64) character string; otherwise fatal.
 *
 * @param string
 * @return
 */
extern BREthereumLogTopic
ethLogTopicCreateFromString (const char *string);

extern BREthereumBloomFilter
ethLogTopicGetBloomFilter (BREthereumLogTopic topic);

extern BREthereumBloomFilter
ethLogTopicGetBloomFilterAddress (BREthereumAddress address);

extern BREthereumBoolean
ethLogTopicMatchesAddress (BREthereumLogTopic topic,
                        BREthereumAddress address);

typedef struct {
    char chars[2 /* 0x */ + 2 * ETHEREUM_LOG_TOPIC_BYTES_COUNT + 1];
} BREthereumLogTopicString;

extern BREthereumLogTopicString
ethLogTopicAsString (BREthereumLogTopic topic);

extern BREthereumAddress
ethLogTopicAsAddress (BREthereumLogTopic topic);


/// MARK: - Log

/**
 * An Ethereum Log is the output of Smart Contract execution.
 *
 * From the Ethereum specificaion:  A log entry, O, is: {address, types, data}.  To that we add
 * a status, an identifier pair as { transactionHash, transactionReceiptIndex} and a hash (of the
 * identifier).
 */
typedef struct BREthereumLogRecord *BREthereumLog;

extern BREthereumLog
ethLogCreate (BREthereumAddress address,
           unsigned int topicsCount,
           BREthereumLogTopic *topics,
           BRRlpData data);


extern void
ethLogInitializeIdentifier (BREthereumLog log,
                         BREthereumHash transactionHash,
                         size_t transactionReceiptIndex);

/**
 * An identifier for an unknown receipt index.
 */
#define ETHEREUM_LOG_TRANSACTION_RECEIPT_INDEX_UNKNOWN       (SIZE_MAX)

/**
 * Extract the log's identifier components.  A Log is identified by the transaction hash that
 * originated the log and the index of the log in the block's transaction receipts array.
 *
 * If the log has not been recorded in a block, then FALSE is returned.  Otherwise TRUE is returned
 * and `transactionHash` and `transacetionReceiptIndex` will be filled.
 *
 * @param log the log
 * @param transactionHash a hash pointer; if non-NULL will be filled with the transaction's hash
 * @param transactionReceiptIndex an index pointer, if non-NULL will be filled with the logs
 *    index in the block's transaction receipts array.
 *
 * @return TRUE if recorded in a block, FALSE otherwise.
 */
extern BREthereumBoolean
ethLogExtractIdentifier (BREthereumLog log,
                      BREthereumHash *transactionHash,
                      size_t *transactionReceiptIndex);

// Will be ETHEREUM_EMPTY_HASH_INIT if no identifier
extern BREthereumHash
ethLogGetIdentifier (BREthereumLog log);

extern BREthereumHash
ethLogGetHash (BREthereumLog log);

extern BREthereumAddress
ethLogGetAddress (BREthereumLog log);

extern BREthereumBoolean
ethLogHasAddress (BREthereumLog log,
               BREthereumAddress address);

extern size_t
ethLogGetTopicsCount (BREthereumLog log);

extern  BREthereumLogTopic
ethLogGetTopic (BREthereumLog log, size_t index);

extern BRRlpData
ethLogGetData (BREthereumLog log);

extern BRRlpData
ethLogGetDataShared (BREthereumLog log);
    
extern BREthereumBoolean
ethLogMatchesAddress (BREthereumLog log,
                   BREthereumAddress address,
                   BREthereumBoolean topicsOnly);

extern BREthereumComparison
ethLogCompare (BREthereumLog l1,
            BREthereumLog l2);

extern BREthereumTransactionStatus
ethLogGetStatus (BREthereumLog log);

extern void
ethLogSetStatus (BREthereumLog log,
              BREthereumTransactionStatus status);

extern BREthereumBoolean
ethLogIsConfirmed (BREthereumLog log);

extern BREthereumBoolean
ethLogIsErrored (BREthereumLog log);

// Support BRSet
extern size_t
ethLogHashValue (const void *h);

// Support BRSet
extern int
ethLogHashEqual (const void *h1, const void *h2);

extern BREthereumLog
ethLogRlpDecode (BRRlpItem item,
              BREthereumRlpType type,
              BRRlpCoder coder);
/**
 * [QUASI-INTERNAL - used by BREthereumBlock]
 */
extern BRRlpItem
ethLogRlpEncode(BREthereumLog log,
             BREthereumRlpType type,
             BRRlpCoder coder);

extern void
ethLogRelease (BREthereumLog log);

extern void
logsRelease (BRArrayOf(BREthereumLog) logs);

extern void
ethLogReleaseForSet (void *ignore, void *item);
    
extern BREthereumLog
ethLogCopy (BREthereumLog log);

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Log_h */
