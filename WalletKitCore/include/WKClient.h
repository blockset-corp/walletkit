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

typedef enum {
    /// The request itself was flawed.  For example, the URL could not be built.  This error is
    /// an internal error and not recoverable, typiclaly.
    WK_CLIENT_ERROR_BAD_REQUEST,

    /// The request was rejected w/ invalid permission.  For example, a 'client token' has expired
    /// or is otherwise rejected.  This error might be recoverable is the User's permission is
    ///reestablished.
    WK_CLIENT_ERROR_PERMISSION,

    /// The request was rejected having exceeded a resource (rateLimit, dataLimit, etc)
    WK_CLIENT_ERROR_RESOURCE,

    /// The response was flawed.  For example response data could not be parsed or was expected but
    /// was not provided.  This might occur if the client's interface is inconsistent with the
    /// WalletKit interface.
    WK_CLIENT_ERROR_BAD_RESPONSE,

    /// The request and response succeeded, but the submission ultimately failed.  For example,
    /// the Client submitted a Transaction to the Ethereum network but the submission failed with
    /// 'gas_too_low'
    WK_CLIENT_ERROR_SUBMISSION,

    /// The client is unavailable.  For example, the client is dead owing to, say, an internal
    /// error.  Generally resubmitting the request will eventually succeed.
    WK_CLIENT_ERROR_UNAVAILABLE,

    /// The client can't be reached because network connectivity has been lost
    WK_CLIENT_ERROR_LOST_CONNECTIVITY,

} WKClientErrorType;

#define NUMBER_OF_CLIENT_ERROR_TYPES     (1 + WK_CLIENT_ERROR_LOST_CONNECTIVITY)

extern const char *
wkClientErrorTypeDescription (WKClientErrorType type);

typedef struct WKClientErrorRecord *WKClientError;

extern OwnershipGiven WKClientError
wkClientErrorCreate (WKClientErrorType type, const char *details);

extern OwnershipGiven WKClientError
wkClientErrorCreateSubmission (WKTransferSubmitErrorType submitErrorType, const char *details);

extern WKClientErrorType
wkClientErrorGetType (WKClientError error);

// ...

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
wkClientAnnounceBlockNumberSuccess (OwnershipKept WKWalletManager cwm,
                                    OwnershipGiven WKClientCallbackState callbackState,
                                    WKBlockNumber blockNumber,
                                    const char *verifiedBlockHash);

extern void
wkClientAnnounceBlockNumberFailure (OwnershipKept WKWalletManager cwm,
                                    OwnershipGiven WKClientCallbackState callbackState,
                                    OwnershipGiven WKClientError error);

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

static inline int
wkClientTransactionBundleCompareForSort (const void *v1, const void *v2) {
    const WKClientTransactionBundle *b1 = v1;
    const WKClientTransactionBundle *b2 = v2;
    return wkClientTransactionBundleCompare (*b1, *b2);
}

extern int
wkClientTransactionBundleCompareByBlockheight (const WKClientTransactionBundle b1,
                                                   const WKClientTransactionBundle b2);

static inline int
wkClientTransactionBundleCompareByBlockheightForSort (const void *tb1, const void *tb2) {
    WKClientTransactionBundle b1 = * (WKClientTransactionBundle *) tb1;
    WKClientTransactionBundle b2 = * (WKClientTransactionBundle *) tb2;
    return wkClientTransactionBundleCompareByBlockheight (b1, b2);
}

/**
 * Announce the array of bundles with an overall success boolean
 */
extern void
wkClientAnnounceTransactionsSuccess (OwnershipKept WKWalletManager cwm,
                                     OwnershipGiven WKClientCallbackState callbackState,
                                     WKClientTransactionBundle *bundles,
                                     size_t bundlesCount);

extern void
wkClientAnnounceTransactionsFailure (OwnershipKept WKWalletManager cwm,
                                     OwnershipGiven WKClientCallbackState callbackState,
                                     OwnershipGiven WKClientError error);

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
                              OwnershipKept const char */* transaction */ hash,
                              OwnershipKept const char */* transaction */ identifier,
                              OwnershipKept const char */* transfer */ uids,
                              OwnershipKept const char *from,
                              OwnershipKept const char *to,
                              OwnershipKept const char *amount,
                              OwnershipKept const char *currency,
                              OwnershipKept const char *fee,
                              uint64_t transferIndex,
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

static inline int
wkClientTransferBundleCompareForSort (const void *v1, const void *v2) {
    const WKClientTransferBundle *b1 = v1;
    const WKClientTransferBundle *b2 = v2;
    return wkClientTransferBundleCompare (*b1, *b2);
}

extern int
wkClientTransferBundleCompareByBlockheight (const WKClientTransferBundle b1,
                                                const WKClientTransferBundle b2);

static inline int
wkClientTransferBundleCompareByBlockheightForSort (const void *tb1, const void *tb2) {
    WKClientTransferBundle b1 = * (WKClientTransferBundle *) tb1;
    WKClientTransferBundle b2 = * (WKClientTransferBundle *) tb2;
    return wkClientTransferBundleCompareByBlockheight (b1, b2);
}


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
wkClientAnnounceTransfersSuccess (OwnershipKept WKWalletManager cwm,
                                  OwnershipGiven WKClientCallbackState callbackState,
                                  WKClientTransferBundle *bundles,
                                  size_t bundlesCount);

extern void
wkClientAnnounceTransfersFailure (OwnershipKept WKWalletManager cwm,
                                  OwnershipGiven WKClientCallbackState callbackState,
                                  WKClientError error);

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
wkClientAnnounceSubmitTransferSuccess (OwnershipKept WKWalletManager cwm,
                                       OwnershipGiven WKClientCallbackState callbackState,
                                       OwnershipKept const char *identifier,
                                       OwnershipKept const char *hash);

extern void
wkClientAnnounceSubmitTransferFailure (OwnershipKept WKWalletManager cwm,
                                       OwnershipGiven WKClientCallbackState callbackState,
                                       OwnershipGiven WKClientError error);

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
wkClientAnnounceEstimateTransactionFeeSuccess (OwnershipKept WKWalletManager cwm,
                                               OwnershipGiven WKClientCallbackState callbackState,
                                               uint64_t costUnits,
                                               size_t attributesCount,
                                               OwnershipKept const char **attributeKeys,
                                               OwnershipKept const char **attributeVals);

extern void
wkClientAnnounceEstimateTransactionFeeFailure (OwnershipKept WKWalletManager cwm,
                                               OwnershipGiven WKClientCallbackState callbackState,
                                               WKClientError error);

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
wkClientAnnounceCurrenciesSuccess (WKSystem system,
                                   OwnershipGiven WKClientCurrencyBundle *bundles,
                                   size_t bundlesCount);

extern void
wkClientAnnounceCurrenciesFailure (WKSystem system,
                                   WKClientError error);

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
