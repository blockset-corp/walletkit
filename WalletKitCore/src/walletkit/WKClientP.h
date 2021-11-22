//
//  WKClientP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 04/28/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKClientP_h
#define WKClientP_h

#include <pthread.h>

#include "support/BRArray.h"
#include "support/BRSet.h"
#include "support/rlp/BRRlp.h"
#include "support/event/BREvent.h"

#include "WKFileService.h"
#include "WKClient.h"
#include "WKSync.h"
#include "WKTransfer.h"
#include "WKWalletManager.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Error

struct WKClientErrorRecord {
    WKClientErrorType type;
    char *details;
    union {
        WKTransferSubmitErrorType submitErrorType;
    } u;
};

private_extern void
wkClientErrorRelease (WKClientError error);

// MARK: - Transaction Bundle

struct WKClientTransactionBundleRecord {
    WKTransferStateType status;
    uint8_t *serialization;
    size_t   serializationCount;
    WKTimestamp timestamp;
    WKBlockNumber blockHeight;
};

private_extern OwnershipKept uint8_t *
wkClientTransactionBundleGetSerialization (WKClientTransactionBundle bundle,
                                               size_t *serializationCount);

private_extern BRRlpItem
wkClientTransactionBundleRlpEncode (WKClientTransactionBundle bundle,
                                        BRRlpCoder coder);

private_extern WKClientTransactionBundle
wkClientTransactionBundleRlpDecode (BRRlpItem item,
                                        BRRlpCoder coder);

// For BRSet
private_extern size_t
wkClientTransactionBundleGetHashValue (WKClientTransactionBundle bundle);

// For BRSet
private_extern int
wkClientTransactionBundleIsEqual (WKClientTransactionBundle bundle1,
                                      WKClientTransactionBundle bundle2);

static inline BRSetOf(WKClientTransactionBundle)
wkClientTransactionBundleSetCreate (size_t capacity) {
    return BRSetNew ((size_t (*) (const void *)) wkClientTransactionBundleGetHashValue,
                     (int (*) (const void *, const void *)) wkClientTransactionBundleIsEqual,
                    capacity);
}

static inline void
wkClientTransactionBundleSetRelease (BRSetOf(WKClientTransactionBundle) bundles) {
    BRSetFreeAll(bundles, (void (*) (void *))  wkClientTransactionBundleRelease);
}


// MARK: - Transfer Bundle

struct WKClientTransferBundleRecord {
    WKTransferStateType status;
    char *hash;
    char *identifier;
    char *uids;
    char *from;
    char *to;
    char *amount;
    char *currency;
    char *fee;
    uint64_t transferIndex;
    WKTimestamp blockTimestamp;
    WKBlockNumber blockNumber;
    WKBlockNumber blockConfirmations;
    uint64_t blockTransactionIndex;
    char *blockHash;

    size_t attributesCount;
    char **attributeKeys;
    char **attributeVals;
};

private_extern BRRlpItem
wkClientTransferBundleRlpEncode (WKClientTransferBundle bundle,
                                     BRRlpCoder coder);

private_extern WKClientTransferBundle
wkClientTransferBundleRlpDecode (BRRlpItem item,
                                 BRRlpCoder coder,
                                 WKFileServiceTransferVersion version);

// For BRSet
private_extern size_t
wkClientTransferBundleGetHashValue (WKClientTransferBundle bundle);

// For BRSet
private_extern int
wkClientTransferBundleIsEqual (WKClientTransferBundle bundle1,
                                   WKClientTransferBundle bundle2);

static inline BRSetOf(WKClientTransferBundle)
wkClientTransferBundleSetCreate (size_t capacity) {
    return BRSetNew ((size_t (*) (const void *)) wkClientTransferBundleGetHashValue,
                     (int (*) (const void *, const void *)) wkClientTransferBundleIsEqual,
                     capacity);
}

static inline void
wkClientTransferBundleSetRelease (BRSetOf(WKClientTransferBundle) bundles) {
    BRSetFreeAll(bundles, (void (*) (void *)) wkClientTransferBundleRelease);
}

// MARK: - Currency / Currency Denomination Bundle

struct WKCliehtCurrencyDenominationBundleRecord {
    char *name;
    char *code;
    char *symbol;
    uint8_t decimals;
};

private_extern BRRlpItem
wkClientCurrencyDenominationBundleRlpEncode (WKClientCurrencyDenominationBundle bundle,
                                                 BRRlpCoder coder);


private_extern WKClientCurrencyDenominationBundle
wkClientCurrencyDenominationBundleRlpDecode (BRRlpItem item,
                                                 BRRlpCoder coder);


struct WKClientCurrencyBundleRecord {
    char *id;
    char *name;
    char *code;
    char *type;
    char *bid;
    char *address;
    bool  verfified;
    BRArrayOf(WKClientCurrencyDenominationBundle) denominations;
};

private_extern BRRlpItem
wkClientCurrencyBundleRlpEncode (WKClientCurrencyBundle bundle,
                                     BRRlpCoder coder);

private_extern WKClientCurrencyBundle
wkClientCurrencyBundleRlpDecode (BRRlpItem item,
                                     BRRlpCoder coder);

extern OwnershipGiven BRSetOf(WKClientCurrencyBundle)
wkClientCurrencyBundleSetCreate (size_t size);

extern void
wkClientCurrencyBundleSetRelease (OwnershipGiven BRSetOf(WKClientCurrencyBundle) bundles);

// MARK: - Client Callback

typedef enum  {
    CLIENT_CALLBACK_REQUEST_BLOCK_NUMBER,
    CLIENT_CALLBACK_REQUEST_TRANSFERS,
    CLIENT_CALLBACK_REQUEST_TRANSACTIONS,
    CLIENT_CALLBACK_SUBMIT_TRANSACTION,
    CLIENT_CALLBACK_ESTIMATE_TRANSACTION_FEE,
} WKClientCallbackType;

struct WKClientCallbackStateRecord {
    WKClientCallbackType type;
    union {
        struct {
            BRSetOf(WKAddress) addresses;
        } getTransfers;

        struct {
            BRSetOf(WKAddress) addresses;
        } getTransactions;

        struct {
            WKWallet wallet;
            WKTransfer transfer;
        } submitTransaction;

        struct {
            WKHash hash;
            WKCookie cookie;
            WKTransfer   transfer;
            WKNetworkFee networkFee;
        } estimateTransactionFee;
        // ...
    } u;
    size_t rid;
};

// MARK: - P2P/QRY Type

typedef struct WKClientP2PManagerRecord *WKClientP2PManager;
typedef struct WKClientQRYManagerRecord *WKClientQRYManager;

typedef enum {
    WK_CLIENT_P2P_MANAGER_TYPE,
    WK_CLIENT_QRY_MANAGER_TYPE
} WKClientManagerType;

// MARK: - Client Sync

typedef struct {
    WKClientManagerType type;
    union {
        WKClientP2PManager p2pManager;
        WKClientQRYManager qryManager;
    } u;
} WKClientSync;

extern void
wkClientSync (WKClientSync sync,
                  WKSyncDepth depth,
                  WKBlockNumber height);

extern void
wkClientSyncPeriodic (WKClientSync sync);

// MARK: Client Send

typedef struct {
    WKClientManagerType type;
    union {
        WKClientP2PManager p2pManager;
        WKClientQRYManager qryManager;
    } u;
} WKClientSend;

extern void
wkClientSend (WKClientSend send,
                  WKWallet wallet,
                  WKTransfer transfer);

// MARK: Client P2P (Peer-to-Peer)

typedef void
(*WKClientP2PManagerReleaseHandler) (WKClientP2PManager p2p);

typedef void
(*WKClientP2PManagerConnectHandler) (WKClientP2PManager p2p,
                                           WKPeer peer);

typedef void
(*WKClientP2PManagerDisconnectHandler) (WKClientP2PManager p2p);

typedef void
(*WKClientP2PManagerSyncHandler) (WKClientP2PManager p2p,
                                        WKSyncDepth depth,
                                        WKBlockNumber height);

typedef void
(*WKClientP2PManagerSendHandler) (WKClientP2PManager p2p,
                                        WKWallet   wallet,
                                        WKTransfer transfer);

typedef void
(*WKClientP2PManagerSetNetworkReachableHandler) (WKClientP2PManager p2p,
                                                       int isNetworkReachable);

typedef struct {
    WKClientP2PManagerReleaseHandler release;
    WKClientP2PManagerConnectHandler connect;
    WKClientP2PManagerDisconnectHandler disconnect;
    WKClientP2PManagerSyncHandler sync;
    WKClientP2PManagerSendHandler send;
    WKClientP2PManagerSetNetworkReachableHandler setNetworkReachable;
} WKClientP2PHandlers;

struct WKClientP2PManagerRecord {
    WKNetworkType type;
    const WKClientP2PHandlers *handlers;
    size_t sizeInBytes;

    // State

    pthread_mutex_t lock;
};

extern WKClientP2PManager
wkClientP2PManagerCreate (size_t sizeInBytes,
                              WKNetworkType type,
                              const WKClientP2PHandlers *handlers);

extern void
wkClientP2PManagerRelease (WKClientP2PManager p2p);

extern void
wkClientP2PManagerConnect (WKClientP2PManager p2p,
                               WKPeer peer);

extern void
wkClientP2PManagerDisconnect (WKClientP2PManager p2p);

extern void
wkClientP2PManagerSetNetworkReachable (WKClientP2PManager p2p,
                                           WKBoolean isNetworkReachable);

static inline WKClientSync
wkClientP2PManagerAsSync (WKClientP2PManager p2p) {
    return (WKClientSync) {
        WK_CLIENT_P2P_MANAGER_TYPE,
        { .p2pManager = p2p }
    };
}

static inline WKClientSend
wkClientP2PManagerAsSend (WKClientP2PManager p2p) {
    return (WKClientSend) {
        WK_CLIENT_P2P_MANAGER_TYPE,
        { .p2pManager = p2p }
    };
}

// MARK: Client QRY (QueRY)

typedef enum {
    WK_CLIENT_REQUEST_USE_TRANSFERS,
    WK_CLIENT_REQUEST_USE_TRANSACTIONS,
} WKClientQRYByType;

struct WKClientQRYManagerRecord {
    WKClient client;
    WKWalletManager manager;
    WKClientQRYByType byType;
    WKBlockNumber blockNumberOffset;

    struct {
        bool completed;
        bool success;
        bool unbounded;     // true if `endBlockNumber` should be unbounded on request
        WKBlockNumber begBlockNumber;
        WKBlockNumber endBlockNumber;
        size_t rid;
    } sync;

    bool connected;
    size_t requestId;

    pthread_mutex_t lock;
};

#define WK_CLIENT_QRY_IS_UNBOUNDED            (true)

extern WKClientQRYManager
wkClientQRYManagerCreate (WKClient client,
                              WKWalletManager manager,
                              WKClientQRYByType byType,
                              WKBlockNumber earliestBlockNumber,
                              WKBlockNumber currentBlockNumber);

extern void
wkClientQRYManagerRelease (WKClientQRYManager qry);

extern void
wkClientQRYManagerConnect (WKClientQRYManager qry);

extern void
wkClientQRYManagerDisconnect (WKClientQRYManager qry);

extern void
wkClientQRYManagerTickTock (WKClientQRYManager qry);

extern void
wkClientQRYEstimateTransferFee (WKClientQRYManager qry,
                                    WKCookie   cookie,
                                    WKTransfer transfer,
                                    WKNetworkFee networkFee);

static inline WKClientSync
wkClientQRYManagerAsSync (WKClientQRYManager qry) {
    return (WKClientSync) {
        WK_CLIENT_QRY_MANAGER_TYPE,
        { .qryManager = qry }
    };
}

static inline WKClientSend
wkClientQRYManagerAsSend (WKClientQRYManager qry) {
    return (WKClientSend) {
        WK_CLIENT_QRY_MANAGER_TYPE,
        { .qryManager = qry }
    };
}

extern BREventType handleClientAnnounceBlockNumberEventType;
extern BREventType handleClientAnnounceTransactionsEventType;
extern BREventType handleClientAnnounceTransfersEventType;
extern BREventType handleClientAnnounceSubmitEventType;
extern BREventType handleClientAnnounceEstimateTransactionFeeEventType;

#define WK_CLIENT_EVENT_TYPES             \
  &handleClientAnnounceBlockNumberEventType,  \
  &handleClientAnnounceTransactionsEventType, \
  &handleClientAnnounceTransfersEventType,    \
  &handleClientAnnounceSubmitEventType,       \
  &handleClientAnnounceEstimateTransactionFeeEventType

#ifdef __cplusplus
}
#endif

#endif /* WKClientP_h */
