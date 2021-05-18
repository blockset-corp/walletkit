//
//  WKClient.h
//  WK
//
//  Created by Michael Carrara on 6/19/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKClient_h
#define WKClient_h

#include "WKBase.h"
#include "WKTransfer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A Client Context is an arbitrary ointer that is provided when a Cliet is created and
 * that is passed back in all client class.  This allows the client to establish the context for
 * handling the call.
 */
typedef void *WKClientContext;

/**
 * @brief A Client Callback State is arbitrary data that is passed in a callback and that must
 * be provided in the corresponding 'announce' call.  This allows the wallet manager to establish
 * its context for handling the data
 */
typedef struct WKClientCallbackStateRecord *WKClientCallbackState;

// MARK: - Get Block Number

/**
 * @brief Get the blockchain's most recent block number.
 *
 * @discussion The client calls `wkClientAnnounceBlockNumber()` with the blockchain's current
 * block number and verified block hash (if a verified hash exists).  The callback should be
 * executed asynchrously.  The blockchain of interest can be determined from manager.network.uids.
 */
typedef void
(*WKClientGetBlockNumberCallback) (WKClientContext context,
                                   OwnershipGiven WKWalletManager manager,
                                   OwnershipGiven WKClientCallbackState callbackState);

/**
 * Announce the current block number and the verified block hash.
 */
extern void
wkClientAnnounceBlockNumber (OwnershipKept WKWalletManager cwm,
                             OwnershipGiven WKClientCallbackState callbackState,
                             WKBoolean success,
                             WKBlockNumber blockNumber,
                             const char *verifiedBlockHash);

// MARK: - Get Transactions

/**
 * @brief Get the blockchain's transactions in `[begBlockNumber,endBlockNumber)` for `addresses`
 *
 * @discussion The client calls `wkClientAnnounceTransactions()` with the transactions on the
 * manager's blockchain what involded `addresses` (as 'source' and/or 'target).
 */
typedef void
(*WKClientGetTransactionsCallback) (WKClientContext context,
                                    OwnershipGiven WKWalletManager manager,
                                    OwnershipGiven WKClientCallbackState callbackState,
                                    OwnershipKept const char **addresses,
                                    size_t addressCount,
                                    WKBlockNumber begBlockNumber,
                                    WKBlockNumber endBlockNumber);

/**
 * A TransactionBundle holds the transaction results from the `...GetTransactionsCallback`
 */
typedef struct WKClientTransactionBundleRecord *WKClientTransactionBundle;

/**
 * Create a Transaction Bundle.  The `transaction` byte-array is the raw transaction data in the
 * form appropriate for the blockchain.
 */
extern WKClientTransactionBundle
wkClientTransactionBundleCreate (WKTransferStateType status,
                                 OwnershipKept uint8_t *transaction,
                                 size_t transactionLength,
                                 WKTimestamp timestamp,
                                 WKBlockNumber blockHeight);

/**
 * Release a Transaction Bundle
 */
extern void
wkClientTransactionBundleRelease (WKClientTransactionBundle bundle);

/**
 * Order two transaction bundles such that: b1 < b2 => -1; b1 > b2 => +1, b1 == b2 => 0.
 */
extern int
wkClientTransactionBundleCompare (const WKClientTransactionBundle b1,
                                  const WKClientTransactionBundle b2);

/**
 * Announce the array of bundles with an overall success boolean
 */
extern void
wkClientAnnounceTransactions (OwnershipKept WKWalletManager cwm,
                              OwnershipGiven WKClientCallbackState callbackState,
                              WKBoolean success,
                              WKClientTransactionBundle *bundles,
                              size_t bundlesCount);

// MARK: - Get Transfers

/**
 * @brief Get the blockchain's transfer in `[begBlockNumber,endBlockNumber)` for `addresses`
 *
 * @discussion The client calls `wkClientAnnounceTransfers()` with the transfers on the
 * manager's blockchain what involded `addresses` (as 'source' and/or 'target).  Transfers are
 * not necessarily a blockchain concept; they are an abstraction representing the exchange of an
 * asset.
 */
typedef void
(*WKClientGetTransfersCallback) (WKClientContext context,
                                 OwnershipGiven WKWalletManager manager,
                                 OwnershipGiven WKClientCallbackState callbackState,
                                 OwnershipKept const char **addresses,
                                 size_t addressCount,
                                 WKBlockNumber begBlockNumber,
                                 WKBlockNumber endBlockNumber);

/**
 * A TransferBundle holds the transfer results from the `...GetTransfersCallback`
 */
typedef struct WKClientTransferBundleRecord *WKClientTransferBundle;

/**
 * Create a Transfer Bundle
 */
extern WKClientTransferBundle
wkClientTransferBundleCreate (WKTransferStateType status,
                              OwnershipKept const char *uids,
                              OwnershipKept const char *hash,
                              OwnershipKept const char *identifier,
                              OwnershipKept const char *from,
                              OwnershipKept const char *to,
                              OwnershipKept const char *amount,
                              OwnershipKept const char *currency,
                              OwnershipKept const char *fee,
                              WKTimestamp blockTimestamp,
                              WKBlockNumber blockNumber,
                              WKBlockNumber blockConfirmations,
                              uint64_t blockTransactionIndex,
                              OwnershipKept const char *blockHash,
                              size_t attributesCount,
                              OwnershipKept const char **attributeKeys,
                              OwnershipKept const char **attributeVals);

/**
 * Release a transfer bundle
 */
extern void
wkClientTransferBundleRelease (WKClientTransferBundle bundle);

/**
 * Order two transfer bundles such that: b1 < b2 => -1; b1 > b2 => +1, b1 == b2 => 0.
 */
extern int
wkClientTransferBundleCompare (const WKClientTransferBundle b1,
                               const WKClientTransferBundle b2);

/**
 * Get the transfer bundle's state.
 */
extern WKTransferState
wkClientTransferBundleGetTransferState (const WKClientTransferBundle bundle,
                                        WKFeeBasis confirmedFeeBasis);

/**
 * Announce an array of transfer bundles.
 */
extern void
wkClientAnnounceTransfers (OwnershipKept WKWalletManager cwm,
                           OwnershipGiven WKClientCallbackState callbackState,
                           WKBoolean success,
                           WKClientTransferBundle *bundles,
                           size_t bundlesCount);

// MARK: - Submit Transaction

/**
 * @brief Submit a transaction
 *
 * @discussion The client calls `wkClientAnnounceSubmitTransfer()` with the result of the
 * submission.
 */
typedef void
(*WKClientSubmitTransactionCallback) (WKClientContext context,
                                      OwnershipGiven WKWalletManager manager,
                                      OwnershipGiven WKClientCallbackState callbackState,
                                      OwnershipKept const char    *identifier,
                                      OwnershipKept const uint8_t *transaction,
                                      size_t transactionLength);

/**
 * Announce the result of the submission.
 */
extern void
wkClientAnnounceSubmitTransfer (OwnershipKept WKWalletManager cwm,
                                OwnershipGiven WKClientCallbackState callbackState,
                                OwnershipKept const char *identifier,
                                OwnershipKept const char *hash,
                                WKBoolean success);

// MARK: - Estimate Transaction Fee

/**
 * @brief Estimate the fees for `transaction`
 *
 * @discussion The client calls `wkClientAnnounceEstimateTransactionFee()` with the estimated
 * fees and any associated data.
 */
typedef void
(*WKClientEstimateTransactionFeeCallback) (WKClientContext context,
                                           OwnershipGiven WKWalletManager manager,
                                           OwnershipGiven WKClientCallbackState callbackState,
                                           OwnershipKept const uint8_t *transaction,
                                           size_t transactionLength,
                                           OwnershipKept const char *hashAsHex);

/**
 * Announce the result of fee estimation
 */
extern void
wkClientAnnounceEstimateTransactionFee (OwnershipKept WKWalletManager cwm,
                                        OwnershipGiven WKClientCallbackState callbackState,
                                        WKBoolean success,
                                        uint64_t costUnits,
                                        size_t attributesCount,
                                        OwnershipKept const char **attributeKeys,
                                        OwnershipKept const char **attributeVals);

// MARK: - Currency

/**
 * A Currency Bundle holds the results of a 'GetCurrency' callback (private)
 */
typedef struct WKClientCurrencyBundleRecord *WKClientCurrencyBundle;

/**
 * A Currency Denomination Bundle holds the result of a `GetCurrency` callbaack (private)
 */
typedef struct WKCliehtCurrencyDenominationBundleRecord *WKClientCurrencyDenominationBundle;

/**
 * Create a Currency Denomination Bundle
 */
extern WKClientCurrencyDenominationBundle
wkClientCurrencyDenominationBundleCreate (const char *name,
                                          const char *code,
                                          const char *symbol,
                                          uint8_t     decimals);

/**
 * Create a Currecny Bundle
 */
extern WKClientCurrencyBundle
wkClientCurrencyBundleCreate (const char *id,
                              const char *name,
                              const char *code,
                              const char *type,
                              const char *blockchainId,
                              const char *address,
                              bool verified,
                              size_t denominationsCount,
                              WKClientCurrencyDenominationBundle *denominations);

/**
 * Release a Currency Bundle
 */
extern void
wkClientCurrencyBundleRelease (WKClientCurrencyBundle bundle);

/**
 * Announce the result of a `GetCurrency' callback (private).
 */
extern void
wkClientAnnounceCurrencies (WKSystem system,
                            OwnershipGiven WKClientCurrencyBundle *bundles,
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
    WKClientContext context;
    WKClientGetBlockNumberCallback  funcGetBlockNumber;
    WKClientGetTransactionsCallback funcGetTransactions;
    WKClientGetTransfersCallback funcGetTransfers;
    WKClientSubmitTransactionCallback funcSubmitTransaction;
    WKClientEstimateTransactionFeeCallback funcEstimateTransactionFee;
} WKClient;

#ifdef __cplusplus
}
#endif

#endif /* WKClient_h */
