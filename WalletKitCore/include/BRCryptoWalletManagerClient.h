//
//  BRCryptoWalletManagerClient.h
//  BRCrypto
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoWalletManagerClient_h
#define BRCryptoWalletManagerClient_h

#include "BRCryptoBase.h"
#include "BRCryptoNetwork.h"
#include "BRCryptoAccount.h"
#include "BRCryptoStatus.h"
#include "BRCryptoTransfer.h"
#include "BRCryptoWallet.h"

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
cwmAnnounceGetBlockNumberSuccess (OwnershipKept BRCryptoWalletManager cwm,
                                  OwnershipGiven BRCryptoClientCallbackState callbackState,
                                  uint64_t blockNumber);

extern void
cwmAnnounceGetBlockNumberFailure (OwnershipKept BRCryptoWalletManager cwm,
                                  OwnershipGiven BRCryptoClientCallbackState callbackState);

// MARK: - Get Transactions

typedef void
(*BRCryptoClientGetTransactionsCallback) (BRCryptoClientContext context,
                                          OwnershipGiven BRCryptoWalletManager manager,
                                          OwnershipGiven BRCryptoClientCallbackState callbackState,
                                          OwnershipKept const char **addresses,
                                          size_t addressCount,
                                          OwnershipKept const char *currency,
                                          uint64_t begBlockNumber,
                                          uint64_t endBlockNumber);

extern void
cwmAnnounceGetTransactionsItem (OwnershipKept BRCryptoWalletManager cwm,
                                OwnershipGiven BRCryptoClientCallbackState callbackState,
                                BRCryptoTransferStateType status,
                                OwnershipKept uint8_t *transaction,
                                size_t transactionLength,
                                uint64_t timestamp,
                                uint64_t blockHeight);

extern void
cwmAnnounceGetTransactionsComplete (OwnershipKept BRCryptoWalletManager cwm,
                                    OwnershipGiven BRCryptoClientCallbackState callbackState,
                                    BRCryptoBoolean success);

// MARK: - Get Transfers

typedef void
(*BRCryptoClientGetTransfersCallback) (BRCryptoClientContext context,
                                       OwnershipGiven BRCryptoWalletManager manager,
                                       OwnershipGiven BRCryptoClientCallbackState callbackState,
                                       OwnershipKept const char **addresses,
                                       size_t addressCount,
                                       OwnershipKept const char *currency,
                                       uint64_t begBlockNumber,
                                       uint64_t endBlockNumber);

extern void
cwmAnnounceGetTransferItem (BRCryptoWalletManager cwm,
                            BRCryptoClientCallbackState callbackState,
                            BRCryptoTransferStateType status,
                            OwnershipKept const char *hash,
                            OwnershipKept const char *uids,
                            OwnershipKept const char *from,
                            OwnershipKept const char *to,
                            OwnershipKept const char *amount,
                            OwnershipKept const char *currency,
                            OwnershipKept const char *fee,
                            OwnershipKept const char *transactionBytes,
                            uint64_t blockTimestamp,
                            uint64_t blockNumber,
                            uint64_t blockConfirmations,
                            uint64_t blockTransactionIndex,
                            OwnershipKept const char *blockHash,
                            size_t attributesCount,
                            OwnershipKept const char **attributeKeys,
                            OwnershipKept const char **attributeVals);

extern void
cwmAnnounceGetTransfersComplete (OwnershipKept BRCryptoWalletManager cwm,
                                 OwnershipGiven BRCryptoClientCallbackState callbackState,
                                 BRCryptoBoolean success);

// MARK: - Submit Transaction

typedef void
(*BRCryptoClientSubmitTransactionCallback) (BRCryptoClientContext context,
                                            OwnershipGiven BRCryptoWalletManager manager,
                                            OwnershipGiven BRCryptoClientCallbackState callbackState,
                                            OwnershipKept const uint8_t *transaction,
                                            size_t transactionLength,
                                            OwnershipKept const char *hashAsHex);

extern void
cwmAnnounceSubmitTransferSuccess (OwnershipKept BRCryptoWalletManager cwm,
                                  OwnershipGiven BRCryptoClientCallbackState callbackState,
                                  OwnershipKept const char *hash);

extern void
cwmAnnounceSubmitTransferFailure (OwnershipKept BRCryptoWalletManager cwm,
                                  OwnershipGiven BRCryptoClientCallbackState callbackState);

// MARK: - Estimate Transaction Fee

typedef void
(*BRCryptoClientEstimateTransactionFeeCallback) (BRCryptoClientContext context,
                                                 OwnershipGiven BRCryptoWalletManager manager,
                                                 OwnershipGiven BRCryptoClientCallbackState callbackState,
                                                 OwnershipKept const uint8_t *transaction,
                                                 size_t transactionLength,
                                                 OwnershipKept const char *hashAsHex);

extern void
cwmAnnounceEstimateTransactionFeeSuccess (OwnershipKept BRCryptoWalletManager cwm,
                                          OwnershipGiven BRCryptoClientCallbackState callbackState,
                                          OwnershipKept const char *hash,
                                          uint64_t costUnits);

extern void
cwmAnnounceEstimateTransactionFeeFailure (OwnershipKept BRCryptoWalletManager cwm,
                                          OwnershipGiven BRCryptoClientCallbackState callbackState,
                                          OwnershipKept const char *hash);

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

#endif /* BRCryptoWalletManagerClient_h */
