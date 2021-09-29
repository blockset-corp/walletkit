//
//  BREthereumTransactionReceipt.h
//  WalletKitCore
//
//  Created by Ed Gamble on 5/10/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Transaction_Receipt_h
#define BR_Ethereum_Transaction_Receipt_h

#include "ethereum/base/BREthereumBase.h"
#include "BREthereumBloomFilter.h"
#include "BREthereumLog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An Etheruem Transaction Receipt contains data pertinent to the execution of a transaction.
 *
 * As per the Ethereum specification: The transaction receipt, R, is a tuple of four items
 * comprising: {gasUsed, logs, bloomfilter, statusCode}
 *
 * [Note: there appears to be a change in interpretation for 'status code'; it is shown here
 * as stateRoot
 */
typedef struct BREthereumTransactionReceiptRecord *BREthereumTransactionReceipt;

extern uint64_t
ethTransactionReceiptGetGasUsed (BREthereumTransactionReceipt receipt);

extern size_t
ethTransactionReceiptGetLogsCount (BREthereumTransactionReceipt receipt);

extern BREthereumLog
ethTransactionReceiptGetLog (BREthereumTransactionReceipt receipt, size_t index);

extern BREthereumBloomFilter
ethTransactionReceiptGetBloomFilter (BREthereumTransactionReceipt receipt);

extern BREthereumBoolean
ethTransactionReceiptMatch (BREthereumTransactionReceipt receipt,
                         BREthereumBloomFilter filter);

extern BREthereumBoolean
ethTransactionReceiptMatchAddress (BREthereumTransactionReceipt receipt,
                                BREthereumAddress address);

extern BREthereumTransactionReceipt
ethTransactionReceiptRlpDecode (BRRlpItem item,
                             BRRlpCoder coder);
    
extern BRRlpItem
ethTransactionReceiptRlpEncode(BREthereumTransactionReceipt receipt,
                            BRRlpCoder coder);

extern BRArrayOf (BREthereumTransactionReceipt)
ethTransactionReceiptDecodeList (BRRlpItem item,
                              BRRlpCoder coder);

extern void
ethTransactionReceiptRelease (BREthereumTransactionReceipt receipt);

extern void
ethTransactionReceiptsRelease (BRArrayOf(BREthereumTransactionReceipt) receipts);

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Transaction_Receipt_h */
