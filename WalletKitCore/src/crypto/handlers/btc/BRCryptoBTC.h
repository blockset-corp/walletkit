//
//  BRCryptoBTC.h
//  Core
//
//  Created by Ed Gamble on 05/07/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#ifndef BRCryptoBTC_h
#define BRCryptoBTC_h

#include "../BRCryptoHandlersExport.h"

#include "crypto/BRCryptoWalletSweeperP.h"
#include "crypto/BRCryptoPaymentP.h"
#include "crypto/BRCryptoFeeBasisP.h"

#include "bitcoin/BRWallet.h"
#include "bitcoin/BRTransaction.h"
#include "bitcoin/BRChainParams.h"
#include "bitcoin/BRPaymentProtocol.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK - Address

typedef struct BRCryptoAddressBTCRecord {
    struct BRCryptoAddressRecord base;

    /// The 'bitcoin/' address.  For BTC, addr.s is the string; for BCH, addr.s is
    /// encoded in a 'BCH' specific way.
    BRAddress addr;
} *BRCryptoAddressBTC;

extern BRCryptoAddress
cryptoAddressCreateAsBTC (BRCryptoBlockChainType type,
                          BRAddress addr);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBTC (BRAddressParams params, const char *btcAddress);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBCH (BRAddressParams params, const char *bchAddress);

extern BRCryptoAddress
cryptoAddressCreateFromStringAsBSV (BRAddressParams params, const char *bsvAddress);

private_extern BRAddress
cryptoAddressAsBTC (BRCryptoAddress address,
                    BRCryptoBlockChainType *type);

private_extern BRCryptoAddress
cryptoAddressCreateFromLegacyStringAsBCH (BRAddressParams params, const char *btcAddr);

// MARK: - Network

typedef struct BRCryptoNetworkBTCRecord {
    struct BRCryptoNetworkRecord base;

    const BRChainParams *params;
} *BRCryptoNetworkBTC;

extern const BRChainParams *
cryptoNetworkAsBTC (BRCryptoNetwork network);

extern uint64_t
cryptoNetworkFeeAsBTC (BRCryptoNetworkFee networkFee);


// MARK: - Transfer

typedef struct BRCryptoTransferBTCRecord {
    struct BRCryptoTransferRecord base;

    // The BRTransaction; this is 100% owned by BRCryptoTransfer.  It can be accessed at any time.
    // Prior to signing the hash will be empty.
    BRTransaction *tid;

    // Tracking of 'deleted'
    bool isDeleted;

    uint64_t fee;
    uint64_t send;
    uint64_t recv;
} *BRCryptoTransferBTC;

extern BRCryptoTransferBTC
cryptoTransferCoerceBTC (BRCryptoTransfer transfer);

extern BRCryptoTransfer
cryptoTransferCreateAsBTC (BRCryptoTransferListener listener,
                           BRCryptoUnit unit,
                           BRCryptoUnit unitForFee,
                           BRWallet *wid,
                           BRTransaction *tid,
                           BRCryptoBlockChainType type);

private_extern BRTransaction *
cryptoTransferAsBTC (BRCryptoTransfer transfer);

private_extern BRCryptoBoolean
cryptoTransferHasBTC (BRCryptoTransfer transfer,
                      BRTransaction *btc);

private_extern BRCryptoBoolean
cryptoTransferChangedAmountBTC (BRCryptoTransfer transfer,
                                BRWallet *wid);

private_extern OwnershipGiven BRCryptoTransferState
cryptoTransferInitializeStateBTC (BRTransaction *tid,
                                  uint64_t blockNumber,
                                  uint64_t blockTimestamp,
                                  OwnershipKept BRCryptoFeeBasis feeBasis);

// MARK: - Wallet

typedef struct BRCryptoWalletBTCRecord {
    struct BRCryptoWalletRecord base;
    BRWallet *wid;
    BRArrayOf (BRTransaction*) tidsUnresolved;
} *BRCryptoWalletBTC;

extern BRCryptoWalletHandlers cryptoWalletHandlersBTC;

private_extern BRCryptoWalletBTC
cryptoWalletCoerceBTC (BRCryptoWallet wallet);

private_extern BRWallet *
cryptoWalletAsBTC (BRCryptoWallet wallet);

private_extern BRCryptoWallet
cryptoWalletCreateAsBTC (BRCryptoBlockChainType type,
                         BRCryptoWalletListener listener,
                         BRCryptoUnit unit,
                         BRCryptoUnit unitForFee,
                         BRWallet *wid);

private_extern BRCryptoTransfer
cryptoWalletFindTransferAsBTC (BRCryptoWallet wallet,
                               BRTransaction *btc);

private_extern BRCryptoTransferBTC
cryptoWalletFindTransferByHashAsBTC (BRCryptoWallet wallet,
                                     UInt256 hash);

private_extern void
cryptoWalletAddUnresolvedAsBTC (BRCryptoWallet wallet,
                                OwnershipGiven BRTransaction *tid);

private_extern void
cryptoWalletUpdUnresolvedAsBTC (BRCryptoWallet wallet,
                                const UInt256 *hash,
                                uint32_t blockHeight,
                                uint32_t timestamp);

private_extern size_t
cryptoWalletRemResolvedAsBTC (BRCryptoWallet wallet,
                              BRTransaction **tids,
                              size_t tidsCount);

// MARK: - (Wallet) Manager

typedef struct BRCryptoWalletManagerBTCRecord {
    struct BRCryptoWalletManagerRecord base;
} *BRCryptoWalletManagerBTC;

extern BRCryptoWalletManagerBTC
cryptoWalletManagerCoerceBTC (BRCryptoWalletManager manager, BRCryptoBlockChainType type);

extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC;

// MAKR: - Wallet Manger P2P

extern BRCryptoClientP2PManager
cryptoWalletManagerCreateP2PManagerBTC (BRCryptoWalletManager manager);

// MARK: - Wallet Sweeper

typedef struct BRCryptoWalletSweeperBTCRecord {
    struct BRCryptoWalletSweeperRecord base;

    BRAddressParams addrParams;
    uint8_t isSegwit;
    char * sourceAddress;
    BRArrayOf(BRTransaction *) txns;
} *BRCryptoWalletSweeperBTC;

// MARK: - Payment Protocol

// MARK: Payment Protocol Bitpay Builder

typedef struct BRCryptoPaymentProtocolRequestBitPayBuilderBTCRecord {
    struct BRCryptoPaymentProtocolRequestBitPayBuilderRecord base;
    
    BRArrayOf(BRTxOutput) outputs;
} *BRCryptoPaymentProtocolRequestBitPayBuilderBTC;

// MARK: Payment Protocol Request

typedef struct BRCryptoPaymentProtocolRequestBTCRecord {
    struct BRCryptoPaymentProtocolRequestRecord base;
    
    BRPaymentProtocolRequest *request;
} *BRCryptoPaymentProtocolRequestBTC;

// MARK: Payment Protocol Payment

typedef struct BRCryptoPaymentProtocolPaymentBTCRecord {
    struct BRCryptoPaymentProtocolPaymentRecord base;
    
    BRTransaction *transaction;
    BRPaymentProtocolPayment *payment; // only used for BIP70
} *BRCryptoPaymentProtocolPaymentBTC;

// MARK: - Fee Basis

typedef struct BRCryptoFeeBasisBTCRecord {
    struct BRCryptoFeeBasisRecord base;
    uint64_t fee;
    uint64_t feePerKB;
    double sizeInKB;
} *BRCryptoFeeBasisBTC;

#define CRYPTO_FEE_BASIS_BTC_FEE_UNKNOWN               (UINT64_MAX)
#define CRYPTO_FEE_BASIS_BTC_FEE_PER_KB_UNKNOWN        (UINT64_MAX)
#define CRYPTO_FEE_BASIS_BTC_SIZE_UNKNOWN              (UINT32_MAX)

private_extern BRCryptoFeeBasis
cryptoFeeBasisCreateAsBTC (BRCryptoUnit unit,
                           uint64_t fee,
                           uint64_t feePerKB,
                           uint32_t sizeInByte);

private_extern uint64_t // SAT-per-KB
cryptoFeeBasisAsBTC (BRCryptoFeeBasis feeBasis);

// MARK: - Support

private_extern BRCryptoHash
cryptoHashCreateAsBTC (UInt256 btc);

/// MARK: - File Service

extern const char *fileServiceTypeTransactionsBTC;
extern const char *fileServiceTypeBlocksBTC;
extern const char *fileServiceTypePeersBTC;

extern size_t fileServiceSpecificationsCountBTC;
extern BRFileServiceTypeSpecification *fileServiceSpecificationsBTC;

extern BRArrayOf(BRTransaction*) initialTransactionsLoadBTC (BRCryptoWalletManager manager);
extern BRArrayOf(BRPeer)         initialPeersLoadBTC        (BRCryptoWalletManager manager);
extern BRArrayOf(BRMerkleBlock*) initialBlocksLoadBTC       (BRCryptoWalletManager manager);

#ifdef __cplusplus
}
#endif

#endif // BRCryptoBTC_h
