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

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct BRCryptoAddressETHRecord {
    struct BRCryptoAddressRecord base;
    BREthereumAddress eth;
} *BRCryptoAddressETH;

private_extern BRCryptoAddress
cryptoAddressCreateFromStringAsETH (const char *address);

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

/// A ETH Transfer
typedef struct BRCryptoTransferETHRecord {
    struct BRCryptoTransferRecord base;

    BRCryptoHash hash;
    BREthereumAccount account;
    BREthereumGas gasEstimate;

    // Normally the nonce is held in the originatingTransaction; but we don't always have one.
    // So, we'll keep a nonce here, recovered from the Client/Blockset.  Work hard to keep it in
    // sync with an originatingTransaction, when we have one.
    uint64_t nonce;

    // The tranaction that originated this transfer.  Will be NULL if the transaction's source
    // address is not in `account`.  May be NULL if the transaction has not been seen yet.
    BREthereumTransaction originatingTransaction;
} *BRCryptoTransferETH;

extern BRCryptoTransferETH
cryptoTransferCoerceETH (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsETH (BRCryptoTransferListener listener,
                           const char *uids,
                           BRCryptoHash hash,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRCryptoFeeBasis feeBasisEstimated,
                           BRCryptoAmount amount,
                           BRCryptoAddress sourceAddress,
                           BRCryptoAddress targetAddress,
                           BRCryptoTransferState transferState,
                           BREthereumAccount account,
                           uint64_t nonce,
                           OwnershipGiven BREthereumTransaction originatingTransaction);

extern const BREthereumHash
cryptoTransferGetIdentifierETH (BRCryptoTransferETH transfer);

extern const BREthereumHash
cryptoTransferGetOriginatingTransactionHashETH (BRCryptoTransferETH transfer);

extern BRCryptoTransferState
cryptoTransferDeriveStateETH (BREthereumTransactionStatus status,
                              BRCryptoFeeBasis feeBasis);

extern uint64_t
cryptoTransferGetNonceETH (BRCryptoTransferETH transfer);

extern void
cryptoTransferSetNonceETH (BRCryptoTransferETH transfer,
                           uint64_t nonce);

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

// MARK: - Events

extern const BREventType *eventTypesETH[];
extern const unsigned int eventTypesCountETH;

// MARK: - Misc

#define EWM_INITIAL_SET_SIZE_DEFAULT  (10)

#ifdef __cplusplus
}
#endif

#endif // BRCryptoETH_h
