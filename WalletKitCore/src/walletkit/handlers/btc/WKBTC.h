//
//  WKBTC.h
//  WalletKitCore
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#ifndef WKBTC_h
#define WKBTC_h

#include "walletkit/WKHandlersP.h"

#include "bitcoin/BRBitcoinWallet.h"
#include "bitcoin/BRBitcoinTransaction.h"
#include "bitcoin/BRBitcoinChainParams.h"
#include "bitcoin/BRBitcoinPaymentProtocol.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK - Address

typedef struct WKAddressBTCRecord {
    struct WKAddressRecord base;

    /// The 'bitcoin/' address.  For BTC, addr.s is the string; for BCH, addr.s is
    /// encoded in a 'BCH' specific way.
    BRAddress addr;
} *WKAddressBTC;

extern WKAddress
wkAddressCreateAsBTC (WKNetworkType type,
                          BRAddress addr);

extern WKAddress
wkAddressCreateFromStringAsBTC (BRAddressParams params, const char *btcAddress);

extern WKAddress
wkAddressCreateFromStringAsBCH (BRAddressParams params, const char *bchAddress);

extern WKAddress
wkAddressCreateFromStringAsBSV (BRAddressParams params, const char *bsvAddress);

extern WKAddress
wkAddressCreateFromStringAsLTC (BRAddressParams params, const char *ltcAddress);

extern WKAddress
wkAddressCreateFromStringAsDOGE (BRAddressParams params, const char *dogeAddress);

private_extern BRAddress
wkAddressAsBTC (WKAddress address,
                    WKNetworkType *type);

private_extern WKAddress
wkAddressCreateFromLegacyStringAsBCH (BRAddressParams params, const char *btcAddr);

// MARK: - Network

typedef struct WKNetworkBTCRecord {
    struct WKNetworkRecord base;

    const BRBitcoinChainParams *params;
} *WKNetworkBTC;

extern const BRBitcoinChainParams *
wkNetworkAsBTC (WKNetwork network);

extern uint64_t
wkNetworkFeeAsBTC (WKNetworkFee networkFee);


// MARK: - Transfer

typedef struct WKTransferBTCRecord {
    struct WKTransferRecord base;

    // The BRBitcoinTransaction; this is 100% owned by WKTransfer.  It can be accessed at any time.
    // Prior to signing the hash will be empty.
    BRBitcoinTransaction *tid;

    // Tracking of 'deleted'
    bool isDeleted;

    uint64_t fee;
    uint64_t send;
    uint64_t recv;
} *WKTransferBTC;

extern WKTransferBTC
wkTransferCoerceBTC (WKTransfer transfer);

extern WKTransfer
wkTransferCreateAsBTC (WKTransferListener listener,
                           WKUnit unit,
                           WKUnit unitForFee,
                           BRBitcoinWallet *wid,
                           BRBitcoinTransaction *tid,
                           WKNetworkType type);

private_extern BRBitcoinTransaction *
wkTransferAsBTC (WKTransfer transfer);

private_extern WKBoolean
wkTransferHasBTC (WKTransfer transfer,
                      BRBitcoinTransaction *btc);

private_extern WKBoolean
wkTransferChangedAmountBTC (WKTransfer transfer,
                                BRBitcoinWallet *wid);

private_extern OwnershipGiven WKTransferState
wkTransferInitializeStateBTC (BRBitcoinTransaction *tid,
                                  uint64_t blockNumber,
                                  uint64_t blockTimestamp,
                                  OwnershipKept WKFeeBasis feeBasis);

// MARK: - Wallet

typedef struct WKWalletBTCRecord {
    struct WKWalletRecord base;
    BRBitcoinWallet *wid;
    BRArrayOf (BRBitcoinTransaction*) tidsUnresolved;
} *WKWalletBTC;

extern WKWalletHandlers wkWalletHandlersBTC;

private_extern WKWalletBTC
wkWalletCoerceBTC (WKWallet wallet);

private_extern BRBitcoinWallet *
wkWalletAsBTC (WKWallet wallet);

private_extern WKWallet
wkWalletCreateAsBTC (WKNetworkType type,
                         WKWalletListener listener,
                         WKUnit unit,
                         WKUnit unitForFee,
                         BRBitcoinWallet *wid);

private_extern WKTransfer
wkWalletFindTransferAsBTC (WKWallet wallet,
                               BRBitcoinTransaction *btc);

private_extern WKTransferBTC
wkWalletFindTransferByHashAsBTC (WKWallet wallet,
                                     UInt256 hash);

private_extern void
wkWalletAddUnresolvedAsBTC (WKWallet wallet,
                                OwnershipGiven BRBitcoinTransaction *tid);

private_extern void
wkWalletUpdUnresolvedAsBTC (WKWallet wallet,
                                const UInt256 *hash,
                                uint32_t blockHeight,
                                uint32_t timestamp);

private_extern size_t
wkWalletRemResolvedAsBTC (WKWallet wallet,
                              BRBitcoinTransaction **tids,
                              size_t tidsCount);

// MARK: - (Wallet) Manager

typedef struct WKWalletManagerBTCRecord {
    struct WKWalletManagerRecord base;
} *WKWalletManagerBTC;

extern WKWalletManagerBTC
wkWalletManagerCoerceBTC (WKWalletManager manager, WKNetworkType type);

extern WKWalletManagerHandlers wkWalletManagerHandlersBTC;

// MAKR: - Wallet Manger P2P

extern WKClientP2PManager
wkWalletManagerCreateP2PManagerBTC (WKWalletManager manager);

// MARK: - Wallet Sweeper

typedef struct WKWalletSweeperBTCRecord {
    struct WKWalletSweeperRecord base;

    BRAddressParams addrParams;
    uint8_t isSegwit;
    char * sourceAddress;
    BRArrayOf(BRBitcoinTransaction *) txns;
} *WKWalletSweeperBTC;

// MARK: - Payment Protocol

// MARK: Payment Protocol Bitpay Builder

typedef struct WKPaymentProtocolRequestBitPayBuilderBTCRecord {
    struct WKPaymentProtocolRequestBitPayBuilderRecord base;
    
    BRArrayOf(BRBitcoinTxOutput) outputs;
} *WKPaymentProtocolRequestBitPayBuilderBTC;

// MARK: Payment Protocol Request

typedef struct WKPaymentProtocolRequestBTCRecord {
    struct WKPaymentProtocolRequestRecord base;
    
    BRBitcoinPaymentProtocolRequest *request;
} *WKPaymentProtocolRequestBTC;

// MARK: Payment Protocol Payment

typedef struct WKPaymentProtocolPaymentBTCRecord {
    struct WKPaymentProtocolPaymentRecord base;
    
    BRBitcoinTransaction *transaction;
    BRBitcoinPaymentProtocolPayment *payment; // only used for BIP70
} *WKPaymentProtocolPaymentBTC;

// MARK: - Fee Basis

typedef struct WKFeeBasisBTCRecord {
    struct WKFeeBasisRecord base;
    uint64_t fee;
    uint64_t feePerKB;
    double sizeInKB;
} *WKFeeBasisBTC;

#define WK_FEE_BASIS_BTC_FEE_UNKNOWN               (UINT64_MAX)
#define WK_FEE_BASIS_BTC_FEE_PER_KB_UNKNOWN        (UINT64_MAX)
#define WK_FEE_BASIS_BTC_SIZE_UNKNOWN              (UINT32_MAX)

private_extern WKFeeBasis
wkFeeBasisCreateAsBTC (WKUnit unit,
                           uint64_t fee,
                           uint64_t feePerKB,
                           uint32_t sizeInByte);

private_extern uint64_t // SAT-per-KB
wkFeeBasisAsBTC (WKFeeBasis feeBasis);

// MARK: - Support

private_extern WKHash
wkHashCreateAsBTC (UInt256 btc);

/// MARK: - File Service

extern const char *fileServiceTypeTransactionsBTC;
extern const char *fileServiceTypeBlocksBTC;
extern const char *fileServiceTypePeersBTC;

extern size_t fileServiceSpecificationsCountBTC;
extern BRFileServiceTypeSpecification *fileServiceSpecificationsBTC;

extern BRArrayOf(BRBitcoinTransaction*) initialTransactionsLoadBTC (WKWalletManager manager);
extern BRArrayOf(BRBitcoinPeer)         initialPeersLoadBTC        (WKWalletManager manager);
extern BRArrayOf(BRBitcoinMerkleBlock*) initialBlocksLoadBTC       (WKWalletManager manager);

#ifdef __cplusplus
}
#endif

#endif // WKBTC_h
