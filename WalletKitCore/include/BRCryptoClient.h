//
//  BRCryptoClient.h
//  BRCrypto
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoClient_h
#define BRCryptoClient_h

#include "BRCryptoBase.h"
#include "BRCryptoTransfer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *BRCryptoClientContext;

typedef struct BRCryptoClientCallbackStateRecord *BRCryptoClientCallbackState;

// MARK: - Get Block Number

typedef void
(*BRCryptoClientGetBlockNumberCallback) (BRCryptoClientContext context,
                                         OwnershipGiven BRCryptoWalletManager manager,
                                         OwnershipGiven BRCryptoClientCallbackState callbackState);

extern void
cwmAnnounceBlockNumber (OwnershipKept BRCryptoWalletManager cwm,
                        OwnershipGiven BRCryptoClientCallbackState callbackState,
                        BRCryptoBoolean success,
                        uint64_t blockNumber);

// MARK: - Get Transactions

typedef void
(*BRCryptoClientGetTransactionsCallback) (BRCryptoClientContext context,
                                          OwnershipGiven BRCryptoWalletManager manager,
                                          OwnershipGiven BRCryptoClientCallbackState callbackState,
                                          OwnershipKept const char **addresses,
                                          size_t addressCount,
                                          uint64_t begBlockNumber,
                                          uint64_t endBlockNumber);

typedef struct BRCryptoClientTransactionBundleRecord *BRCryptoClientTransactionBundle;

extern BRCryptoClientTransactionBundle
cryptoClientTransactionBundleCreate (BRCryptoTransferStateType status,
                                     OwnershipKept uint8_t *transaction,
                                     size_t transactionLength,
                                     uint64_t timestamp,
                                     uint64_t blockHeight);

extern void
cryptoClientTransactionBundleRelease (BRCryptoClientTransactionBundle bundle);

// b1 < b2 => -1; b1 > b2 => +1, b1 == b2 => 0
extern int
cryptoClientTransactionBundleCompare (const BRCryptoClientTransactionBundle b1,
                                      const BRCryptoClientTransactionBundle b2);

extern void
cwmAnnounceTransactions (OwnershipKept BRCryptoWalletManager cwm,
                         OwnershipGiven BRCryptoClientCallbackState callbackState,
                         BRCryptoBoolean success,
                         BRCryptoClientTransactionBundle *bundles,
                         size_t bundlesCount);

// MARK: - Get Transfers

typedef void
(*BRCryptoClientGetTransfersCallback) (BRCryptoClientContext context,
                                       OwnershipGiven BRCryptoWalletManager manager,
                                       OwnershipGiven BRCryptoClientCallbackState callbackState,
                                       OwnershipKept const char **addresses,
                                       size_t addressCount,
                                       uint64_t begBlockNumber,
                                       uint64_t endBlockNumber);

typedef struct BRCryptoClientTransferBundleRecord *BRCryptoClientTransferBundle;

extern BRCryptoClientTransferBundle
cryptoClientTransferBundleCreate (BRCryptoTransferStateType status,
                                  OwnershipKept const char *hash,
                                  OwnershipKept const char *uids,
                                  OwnershipKept const char *from,
                                  OwnershipKept const char *to,
                                  OwnershipKept const char *amount,
                                  OwnershipKept const char *currency,
                                  OwnershipKept const char *fee,
                                  uint64_t blockTimestamp,
                                  uint64_t blockNumber,
                                  uint64_t blockConfirmations,
                                  uint64_t blockTransactionIndex,
                                  OwnershipKept const char *blockHash,
                                  size_t attributesCount,
                                  OwnershipKept const char **attributeKeys,
                                  OwnershipKept const char **attributeVals);

extern void
cryptoClientTransferBundleRelease (BRCryptoClientTransferBundle bundle);

extern int
cryptoClientTransferBundleCompare (const BRCryptoClientTransferBundle b1,
                                   const BRCryptoClientTransferBundle b2);

extern void
cwmAnnounceTransfers (OwnershipKept BRCryptoWalletManager cwm,
                      OwnershipGiven BRCryptoClientCallbackState callbackState,
                      BRCryptoBoolean success,
                      BRCryptoClientTransferBundle *bundles,
                      size_t bundlesCount);

// MARK: - Submit Transaction

typedef void
(*BRCryptoClientSubmitTransactionCallback) (BRCryptoClientContext context,
                                            OwnershipGiven BRCryptoWalletManager manager,
                                            OwnershipGiven BRCryptoClientCallbackState callbackState,
                                            OwnershipKept const uint8_t *transaction,
                                            size_t transactionLength,
                                            OwnershipKept const char *hashAsHex);

extern void
cwmAnnounceSubmitTransfer (OwnershipKept BRCryptoWalletManager cwm,
                           OwnershipGiven BRCryptoClientCallbackState callbackState,
                           BRCryptoBoolean success);

// MARK: - Estimate Transaction Fee

typedef void
(*BRCryptoClientEstimateTransactionFeeCallback) (BRCryptoClientContext context,
                                                 OwnershipGiven BRCryptoWalletManager manager,
                                                 OwnershipGiven BRCryptoClientCallbackState callbackState,
                                                 OwnershipKept const uint8_t *transaction,
                                                 size_t transactionLength,
                                                 OwnershipKept const char *hashAsHex);

extern void
cwmAnnounceEstimateTransactionFee (OwnershipKept BRCryptoWalletManager cwm,
                                   OwnershipGiven BRCryptoClientCallbackState callbackState,
                                   BRCryptoBoolean success,
                                   OwnershipKept const char *hash,
                                   uint64_t costUnits,
                                   size_t attributesCount,
                                   OwnershipKept const char **attributeKeys,
                                   OwnershipKept const char **attributeVals);

typedef struct {
    BRCryptoClientContext context;
    BRCryptoClientGetBlockNumberCallback  funcGetBlockNumber;
    BRCryptoClientGetTransactionsCallback funcGetTransactions;
    BRCryptoClientGetTransfersCallback funcGetTransfers;
    BRCryptoClientSubmitTransactionCallback funcSubmitTransaction;
    BRCryptoClientEstimateTransactionFeeCallback funcEstimateTransactionFee;
} BRCryptoClient;

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoClient_h */
