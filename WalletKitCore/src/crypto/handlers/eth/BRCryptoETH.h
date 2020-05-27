#ifndef BRCryptoETH_h
#define BRCryptoETH_h

#include "../BRCryptoHandlersExport.h"

#include "ethereum/base/BREthereumBase.h"
#include "ethereum/blockchain/BREthereumAccount.h"
#include "ethereum/blockchain/BREthereumTransaction.h"
#include "ethereum/blockchain/BREthereumLog.h"

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
    TRANSFER_BASIS_LOG
} BREthereumTransferBasisType;


/// A ETH Transfer
typedef struct BRCryptoTransferETHRecord {
    struct BRCryptoTransferRecord base;
    //    BREthereumEWM ewm;
    //    BREthereumTransfer tid;
    //    BREthereumAddress sourceAddress;
    //    BREthereumAddress targetAddress;
    //    BREthereumAmount amount;
    //    BREthereumFeeBasis feeBasis;

    BREthereumAccount account;
    BREthereumGas gasEstimate;
    BREthereumTransferStatus status;

    BREthereumTransferBasisType type;
    union {
        BREthereumTransaction transaction;
        BREthereumLog log;
    } basis;

    BREthereumTransaction originatingTransaction;

    // TODO: Remove
    BRCryptoAmount amount;
    BRCryptoTransferDirection direction;
    
} *BRCryptoTransferETH;

extern BRCryptoTransferETH
cryptoTransferCoerce (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsETH (BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRCryptoFeeBasis feeBasisEstimated,
                           BRCryptoAmount amount,
                           BRCryptoTransferDirection direction,
                           BRCryptoAddress sourceAddress,
                           BRCryptoAddress targetAddress,
                           BREthereumAccount account,
                           BREthereumTransferBasisType type,
                           OwnershipGiven BREthereumTransaction originatingTransaction);

extern BRCryptoTransfer
cryptoTransferCreateWithTransactionAsETH (BRCryptoUnit unit,
                                          BRCryptoUnit unitForFee,
                                          BREthereumAccount account,
                                          OwnershipGiven BREthereumTransaction ethTransaction);

extern BRCryptoTransfer
cryptoTransferCreateWithLogAsETH (BRCryptoUnit unit,
                                  BRCryptoUnit unitForFee,
                                  BREthereumAccount account,
                                  UInt256 ethAmount,
                                  OwnershipGiven BREthereumLog ethLog);

// MARK: - Wallet

typedef struct BRCryptoWalletETHRecord {
    struct BRCryptoWalletRecord base;
    BREthereumAccount ethAccount;
    BREthereumToken   ethToken;    // NULL if `ETH`
} *BRCryptoWalletETH;

extern BRCryptoWalletETH
cryptoWalletCoerce (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsETH (BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BREthereumToken   ethToken,
                         BREthereumAccount ethAccount);

// MARK: - Wallet Manager

typedef struct BRCryptoWalletManagerETHRecord {
    struct BRCryptoWalletManagerRecord base;

    BREthereumNetwork network;
    BREthereumAccount account;
    BRRlpCoder coder;
} *BRCryptoWalletManagerETH;

// MARK: - Support

private_extern BRCryptoCurrency
cryptoNetworkGetCurrencyforTokenETH (BRCryptoNetwork network,
                                     BREthereumToken token);

// MARK: - Support



private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsETH (BRCryptoUnit unit,
                           BREthereumFeeBasis feeBasis);


private_extern BREthereumFeeBasis
cryptoFeeBasisAsETH (BRCryptoFeeBasis feeBasis);

private_extern BRCryptoHash
cryptoHashCreateAsETH (BREthereumHash hash);

/// MARK: - File Service

extern const char *fileServiceTypeTransactionsETH;
extern const char *fileServiceTypeLogsETH;
extern const char *fileServiceTypeBlocksETH;
extern const char *fileServiceTypeNodesETH;
extern const char *fileServiceTypeTokensETH;
extern const char *fileServiceTypeWalletsETH;

extern size_t fileServiceSpecificationsCountETH;
extern BRFileServiceTypeSpecification *fileServiceSpecificationsETH;

// MARK: - Events

extern const BREventType *eventTypesETH[];
extern const unsigned int eventTypesCountETH;

#ifdef __cplusplus
}
#endif

#endif // BRCryptoETH_h
