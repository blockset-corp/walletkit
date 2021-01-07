//
//  BRCryptoListener.c
//  BRCrypto
//
//  Created by Ed Gamble on 8/11/20
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoListenerP.h"
#include "support/BROSCompat.h"

#include "BRCryptoNetwork.h"
#include "BRCryptoTransfer.h"
#include "BRCryptoWallet.h"
#include "BRCryptoWalletManager.h"
#include "BRCryptoSystem.h"

IMPLEMENT_CRYPTO_GIVE_TAKE (BRCryptoListener, cryptoListener)

// MARK: - Generate Transfer Event

typedef struct {
    BREvent base;
    BRCryptoListener listener;
    BRCryptoWalletManager manager;
    BRCryptoWallet wallet;
    BRCryptoTransfer transfer;
    BRCryptoTransferEvent event;
} BRListenerSignalTransferEvent;

static void
cryptoListenerSignalTransferEventDispatcher (BREventHandler ignore,
                                             BRListenerSignalTransferEvent *event) {
    event->listener->transferCallback (event->listener->context,
                                       event->manager,
                                       event->wallet,
                                       event->transfer,
                                       event->event);
}

static BREventType handleListenerSignalTransferEventType = {
    "CWM: Handle Listener Transfer EVent",
    sizeof (BRListenerSignalTransferEvent),
    (BREventDispatcher) cryptoListenerSignalTransferEventDispatcher
};

extern void
cryptoListenerGenerateTransferEvent (const BRCryptoTransferListener *listener,
                                     BRCryptoTransfer transfer,
                                     BRCryptoTransferEvent event) {
    if (NULL == listener || NULL == listener->listener) return;

    BRListenerSignalTransferEvent listenerEvent =
    { { NULL, &handleListenerSignalTransferEventType},
        listener->listener,
        cryptoWalletManagerTakeWeak (listener->manager),
        cryptoWalletTakeWeak (listener->wallet),
        cryptoTransferTakeWeak (transfer),
        event };

    eventHandlerSignalEvent(listener->listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Generate Wallet Event

typedef struct {
    BREvent base;
    BRCryptoListener listener;
    BRCryptoWalletManager manager;
    BRCryptoWallet wallet;
    BRCryptoWalletEvent event;
} BRListenerSignalWalletEvent;

static void
cryptoListenerSignalWalletEventDispatcher (BREventHandler ignore,
                                           BRListenerSignalWalletEvent *event) {
    event->listener->walletCallback (event->listener->context,
                                     event->manager,
                                     event->wallet,
                                     event->event);
}

static BREventType handleListenerSignalWalletEventType = {
    "CWM: Handle Listener Wallet Event",
    sizeof (BRListenerSignalWalletEvent),
    (BREventDispatcher) cryptoListenerSignalWalletEventDispatcher
};

extern void
cryptoListenerGenerateWalletEvent (const BRCryptoWalletListener *listener,
                                   BRCryptoWallet wallet,
                                   OwnershipGiven BRCryptoWalletEvent event) {
    if (NULL == listener || NULL == listener->listener) return;

    BRListenerSignalWalletEvent listenerEvent =
    { { NULL, &handleListenerSignalWalletEventType},
        listener->listener,
        cryptoWalletManagerTakeWeak (listener->manager),
        cryptoWalletTakeWeak (wallet),
        event };

    eventHandlerSignalEvent(listener->listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Generate Manager Event

typedef struct {
    BREvent base;
    BRCryptoListener listener;
    BRCryptoWalletManager manager;
    BRCryptoWalletManagerEvent event;
} BRListenerSignalManagerEvent;

static void
cryptoListenerSignalManagerEventDispatcher (BREventHandler ignore,
                                            BRListenerSignalManagerEvent *event) {
    event->listener->managerCallback (event->listener->context,
                                      event->manager,
                                      event->event);
}

static BREventType handleListenerSignalManagerEventType = {
    "CWM: Handle Listener Manager Event",
    sizeof (BRListenerSignalManagerEvent),
    (BREventDispatcher) cryptoListenerSignalManagerEventDispatcher
};

extern void
cryptoListenerGenerateManagerEvent (const BRCryptoWalletManagerListener *listener,
                                    BRCryptoWalletManager manager,
                                    BRCryptoWalletManagerEvent event) {
    if (NULL == listener || NULL == listener->listener) return;

    BRListenerSignalManagerEvent listenerEvent =
    { { NULL, &handleListenerSignalManagerEventType},
        listener->listener,
        cryptoWalletManagerTakeWeak (manager),
        event };

    eventHandlerSignalEvent (listener->listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Generate Network Event

typedef struct {
    BREvent base;
    BRCryptoListener listener;
    BRCryptoNetwork network;
    BRCryptoNetworkEvent event;
} BRListenerSignalNetworkEvent;

static void
cryptoListenerSignalNetworkEventDispatcher (BREventHandler ignore,
                                            BRListenerSignalNetworkEvent *event) {
    event->listener->networkCallback (event->listener->context,
                                      event->network,
                                      event->event);
}

static BREventType handleListenerSignalNetworkEventType = {
    "CWM: Handle Listener Network Event",
    sizeof (BRListenerSignalNetworkEvent),
    (BREventDispatcher) cryptoListenerSignalNetworkEventDispatcher
};

extern void
cryptoListenerGenerateNetworkEvent (const BRCryptoNetworkListener *listener,
                                    BRCryptoNetwork network,
                                    BRCryptoNetworkEvent event) {
    if (NULL == listener || NULL == listener->listener) return;

    BRListenerSignalNetworkEvent listenerEvent =
    { { NULL, &handleListenerSignalNetworkEventType},
        listener->listener,
        cryptoNetworkTakeWeak (network),
        event };

    eventHandlerSignalEvent (listener->listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Generate System Event

typedef struct {
    BREvent base;
    BRCryptoListener listener;
    BRCryptoSystem system;
    BRCryptoSystemEvent event;
} BRListenerSignalSystemEvent;

static void
cryptoListenerSignalSystemEventDispatcher (BREventHandler ignore,
                                           BRListenerSignalSystemEvent *event) {
    event->listener->systemCallback (event->listener->context,
                                     event->system,
                                     event->event);
}

static BREventType handleListenerSignalSystemEventType = {
    "CWM: Handle Listener System Event",
    sizeof (BRListenerSignalSystemEvent),
    (BREventDispatcher) cryptoListenerSignalSystemEventDispatcher
};

extern void
cryptoListenerGenerateSystemEvent (BRCryptoListener listener,
                                   BRCryptoSystem system,
                                   BRCryptoSystemEvent event) {
    if (NULL == listener) return;

    BRListenerSignalSystemEvent listenerEvent =
    { { NULL, &handleListenerSignalSystemEventType},
        listener,
        cryptoSystemTakeWeak (system),
        event };

    eventHandlerSignalEvent (listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Event Type

static const BREventType *
cryptoListenerEventTypes[] = {
    &handleListenerSignalNetworkEventType,
    &handleListenerSignalTransferEventType,
    &handleListenerSignalWalletEventType,
    &handleListenerSignalManagerEventType,
    &handleListenerSignalSystemEventType
};

static const unsigned int
cryptoListenerEventTypesCount = (sizeof (cryptoListenerEventTypes) / sizeof(BREventType*)); //  11

extern BRCryptoListener
cryptoListenerCreate (BRCryptoListenerContext context,
                      BRCryptoListenerSystemCallback systemCallback,
                      BRCryptoListenerNetworkCallback networkCallback,
                      BRCryptoListenerWalletManagerCallback managerCallback,
                      BRCryptoListenerWalletCallback walletCallback,
                      BRCryptoListenerTransferCallback transferCallback) {
    
    BRCryptoListener listener = calloc (1, sizeof (struct BRCryptoListenerRecord));

    listener->ref = CRYPTO_REF_ASSIGN (cryptoListenerRelease);
    pthread_mutex_init_brd (&listener->lock, PTHREAD_MUTEX_NORMAL);

    listener->context          = context;
    listener->systemCallback   = systemCallback;
    listener->networkCallback  = networkCallback;
    listener->managerCallback  = managerCallback;
    listener->walletCallback   = walletCallback;
    listener->transferCallback = transferCallback;

    listener->handler = eventHandlerCreate ("Core SYS, Listener",
                                            cryptoListenerEventTypes,
                                            cryptoListenerEventTypesCount,
                                            &listener->lock);

    return listener;
}

static void
cryptoListenerRelease (BRCryptoListener listener) {
    eventHandlerStop (listener->handler);
    eventHandlerDestroy (listener->handler);

    pthread_mutex_destroy (&listener->lock);

    memset (listener, 0, sizeof(*listener));
    free (listener);
}

extern void
cryptoListenerStart (BRCryptoListener listener) {
    eventHandlerStart (listener->handler);
}

extern void
cryptoListenerStop (BRCryptoListener listener) {
    eventHandlerStop (listener->handler);
}
