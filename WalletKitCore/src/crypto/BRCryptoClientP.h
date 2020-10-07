//
//  BRCryptoClientP.h
//  BRCore
//
//  Created by Ed Gamble on 04/28/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoClientP_h
#define BRCryptoClientP_h

#include "support/BRSet.h"

#include "BRCryptoClient.h"
#include "BRCryptoSync.h"
#include "BRCryptoTransfer.h"
#include "BRCryptoWalletManager.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Transaction/Transfer Bundle

struct BRCryptoClientTransactionBundleRecord {
    BRCryptoTransferStateType status;
    uint8_t *serialization;
    size_t   serializationCount;
    BRCryptoTimestamp timestamp;
    BRCryptoBlockNumber blockHeight;
};

struct BRCryptoClientTransferBundleRecord {
    BRCryptoTransferStateType status;
    char *hash;
    char *uids;
    char *from;
    char *to;
    char *amount;
    char *currency;
    char *fee;
    uint64_t blockTimestamp;
    uint64_t blockNumber;
    uint64_t blockConfirmations;
    uint64_t blockTransactionIndex;
    char *blockHash;

    size_t attributesCount;
    char **attributeKeys;
    char **attributeVals;
};

// MARK: - Client Callback

typedef enum  {
    CLIENT_CALLBACK_REQUEST_BLOCK_NUMBER,
    CLIENT_CALLBACK_REQUEST_TRANSFERS,
    CLIENT_CALLBACK_REQUEST_TRANSACTIONS,
    CLIENT_CALLBACK_SUBMIT_TRANSACTION,
    CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE,
} BRCryptoClientCallbackType;

struct BRCryptoClientCallbackStateRecord {
    BRCryptoClientCallbackType type;
    union {
        struct {
            BRSetOf(BRCryptoAddress) addresses;
        } getTransfers;

        struct {
            BRSetOf(BRCryptoAddress) addresses;
        } getTransactions;

        struct {
            BRCryptoWallet wallet;
            BRCryptoTransfer transfer;
            BRCryptoHash hash;
        } submitTransaction;

        struct {
            BRCryptoHash hash;
            BRCryptoCookie cookie;
            BRCryptoNetworkFee networkFee;
            BRCryptoFeeBasis initialFeeBasis;
        } estimateTransactionFee;
        // ...
    } u;
    size_t rid;
};

// MARK: - P2P/QRY Type

typedef struct BRCryptoClientP2PManagerRecord *BRCryptoClientP2PManager;
typedef struct BRCryptoClientQRYManagerRecord *BRCryptoClientQRYManager;

typedef enum {
    CRYPTO_CLIENT_P2P_MANAGER_TYPE,
    CRYPTO_CLIENT_QRY_MANAGER_TYPE
} BRCryptoClientManagerType;

// MARK: - Client Sync

typedef struct {
    BRCryptoClientManagerType type;
    union {
        BRCryptoClientP2PManager p2pManager;
        BRCryptoClientQRYManager qryManager;
    } u;
} BRCryptoClientSync;

extern void
cryptoClientSync (BRCryptoClientSync sync,
                  BRCryptoSyncDepth depth,
                  BRCryptoBlockNumber height);

extern void
cryptoClientSyncPeriodic (BRCryptoClientSync sync);

// MARK: Client Send

typedef struct {
    BRCryptoClientManagerType type;
    union {
        BRCryptoClientP2PManager p2pManager;
        BRCryptoClientQRYManager qryManager;
    } u;
} BRCryptoClientSend;

extern void
cryptoClientSend (BRCryptoClientSend send,
                  BRCryptoWallet wallet,
                  BRCryptoTransfer transfer);

// MARK: Client P2P (Peer-to-Peer)

typedef void
(*BRCryptoClientP2PManagerReleaseHandler) (BRCryptoClientP2PManager p2p);

typedef void
(*BRCryptoClientP2PManagerConnectHandler) (BRCryptoClientP2PManager p2p,
                                           BRCryptoPeer peer);

typedef void
(*BRCryptoClientP2PManagerDisconnectHandler) (BRCryptoClientP2PManager p2p);

typedef void
(*BRCryptoClientP2PManagerSyncHandler) (BRCryptoClientP2PManager p2p,
                                        BRCryptoSyncDepth depth,
                                        BRCryptoBlockNumber height);

typedef void
(*BRCryptoClientP2PManagerSendHandler) (BRCryptoClientP2PManager p2p,
                                        BRCryptoWallet   wallet,
                                        BRCryptoTransfer transfer);

typedef struct {
    BRCryptoClientP2PManagerReleaseHandler release;
    BRCryptoClientP2PManagerConnectHandler connect;
    BRCryptoClientP2PManagerDisconnectHandler disconnect;
    BRCryptoClientP2PManagerSyncHandler sync;
    BRCryptoClientP2PManagerSendHandler send;
} BRCryptoClientP2PHandlers;

struct BRCryptoClientP2PManagerRecord {
    BRCryptoBlockChainType type;
    const BRCryptoClientP2PHandlers *handlers;
    size_t sizeInBytes;
};

extern BRCryptoClientP2PManager
cryptoClientP2PManagerCreate (size_t sizeInBytes,
                              BRCryptoBlockChainType type,
                              const BRCryptoClientP2PHandlers *handlers);

extern void
cryptoClientP2PManagerRelease (BRCryptoClientP2PManager p2p);

extern void
cryptoClientP2PManagerConnect (BRCryptoClientP2PManager p2p,
                               BRCryptoPeer peer);

extern void
cryptoClientP2PManagerDisconnect (BRCryptoClientP2PManager p2p);

static inline BRCryptoClientSync
cryptoClientP2PManagerAsSync (BRCryptoClientP2PManager p2p) {
    return (BRCryptoClientSync) {
        CRYPTO_CLIENT_P2P_MANAGER_TYPE,
        { .p2pManager = p2p }
    };
}

static inline BRCryptoClientSend
cryptoClientP2PManagerAsSend (BRCryptoClientP2PManager p2p) {
    return (BRCryptoClientSend) {
        CRYPTO_CLIENT_P2P_MANAGER_TYPE,
        { .p2pManager = p2p }
    };
}

// MARK: Client QRY (QueRY)

typedef enum {
    CRYPTO_CLIENT_REQUEST_USE_TRANSFERS,
    CRYPTO_CLIENT_REQUEST_USE_TRANSACTIONS,
} BRCryptoClientQRYByType;

struct BRCryptoClientQRYManagerRecord {
    BRCryptoClient client;
    BRCryptoWalletManager manager;
    BRCryptoClientQRYByType byType;
    BRCryptoBlockNumber blockNumberOffset;

    struct {
        bool completed;
        bool success;
        bool unbounded;     // true if `endBlockNumber` should be unbounded on request
        BRCryptoBlockNumber begBlockNumber;
        BRCryptoBlockNumber endBlockNumber;
        size_t rid;
    } sync;

    size_t requestId;
};

#define CRYPTO_CLIENT_QRY_IS_UNBOUNDED            (true)

extern BRCryptoClientQRYManager
cryptoClientQRYManagerCreate (BRCryptoClient client,
                              BRCryptoWalletManager manager,
                              BRCryptoClientQRYByType byType,
                              BRCryptoBlockNumber earliestBlockNumber,
                              BRCryptoBlockNumber currentBlockNumber);

extern void
cryptoClientQRYManagerRelease (BRCryptoClientQRYManager qry);

extern void
cryptoClientQRYManagerConnect (BRCryptoClientQRYManager qry);

extern void
cryptoClientQRYManagerDisconnect (BRCryptoClientQRYManager qry);

extern void
cryptoClientQRYManagerTickTock (BRCryptoClientQRYManager qry);

extern void
cryptoClientQRYEstimateTransferFee (BRCryptoClientQRYManager qry,
                                    BRCryptoCookie   cookie,
                                    BRCryptoTransfer transfer,
                                    BRCryptoNetworkFee networkFee,
                                    BRCryptoFeeBasis initialFeeBasis);

static inline BRCryptoClientSync
cryptoClientQRYManagerAsSync (BRCryptoClientQRYManager qry) {
    return (BRCryptoClientSync) {
        CRYPTO_CLIENT_QRY_MANAGER_TYPE,
        { .qryManager = qry }
    };
}

static inline BRCryptoClientSend
cryptoClientQRYManagerAsSend (BRCryptoClientQRYManager qry) {
    return (BRCryptoClientSend) {
        CRYPTO_CLIENT_QRY_MANAGER_TYPE,
        { .qryManager = qry }
    };
}

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoClientP_h */
