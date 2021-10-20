//
//  WKETH.h
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKETH_h
#define WKETH_h

#include "walletkit/WKHandlersP.h"

#include "ethereum/base/BREthereumBase.h"
#include "ethereum/blockchain/BREthereumAccount.h"
#include "ethereum/blockchain/BREthereumTransaction.h"
#include "ethereum/blockchain/BREthereumLog.h"
#include "ethereum/contract/BREthereumExchange.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Address

typedef struct WKAddressETHRecord {
    struct WKAddressRecord base;
    BREthereumAddress eth;
} *WKAddressETH;

private_extern WKAddress
wkAddressCreateFromStringAsETH (const char *address);

private_extern WKAddress
wkAddressCreateAsETH (BREthereumAddress eth);

private_extern BREthereumAddress
wkAddressAsETH (WKAddress address);

// MARK: - Network

typedef struct WKNetworkETHRecord {
    struct WKNetworkRecord base;
    BREthereumNetwork eth;
} *WKNetworkETH;

private_extern BREthereumNetwork
wkNetworkAsETH (WKNetwork network);

private_extern BREthereumGasPrice
wkNetworkFeeAsETH (WKNetworkFee fee);

// MARK: - Transfer

/// A ETH Transfer
typedef struct WKTransferETHRecord {
    struct WKTransferRecord base;

    WKHash hash;
    BREthereumAccount account;
    BREthereumGas gasEstimate;

    // Normally the nonce is held in the originatingTransaction; but we don't always have one.
    // So, we'll keep a nonce here, recovered from the Client/Blockset.  Work hard to keep it in
    // sync with an originatingTransaction, when we have one.
    uint64_t nonce;

    // The tranaction that originated this transfer.  Will be NULL if the transaction's source
    // address is not in `account`.  May be NULL if the transaction has not been seen yet.
    BREthereumTransaction originatingTransaction;
} *WKTransferETH;

extern WKTransferETH
wkTransferCoerceETH (WKTransfer transfer);

extern WKTransfer
wkTransferCreateAsETH (WKTransferListener listener,
                           const char *uids,
                           WKHash hash,
                           WKUnit unit,
                           WKUnit unitForFee,
                           WKFeeBasis feeBasisEstimated,
                           WKAmount amount,
                           WKAddress sourceAddress,
                           WKAddress targetAddress,
                           WKTransferState transferState,
                           BREthereumAccount account,
                           uint64_t nonce,
                           OwnershipGiven BREthereumTransaction originatingTransaction);

extern const BREthereumHash
wkTransferGetIdentifierETH (WKTransferETH transfer);

extern const BREthereumHash
wkTransferGetOriginatingTransactionHashETH (WKTransferETH transfer);

extern WKTransferState
wkTransferDeriveStateETH (BREthereumTransactionStatus status,
                              WKFeeBasis feeBasis);

extern uint64_t
wkTransferGetNonceETH (WKTransferETH transfer);

extern void
wkTransferSetNonceETH (WKTransferETH transfer,
                           uint64_t nonce);

// MARK: - Wallet

typedef struct WKWalletETHRecord {
    struct WKWalletRecord base;
    
    BREthereumAccount ethAccount;
    BREthereumToken   ethToken;    // NULL if `ETH`
    BREthereumGas     ethGasLimit;
} *WKWalletETH;

extern WKWalletETH
wkWalletCoerce (WKWallet wallet);

private_extern WKWallet
wkWalletCreateAsETH (WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BREthereumToken   ethToken,
                         BREthereumAccount ethAccount);

extern WKTransfer
wkWalletCreateTransferETH (WKWallet  wallet,
                               WKAddress target,
                               WKAmount  amount,
                               WKFeeBasis estimatedFeeBasis,
                               size_t attributesCount,
                               OwnershipKept WKTransferAttribute *attributes,
                               WKCurrency currency,
                               WKUnit unit,
                               WKUnit unitForFee);

extern WKTransferETH
wkWalletLookupTransferByIdentifier (WKWalletETH wallet,
                                        BREthereumHash identifier);
extern WKTransferETH
wkWalletLookupTransferByOriginatingHash (WKWalletETH wallet,
                                             BREthereumHash hash);

// MARK: - Wallet Manager

typedef struct WKWalletManagerETHRecord {
    struct WKWalletManagerRecord base;

    BREthereumAccount account;
    BRSetOf(BREthereumToken) tokens;

    BRRlpCoder coder;
} *WKWalletManagerETH;

extern WKWalletManagerETH
wkWalletManagerCoerceETH (WKWalletManager manager);

static inline BREthereumNetwork
wkWalletManagerGetNetworkAsETH (WKWalletManager manager) {
    return wkNetworkAsETH (manager->network);
}

private_extern WKWalletETH
wkWalletManagerEnsureWalletForToken (WKWalletManagerETH managerETH,
                                         BREthereumToken token);

// MARK: - Support

private_extern WKCurrency
wkNetworkGetCurrencyforTokenETH (WKNetwork network,
                                     BREthereumToken token);

// MARK: - Fee Basis

typedef struct WKFeeBasisETHRecord {
    struct WKFeeBasisRecord base;
    BREthereumFeeBasis ethFeeBasis;
} *WKFeeBasisETH;

private_extern WKFeeBasis
wkFeeBasisCreateAsETH (WKUnit unit,
                           BREthereumFeeBasis feeBasis);

private_extern BREthereumFeeBasis
wkFeeBasisAsETH (WKFeeBasis feeBasis);

// MARK: - Support

private_extern WKHash
wkHashCreateAsETH (BREthereumHash hash);

private_extern BREthereumHash
wkHashAsETH (WKHash hash);

// MARK: - Events

extern const BREventType *eventTypesETH[];
extern const unsigned int eventTypesCountETH;

// MARK: - Misc

#define EWM_INITIAL_SET_SIZE_DEFAULT  (10)

#ifdef __cplusplus
}
#endif

#endif // WKETH_h
