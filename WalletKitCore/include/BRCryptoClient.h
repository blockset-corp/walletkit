//
//  BRCryptoClient.h
//  BRCrypto
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
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

/**
 * @brief A Client Context is an arbitrary ointer that is provided when a Cliet is created and
 * that is passed back in all client class.  This allows the client to establish the context for
 * handling the call.
 */
typedef void *BRCryptoClientContext;

/**
 * @brief A Client Callback State is arbitrary data that is passed in a callback and that must
 * be provided in the corresponding 'announce' call.  This allows the wallet manager to establish
 * its context for handling the data
 */
typedef struct BRCryptoClientCallbackStateRecord *BRCryptoClientCallbackState;

// MARK: - Get Block Number

/**
 * @brief Get the blockchain's most recent block number.
 *
 * @discussion The client calls `cryptoClientAnnounceBlockNumber()` with the blockchain's current
 * block number and verified block hash (if a verified hash exists).  The callback should be
 * executed asynchrously.  The blockchain of interest can be determined from manager.network.uids.
 */
typedef void
(*BRCryptoClientGetBlockNumberCallback) (BRCryptoClientContext context,
                                         OwnershipGiven BRCryptoWalletManager manager,
                                         OwnershipGiven BRCryptoClientCallbackState callbackState);

/**
 * Announce the current block number and the verified block hash.
 */
extern void
cryptoClientAnnounceBlockNumber (OwnershipKept BRCryptoWalletManager cwm,
                                 OwnershipGiven BRCryptoClientCallbackState callbackState,
                                 BRCryptoBoolean success,
                                 BRCryptoBlockNumber blockNumber,
                                 const char *verifiedBlockHash);

// MARK: - Get Transactions

/**
 * @brief Get the blockchain's transactions in `[begBlockNumber,endBlockNumber)` for `addresses`
 *
 * @discussion The client calls `cryptoClientAnnounceTransactions()` with the transactions on the
 * manager's blockchain what involded `addresses` (as 'source' and/or 'target).
 */
typedef void
(*BRCryptoClientGetTransactionsCallback) (BRCryptoClientContext context,
                                          OwnershipGiven BRCryptoWalletManager manager,
                                          OwnershipGiven BRCryptoClientCallbackState callbackState,
                                          OwnershipKept const char **addresses,
                                          size_t addressCount,
                                          BRCryptoBlockNumber begBlockNumber,
                                          BRCryptoBlockNumber endBlockNumber);

/**
 * A TransactionBundle holds the transaction results from the `...GetTransactionsCallback`
 */
typedef struct BRCryptoClientTransactionBundleRecord *BRCryptoClientTransactionBundle;

/**
 * Create a Transaction Bundle.  The `transaction` byte-array is the raw transaction data in the
 * form appropriate for the blockchain.
 */
extern BRCryptoClientTransactionBundle
cryptoClientTransactionBundleCreate (BRCryptoTransferStateType status,
                                     OwnershipKept uint8_t *transaction,
                                     size_t transactionLength,
                                     BRCryptoTimestamp timestamp,
                                     BRCryptoBlockNumber blockHeight);

/**
 * Release a Transaction Bundle
 */
extern void
cryptoClientTransactionBundleRelease (BRCryptoClientTransactionBundle bundle);

/**
 * Order two transaction bundles such that: b1 < b2 => -1; b1 > b2 => +1, b1 == b2 => 0.
 */
extern int
cryptoClientTransactionBundleCompare (const BRCryptoClientTransactionBundle b1,
                                      const BRCryptoClientTransactionBundle b2);

/**
 * Announce the array of bundles with an overall success boolean
 */
extern void
cryptoClientAnnounceTransactions (OwnershipKept BRCryptoWalletManager cwm,
                                  OwnershipGiven BRCryptoClientCallbackState callbackState,
                                  BRCryptoBoolean success,
                                  BRCryptoClientTransactionBundle *bundles,
                                  size_t bundlesCount);

// MARK: - Get Transfers

/**
 * @brief Get the blockchain's transfer in `[begBlockNumber,endBlockNumber)` for `addresses`
 *
 * @discussion The client calls `cryptoClientAnnounceTransfers()` with the transfers on the
 * manager's blockchain what involded `addresses` (as 'source' and/or 'target).  Transfers are
 * not necessarily a blockchain concept; they are an abstraction representing the exchange of an
 * asset.
 */
typedef void
(*BRCryptoClientGetTransfersCallback) (BRCryptoClientContext context,
                                       OwnershipGiven BRCryptoWalletManager manager,
                                       OwnershipGiven BRCryptoClientCallbackState callbackState,
                                       OwnershipKept const char **addresses,
                                       size_t addressCount,
                                       BRCryptoBlockNumber begBlockNumber,
                                       BRCryptoBlockNumber endBlockNumber);

/**
 * A TransferBundle holds the transfer results from the `...GetTransfersCallback`
 */
typedef struct BRCryptoClientTransferBundleRecord *BRCryptoClientTransferBundle;

/**
 * Create a Transfer Bundle
 */
extern BRCryptoClientTransferBundle
cryptoClientTransferBundleCreate (BRCryptoTransferStateType status,
                                  OwnershipKept const char *uids,
                                  OwnershipKept const char *hash,
                                  OwnershipKept const char *identifier,
                                  OwnershipKept const char *from,
                                  OwnershipKept const char *to,
                                  OwnershipKept const char *amount,
                                  OwnershipKept const char *currency,
                                  OwnershipKept const char *fee,
                                  BRCryptoTimestamp blockTimestamp,
                                  BRCryptoBlockNumber blockNumber,
                                  BRCryptoBlockNumber blockConfirmations,
                                  uint64_t blockTransactionIndex,
                                  OwnershipKept const char *blockHash,
                                  size_t attributesCount,
                                  OwnershipKept const char **attributeKeys,
                                  OwnershipKept const char **attributeVals);

/**
 * Release a transfer bundle
 */
extern void
cryptoClientTransferBundleRelease (BRCryptoClientTransferBundle bundle);

/**
 * Order two transfer bundles such that: b1 < b2 => -1; b1 > b2 => +1, b1 == b2 => 0.
 */
extern int
cryptoClientTransferBundleCompare (const BRCryptoClientTransferBundle b1,
                                   const BRCryptoClientTransferBundle b2);

/**
 * Get the transfer bundle's state.
 */
extern BRCryptoTransferState
cryptoClientTransferBundleGetTransferState (const BRCryptoClientTransferBundle bundle,
                                            BRCryptoFeeBasis confirmedFeeBasis);

/**
 * Announce an array of transfer bundles.
 */
extern void
cryptoClientAnnounceTransfers (OwnershipKept BRCryptoWalletManager cwm,
                               OwnershipGiven BRCryptoClientCallbackState callbackState,
                               BRCryptoBoolean success,
                               BRCryptoClientTransferBundle *bundles,
                               size_t bundlesCount);

// MARK: - Submit Transaction

/**
 * @brief Submit a transaction
 *
 * @discussion The client calls `cryptoClientAnnounceSubmitTransfer()` with the result of the
 * submission.
 */
typedef void
(*BRCryptoClientSubmitTransactionCallback) (BRCryptoClientContext context,
                                            OwnershipGiven BRCryptoWalletManager manager,
                                            OwnershipGiven BRCryptoClientCallbackState callbackState,
                                            OwnershipKept const char    *identifier,
                                            OwnershipKept const uint8_t *transaction,
                                            size_t transactionLength);

/**
 * Announce the result of the submission.
 */
extern void
cryptoClientAnnounceSubmitTransfer (OwnershipKept BRCryptoWalletManager cwm,
                                    OwnershipGiven BRCryptoClientCallbackState callbackState,
                                    OwnershipKept const char *identifier,
                                    OwnershipKept const char *hash,
                                    BRCryptoBoolean success);

// MARK: - Estimate Transaction Fee

/**
 * @brief Estimate the fees for `transaction`
 *
 * @discussion The client calls `cryptoClientAnnounceEstimateTransactionFee()` with the estimated
 * fees and any associated data.
 */
typedef void
(*BRCryptoClientEstimateTransactionFeeCallback) (BRCryptoClientContext context,
                                                 OwnershipGiven BRCryptoWalletManager manager,
                                                 OwnershipGiven BRCryptoClientCallbackState callbackState,
                                                 OwnershipKept const uint8_t *transaction,
                                                 size_t transactionLength,
                                                 OwnershipKept const char *hashAsHex);

/**
 * Announce the result of fee estimation
 */
extern void
cryptoClientAnnounceEstimateTransactionFee (OwnershipKept BRCryptoWalletManager cwm,
                                            OwnershipGiven BRCryptoClientCallbackState callbackState,
                                            BRCryptoBoolean success,
                                            uint64_t costUnits,
                                            size_t attributesCount,
                                            OwnershipKept const char **attributeKeys,
                                            OwnershipKept const char **attributeVals);

// MARK: - Currency

/**
 * A Currency Bundle holds the results of a 'GetCurrency' callback (private)
 */
typedef struct BRCryptoClientCurrencyBundleRecord *BRCryptoClientCurrencyBundle;

/**
 * A Currency Denomination Bundle holds the result of a `GetCurrency` callbaack (private)
 */
typedef struct BRCryptoCliehtCurrencyDenominationBundleRecord *BRCryptoClientCurrencyDenominationBundle;

/**
 * Create a Currency Denomination Bundle
 */
extern BRCryptoClientCurrencyDenominationBundle
cryptoClientCurrencyDenominationBundleCreate (const char *name,
                                              const char *code,
                                              const char *symbol,
                                              uint8_t     decimals);

/**
 * Create a Currecny Bundle
 */
extern BRCryptoClientCurrencyBundle
cryptoClientCurrencyBundleCreate (const char *id,
                                  const char *name,
                                  const char *code,
                                  const char *type,
                                  const char *blockchainId,
                                  const char *address,
                                  bool verified,
                                  size_t denominationsCount,
                                  BRCryptoClientCurrencyDenominationBundle *denominations);

/**
 * Release a Currency Bundle
 */
extern void
cryptoClientCurrencyBundleRelease (BRCryptoClientCurrencyBundle bundle);

/**
 * Announce the result of a `GetCurrency' callback (private).
 */
extern void
cryptoClientAnnounceCurrencies (BRCryptoSystem system,
                                OwnershipGiven BRCryptoClientCurrencyBundle *bundles,
                                size_t bundlesCount);

/**
 * @brief A Client implements a number of callback functions that provide WalletKit with
 * blockchain data.
 *
 * @discussion
 *
 * The client includes a context that can be used by the client to establish a context for handling
 * callbacks.
 */
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
