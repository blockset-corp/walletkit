//
//  BRCryptoETH.h
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRCryptoETH_h
#define BRCryptoETH_h

#include "../BRCryptoHandlersExport.h"

#include "ethereum/base/BREthereumBase.h"
#include "ethereum/blockchain/BREthereumAccount.h"
#include "ethereum/blockchain/BREthereumTransaction.h"
#include "ethereum/blockchain/BREthereumLog.h"
#include "ethereum/contract/BREthereumExchange.h"

#include "ethereum/bcs/BREthereumBCS.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct BRCryptoAddressETHRecord {
    struct BRCryptoAddressRecord base;
    BREthereumAddress eth;
} *BRCryptoAddressETH;

private_extern BRCryptoAddress
cryptoAddressCreateAsETH (BREthereumAddress eth);

private_extern BREthereumAddress
cryptoAddressAsETH (BRCryptoAddress address);

// MARK: - Network

typedef struct BRCryptoNetworkETHRecord {
    struct BRCryptoNetworkRecord base;
    BREthereumNetwork eth;
} *BRCryptoNetworkETH;

private_extern BREthereumNetwork
cryptoNetworkAsETH (BRCryptoNetwork network);

private_extern BREthereumGasPrice
cryptoNetworkFeeAsETH (BRCryptoNetworkFee fee);

// MARK: - Transfer

typedef enum  {
    TRANSFER_BASIS_TRANSACTION,
    TRANSFER_BASIS_LOG,
    TRANSFER_BASIS_EXCHANGE
} BREthereumTransferBasisType;

typedef struct {
    BREthereumTransferBasisType type;
    union {
        BREthereumTransaction transaction;
        BREthereumLog log;
        BREthereumExchange exchange;
    } u;
} BREthereumTransferBasis;

/// A ETH Transfer
typedef struct BRCryptoTransferETHRecord {
    struct BRCryptoTransferRecord base;

    BREthereumAccount account;
    BREthereumGas gasEstimate;
    BREthereumTransferBasis basis;

    // The tranaction that originated this transfer.  Will be NULL if the transaction's source
    // address is not in `account`.  My be NULL if the transaction has not been seen yet.
    BREthereumTransaction originatingTransaction;
} *BRCryptoTransferETH;

extern BRCryptoTransferETH
cryptoTransferCoerceETH (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsETH (BRCryptoTransferListener listener,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRCryptoFeeBasis feeBasisEstimated,
                           BRCryptoAmount amount,
                           BRCryptoTransferDirection direction,
                           BRCryptoAddress sourceAddress,
                           BRCryptoAddress targetAddress,
                           BRCryptoTransferState transferState,
                           BREthereumAccount account,
                           BREthereumTransferBasis basis,
                           OwnershipGiven BREthereumTransaction originatingTransaction);

extern BRCryptoTransfer
cryptoTransferCreateWithTransactionAsETH (BRCryptoTransferListener listener,
                                          BRCryptoUnit unit,
                                          BRCryptoUnit unitForFee,
                                          BREthereumAccount account,
                                          OwnershipGiven BREthereumTransaction ethTransaction);

extern BRCryptoTransfer
cryptoTransferCreateWithLogAsETH (BRCryptoTransferListener listener,
                                  BRCryptoUnit unit,
                                  BRCryptoUnit unitForFee,
                                  BREthereumAccount account,
                                  UInt256 ethAmount,
                                  OwnershipGiven BREthereumLog ethLog);

extern BRCryptoTransfer
cryptoTransferCreateWithExchangeAsETH (BRCryptoTransferListener listener,
                                       BRCryptoUnit unit,
                                       BRCryptoUnit unitForFee,
                                       BREthereumAccount account,
                                       UInt256 ethAmount,
                                       OwnershipGiven BREthereumExchange ethExchange);

extern const BREthereumHash
cryptoTransferGetIdentifierETH (BRCryptoTransferETH transfer);

extern const BREthereumHash
cryptoTransferGetOriginatingTransactionHashETH (BRCryptoTransferETH transfer);

extern BRCryptoTransferState
cryptoTransferDeriveStateETH (BREthereumTransactionStatus status,
                              BRCryptoFeeBasis feeBasis);

// MARK: - Wallet

typedef struct BRCryptoWalletETHRecord {
    struct BRCryptoWalletRecord base;
    
    BREthereumAccount ethAccount;
    BREthereumToken   ethToken;    // NULL if `ETH`
    BREthereumGas     ethGasLimit;
} *BRCryptoWalletETH;

extern BRCryptoWalletETH
cryptoWalletCoerce (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsETH (BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BREthereumToken   ethToken,
                         BREthereumAccount ethAccount);

extern BRCryptoTransfer
cryptoWalletCreateTransferETH (BRCryptoWallet  wallet,
                               BRCryptoAddress target,
                               BRCryptoAmount  amount,
                               BRCryptoFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept BRCryptoTransferAttribute *attributes,
                               BRCryptoCurrency currency,
                               BRCryptoUnit unit,
                               BRCryptoUnit unitForFee);

extern BRCryptoTransferETH
cryptoWalletLookupTransferByIdentifier (BRCryptoWalletETH wallet,
                                        BREthereumHash identifier);
extern BRCryptoTransferETH
cryptoWalletLookupTransferByOriginatingHash (BRCryptoWalletETH wallet,
                                             BREthereumHash hash);

// MARK: - Wallet Manager

typedef struct BRCryptoWalletManagerETHRecord {
    struct BRCryptoWalletManagerRecord base;

    BREthereumNetwork network;
    BREthereumAccount account;

    BRSetOf(BREthereumToken) tokens;

    BRRlpCoder coder;

} *BRCryptoWalletManagerETH;

extern BRCryptoWalletManagerETH
cryptoWalletManagerCoerceETH (BRCryptoWalletManager manager);

private_extern BRCryptoWalletETH
cryptoWalletManagerEnsureWalletForToken (BRCryptoWalletManagerETH managerETH,
                                         BREthereumToken token);

// MARK: - P2P

extern BRCryptoClientP2PManager
cryptoWalletManagerCreateP2PManagerETH (BRCryptoWalletManager manager);

private_extern void
ewmHandleTransaction (BREthereumBCSCallbackContext context,
                      BREthereumBCSCallbackTransactionType type,
                      OwnershipGiven BREthereumTransaction transaction);

private_extern void
ewmHandleLog (BREthereumBCSCallbackContext context,
              BREthereumBCSCallbackLogType type,
              OwnershipGiven BREthereumLog log);

private_extern void
ewmHandleExchange (BREthereumBCSCallbackContext context,
                   BREthereumBCSCallbackExchangeType type,
                   OwnershipGiven BREthereumExchange exchange);

// MARK: - Support

private_extern BRCryptoCurrency
cryptoNetworkGetCurrencyforTokenETH (BRCryptoNetwork network,
                                     BREthereumToken token);

// MARK: - Fee Basis

typedef struct BRCryptoFeeBasisETHRecord {
    struct BRCryptoFeeBasisRecord base;
    BREthereumFeeBasis ethFeeBasis;
} *BRCryptoFeeBasisETH;

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsETH (BRCryptoUnit unit,
                           BREthereumFeeBasis feeBasis);

private_extern BREthereumFeeBasis
cryptoFeeBasisAsETH (BRCryptoFeeBasis feeBasis);

// MARK: - Support

private_extern BRCryptoHash
cryptoHashCreateAsETH (BREthereumHash hash);

private_extern BREthereumHash
cryptoHashAsETH (BRCryptoHash hash);

/// MARK: - File Service

extern const char *fileServiceTypeTransactionsETH;
extern const char *fileServiceTypeLogsETH;
extern const char *fileServiceTypeExchangesETH;
extern const char *fileServiceTypeBlocksETH;
extern const char *fileServiceTypeNodesETH;
extern const char *fileServiceTypeTokensETH;
extern const char *fileServiceTypeWalletsETH;

extern size_t fileServiceSpecificationsCountETH;
extern BRFileServiceTypeSpecification *fileServiceSpecificationsETH;

#define EWM_INITIAL_SET_SIZE_DEFAULT  (10)

extern BRSetOf(BREthereumTransaction) initialTransactionsLoadETH (BRCryptoWalletManager manager);
extern BRSetOf(BREthereumLog)         initialLogsLoadETH         (BRCryptoWalletManager manager);
extern BRSetOf(BREthereumExchange)    initialExchangesLoadETH    (BRCryptoWalletManager manager);
extern BRSetOf(BREthereumBlock)       initialBlocksLoadETH       (BRCryptoWalletManager manager);
extern BRSetOf(BREthereumNodeConfig)  initialNodesLoadETH        (BRCryptoWalletManager manager);
extern BRSetOf(BREthereumToken)       initialTokensLoadETH       (BRCryptoWalletManager manager);
#if defined(NEED_ETH_WALLET_IN_FILE_SERVICE)
extern BRSetOf(BREthereumWalletState) initialWalletsLoadETH      (BRCryptoWalletManager manager);
#endif

// MARK: - Events

extern const BREventType *eventTypesETH[];
extern const unsigned int eventTypesCountETH;

#ifdef __cplusplus
}
#endif

#endif // BRCryptoETH_h
