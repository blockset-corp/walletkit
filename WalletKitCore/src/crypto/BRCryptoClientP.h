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

#include <pthread.h>

#include "support/BRArray.h"
#include "support/BRSet.h"
#include "support/rlp/BRRlp.h"
#include "support/event/BREvent.h"

#include "BRCryptoFileService.h"
#include "BRCryptoClient.h"
#include "BRCryptoSync.h"
#include "BRCryptoTransfer.h"
#include "BRCryptoWalletManager.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Transaction Bundle

struct BRCryptoClientTransactionBundleRecord {
    BRCryptoTransferStateType status;
    uint8_t *serialization;
    size_t   serializationCount;
    BRCryptoTimestamp timestamp;
    BRCryptoBlockNumber blockHeight;
};

private_extern OwnershipKept uint8_t *
cryptoClientTransactionBundleGetSerialization (BRCryptoClientTransactionBundle bundle,
                                               size_t *serializationCount);

private_extern BRRlpItem
cryptoClientTransactionBundleRlpEncode (BRCryptoClientTransactionBundle bundle,
                                        BRRlpCoder coder);

private_extern BRCryptoClientTransactionBundle
cryptoClientTransactionBundleRlpDecode (BRRlpItem item,
                                        BRRlpCoder coder);

// For BRSet
private_extern size_t
cryptoClientTransactionBundleGetHashValue (BRCryptoClientTransactionBundle bundle);

// For BRSet
private_extern int
cryptoClientTransactionBundleIsEqual (BRCryptoClientTransactionBundle bundle1,
                                      BRCryptoClientTransactionBundle bundle2);

static inline BRSetOf(BRCryptoClientTransactionBundle)
cryptoClientTransactionBundleSetCreate (size_t capacity) {
    return BRSetNew ((size_t (*) (const void *)) cryptoClientTransactionBundleGetHashValue,
                     (int (*) (const void *, const void *)) cryptoClientTransactionBundleIsEqual,
                    capacity);
}

static inline void
cryptoClientTransactionBundleSetRelease (BRSetOf(BRCryptoClientTransactionBundle) bundles) {
    BRSetFreeAll(bundles, (void (*) (void *))  cryptoClientTransactionBundleRelease);
}


// MARK: - Transfer Bundle

struct BRCryptoClientTransferBundleRecord {
    BRCryptoTransferStateType status;
    char * /* transaction */ hash;
    char * /* transaction */ identifier;
    char * /* transfer */ uids;
    char *from;
    char *to;
    char *amount;
    char *currency;
    char *fee;
    uint64_t transferIndex;
    BRCryptoTimestamp blockTimestamp;
    BRCryptoBlockNumber blockNumber;
    BRCryptoBlockNumber blockConfirmations;
    uint64_t blockTransactionIndex;
    char *blockHash;

    size_t attributesCount;
    char **attributeKeys;
    char **attributeVals;
};

private_extern BRRlpItem
cryptoClientTransferBundleRlpEncode (BRCryptoClientTransferBundle bundle,
                                     BRRlpCoder coder);

private_extern BRCryptoClientTransferBundle
cryptoClientTransferBundleRlpDecode (BRRlpItem item,
                                     BRRlpCoder coder,
                                     BRCryptoFileServiceTransferVersion version);

// For BRSet
private_extern size_t
cryptoClientTransferBundleGetHashValue (BRCryptoClientTransferBundle bundle);

// For BRSet
private_extern int
cryptoClientTransferBundleIsEqual (BRCryptoClientTransferBundle bundle1,
                                   BRCryptoClientTransferBundle bundle2);

static inline BRSetOf(BRCryptoClientTransferBundle)
cryptoClientTransferBundleSetCreate (size_t capacity) {
    return BRSetNew ((size_t (*) (const void *)) cryptoClientTransferBundleGetHashValue,
                     (int (*) (const void *, const void *)) cryptoClientTransferBundleIsEqual,
                     capacity);
}

static inline void
cryptoClientTransferBundleSetRelease (BRSetOf(BRCryptoClientTransferBundle) bundles) {
    BRSetFreeAll(bundles, (void (*) (void *)) cryptoClientTransferBundleRelease);
}

// MARK: - Currency / Currency Denomination Bundle

struct BRCryptoCliehtCurrencyDenominationBundleRecord {
    char *name;
    char *code;
    char *symbol;
    uint8_t decimals;
};

private_extern BRRlpItem
cryptoClientCurrencyDenominationBundleRlpEncode (BRCryptoClientCurrencyDenominationBundle bundle,
                                                 BRRlpCoder coder);


private_extern BRCryptoClientCurrencyDenominationBundle
cryptoClientCurrencyDenominationBundleRlpDecode (BRRlpItem item,
                                                 BRRlpCoder coder);


struct BRCryptoClientCurrencyBundleRecord {
    char *id;
    char *name;
    char *code;
    char *type;
    char *bid;
    char *address;
    bool  verfified;
    BRArrayOf(BRCryptoClientCurrencyDenominationBundle) denominations;
};

private_extern BRRlpItem
cryptoClientCurrencyBundleRlpEncode (BRCryptoClientCurrencyBundle bundle,
                                     BRRlpCoder coder);

private_extern BRCryptoClientCurrencyBundle
cryptoClientCurrencyBundleRlpDecode (BRRlpItem item,
                                     BRRlpCoder coder);

extern OwnershipGiven BRSetOf(BRCryptoClientCurrencyBundle)
cryptoClientCurrencyBundleSetCreate (size_t size);

extern void
cryptoClientCurrencyBundleSetRelease (OwnershipGiven BRSetOf(BRCryptoClientCurrencyBundle) bundles);

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

typedef void
(*BRCryptoClientP2PManagerSetNetworkReachableHandler) (BRCryptoClientP2PManager p2p,
                                                       int isNetworkReachable);

typedef struct {
    BRCryptoClientP2PManagerReleaseHandler release;
    BRCryptoClientP2PManagerConnectHandler connect;
    BRCryptoClientP2PManagerDisconnectHandler disconnect;
    BRCryptoClientP2PManagerSyncHandler sync;
    BRCryptoClientP2PManagerSendHandler send;
    BRCryptoClientP2PManagerSetNetworkReachableHandler setNetworkReachable;
} BRCryptoClientP2PHandlers;

struct BRCryptoClientP2PManagerRecord {
    BRCryptoBlockChainType type;
    const BRCryptoClientP2PHandlers *handlers;
    size_t sizeInBytes;

    // State

    pthread_mutex_t lock;
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

extern void
cryptoClientP2PManagerSetNetworkReachable (BRCryptoClientP2PManager p2p,
                                           BRCryptoBoolean isNetworkReachable);

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

    bool connected;
    size_t requestId;

    pthread_mutex_t lock;
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

extern BREventType handleClientAnnounceBlockNumberEventType;
extern BREventType handleClientAnnounceTransactionsEventType;
extern BREventType handleClientAnnounceTransfersEventType;
extern BREventType handleClientAnnounceSubmitEventType;
extern BREventType handleClientAnnounceEstimateTransactionFeeEventType;

#define CRYPTO_CLIENT_EVENT_TYPES             \
  &handleClientAnnounceBlockNumberEventType,  \
  &handleClientAnnounceTransactionsEventType, \
  &handleClientAnnounceTransfersEventType,    \
  &handleClientAnnounceSubmitEventType,       \
  &handleClientAnnounceEstimateTransactionFeeEventType

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoClientP_h */
