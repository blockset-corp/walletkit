//
//  BRCryptoWalletManagerEvent.c
//  BRCore
//
//  Created by Michael Carrara on 3/19/19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "BRCryptoBTC.h"

//#include "support/BRBase.h"
//#include "BRCryptoWalletManager.h"
//#include "BRCryptoWalletManagerPrivate.h"
#include "bitcoin/BRTransaction.h"

//
// Wallet Callbacks
//


#if 0
//
// WalletManager Event
//

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    BRCryptoWalletManagerEvent event;
} BRCryptoWalletManagerWMEvent;

static void
bwmSignalWalletManagerWMEventDispatcher (BREventHandler ignore,
                                         BRCryptoWalletManagerWMEvent *event) {
    bwmHandleWalletManagerEvent(event->manager, event->event);
}

static BREventType bwmWalletManagerEventType = {
    "BTC: WalletManager Event",
    sizeof (BRCryptoWalletManagerWMEvent),
    (BREventDispatcher) bwmSignalWalletManagerWMEventDispatcher
};

extern void
bwmSignalWalletManagerEvent (BRCryptoWalletManager manager,
                             BRCryptoWalletManagerEvent event) {
    BRCryptoWalletManagerWMEvent message =
    { { NULL, &bwmWalletManagerEventType}, manager, event};
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}

//
// Wallet Event
//

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    BRWallet *wallet;
    BRWalletEvent event;
} BRCryptoWalletManagerWalletEvent;

static void
bwmSignalWalletEventDispatcher (BREventHandler ignore,
                                BRCryptoWalletManagerWalletEvent *event) {
    bwmHandleWalletEvent(event->manager, event->wallet, event->event);
}

static BREventType bwmWalletEventType = {
    "BTC: Wallet Event",
    sizeof (BRCryptoWalletManagerWalletEvent),
    (BREventDispatcher) bwmSignalWalletEventDispatcher
};

extern void
bwmSignalWalletEvent (BRCryptoWalletManager manager,
                      BRWallet *wallet,
                      BRWalletEvent event) {
    BRCryptoWalletManagerWalletEvent message =
    { { NULL, &bwmWalletEventType}, manager, wallet, event};
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}

//
// Wallet Event
//

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    BRWallet *wallet;
    BRTransaction *transaction;
    BRTransactionEvent event;
} BRCryptoWalletManagerTransactionEvent;

static void
bwmSignalTransactionEventDispatcher (BREventHandler ignore,
                                     BRCryptoWalletManagerTransactionEvent *event) {
    bwmHandleTransactionEvent(event->manager, event->wallet, event->transaction, event->event);
}

static BREventType bwmTransactionEventType = {
    "BTC: Transaction Event",
    sizeof (BRCryptoWalletManagerTransactionEvent),
    (BREventDispatcher) bwmSignalTransactionEventDispatcher
};

extern void
bwmSignalTransactionEvent (BRCryptoWalletManager manager,
                           BRWallet *wallet,
                           BRTransaction *transaction,
                           BRTransactionEvent event) {
    BRCryptoWalletManagerTransactionEvent message =
    { { NULL, &bwmTransactionEventType}, manager, wallet, transaction, event};
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}

//
// Announce Block Number
//

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    int rid;
    uint64_t blockNumber;
} BRCryptoWalletManagerClientAnnounceBlockNumberEvent;

static void
bwmSignalAnnounceBlockNumberDispatcher (BREventHandler ignore,
                                        BRCryptoWalletManagerClientAnnounceBlockNumberEvent *event) {
    bwmHandleAnnounceBlockNumber(event->manager, event->rid, event->blockNumber);
}

static BREventType bwmClientAnnounceBlockNumberEventType = {
    "BTC: Client Announce Block Number Event",
    sizeof (BRCryptoWalletManagerClientAnnounceBlockNumberEvent),
    (BREventDispatcher) bwmSignalAnnounceBlockNumberDispatcher
};

extern void
bwmSignalAnnounceBlockNumber (BRCryptoWalletManager manager,
                              int rid,
                              uint64_t blockNumber) {
    BRCryptoWalletManagerClientAnnounceBlockNumberEvent message =
    { { NULL, &bwmClientAnnounceBlockNumberEventType}, manager, rid, blockNumber};
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}

//
// Announce Transaction
//

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    int rid;
    uint8_t *transaction;
    size_t transactionLength;
    uint64_t timestamp;
    uint64_t blockHeight;
    uint8_t  error;
} BRCryptoWalletManagerClientAnnounceTransactionEvent;

static void
bwmSignalAnnounceTransactionDispatcher (BREventHandler ignore,
                                        BRCryptoWalletManagerClientAnnounceTransactionEvent *event) {
    bwmHandleAnnounceTransaction(event->manager,
                                 event->rid,
                                 event->transaction,
                                 event->transactionLength,
                                 event->timestamp,
                                 event->blockHeight,
                                 event->error);
    free (event->transaction);
}

static void
bwmSignalAnnounceTransactionDestroyer (BRCryptoWalletManagerClientAnnounceTransactionEvent *event) {
    free (event->transaction);
}

static BREventType bwmClientAnnounceTransactionEventType = {
    "BWM: Client Announce Transaction Event",
    sizeof (BRCryptoWalletManagerClientAnnounceTransactionEvent),
    (BREventDispatcher) bwmSignalAnnounceTransactionDispatcher,
    (BREventDestroyer) bwmSignalAnnounceTransactionDestroyer
};

extern void
bwmSignalAnnounceTransaction(BRCryptoWalletManager manager,
                             int rid,
                             OwnershipKept uint8_t *transaction,
                             size_t transactionLength,
                             uint64_t timestamp,
                             uint64_t blockHeight,
                             uint8_t  error) {
    uint8_t *transactionCopy = malloc (transactionLength);
    memcpy (transactionCopy, transaction, transactionLength);

    BRCryptoWalletManagerClientAnnounceTransactionEvent message =
    { { NULL, &bwmClientAnnounceTransactionEventType}, manager, rid, transactionCopy, transactionLength, timestamp, blockHeight, error };
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}

//
// Announce Transaction Complete
//

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    int rid;
    int success;
} BRCryptoWalletManagerClientAnnounceTransactionCompleteEvent;

static void
bwmSignalAnnounceTransactionCompleteDispatcher (BREventHandler ignore,
                                     BRCryptoWalletManagerClientAnnounceTransactionCompleteEvent *event) {
    bwmHandleAnnounceTransactionComplete(event->manager, event->rid, event->success);
}

static BREventType bwmClientAnnounceTransactionCompleteEventType = {
    "BWM: Client Announce Get Transactions Complete Event",
    sizeof (BRCryptoWalletManagerClientAnnounceTransactionCompleteEvent),
    (BREventDispatcher) bwmSignalAnnounceTransactionCompleteDispatcher
};

extern void
bwmSignalAnnounceTransactionComplete (BRCryptoWalletManager manager,
                                      int rid,
                                      int success) {
    BRCryptoWalletManagerClientAnnounceTransactionCompleteEvent message =
    { { NULL, &bwmClientAnnounceTransactionCompleteEventType}, manager, rid, success };
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}

// Announce Submit

typedef struct {
    struct BREventRecord base;
    BRCryptoWalletManager manager;
    int rid;
    UInt256 txHash;
    int error;
} BRCryptoWalletManagerClientAnnounceSubmitEvent;

static void
bwmSignalAnnounceSubmitDispatcher (BREventHandler ignore,
                                   BRCryptoWalletManagerClientAnnounceSubmitEvent *event) {
    bwmHandleAnnounceSubmit(event->manager, event->rid, event->txHash, event->error);
}

static BREventType bwmClientAnnounceSubmitEventType = {
    "BTC: Client Announce Submit Event",
    sizeof (BRCryptoWalletManagerClientAnnounceSubmitEvent),
    (BREventDispatcher) bwmSignalAnnounceSubmitDispatcher
};

extern void
bwmSignalAnnounceSubmit (BRCryptoWalletManager manager,
                         int rid,
                         UInt256 txHash,
                         int error) {
    BRCryptoWalletManagerClientAnnounceSubmitEvent message =
    { { NULL, &bwmClientAnnounceSubmitEventType}, manager, rid, txHash, error};
    eventHandlerSignalEvent (manager->handler, (BREvent*) &message);
}
#endif
// ==============================================================================================
//
// All Event Types
//
