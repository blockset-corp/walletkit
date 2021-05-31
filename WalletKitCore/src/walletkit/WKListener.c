//
//  WKListener.c
//  WK
//
//  Created by Ed Gamble on 8/11/20
//  Copyright © 2020 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "WKListenerP.h"
#include "support/BROSCompat.h"

#include "WKNetwork.h"
#include "WKTransfer.h"
#include "WKWallet.h"
#include "WKWalletManager.h"
#include "WKSystem.h"

IMPLEMENT_WK_GIVE_TAKE (WKListener, wkListener)

// MARK: - Generate Transfer Event

typedef struct {
    BREvent base;
    WKListener listener;
    WKWalletManager manager;
    WKWallet wallet;
    WKTransfer transfer;
    WKTransferEvent event;
} BRListenerSignalTransferEvent;

static void
wkListenerSignalTransferEventDispatcher (BREventHandler ignore,
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
    (BREventDispatcher) wkListenerSignalTransferEventDispatcher
};

private_extern void
wkListenerGenerateTransferEvent (const WKTransferListener *listener,
                                     WKTransfer transfer,
                                     WKTransferEvent event) {
    if (NULL == listener || NULL == listener->listener) return;

    BRListenerSignalTransferEvent listenerEvent =
    { { NULL, &handleListenerSignalTransferEventType},
        listener->listener,
        wkWalletManagerTakeWeak (listener->manager),
        wkWalletTakeWeak (listener->wallet),
        wkTransferTakeWeak (transfer),
        event };

    eventHandlerSignalEvent(listener->listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Generate Wallet Event

typedef struct {
    BREvent base;
    WKListener listener;
    WKWalletManager manager;
    WKWallet wallet;
    WKWalletEvent event;
} BRListenerSignalWalletEvent;

static void
wkListenerSignalWalletEventDispatcher (BREventHandler ignore,
                                           BRListenerSignalWalletEvent *event) {
    event->listener->walletCallback (event->listener->context,
                                     event->manager,
                                     event->wallet,
                                     event->event);
}

static BREventType handleListenerSignalWalletEventType = {
    "CWM: Handle Listener Wallet Event",
    sizeof (BRListenerSignalWalletEvent),
    (BREventDispatcher) wkListenerSignalWalletEventDispatcher
};

private_extern void
wkListenerGenerateWalletEvent (const WKWalletListener *listener,
                                   WKWallet wallet,
                                   OwnershipGiven WKWalletEvent event) {
    if (NULL == listener || NULL == listener->listener) return;

    BRListenerSignalWalletEvent listenerEvent =
    { { NULL, &handleListenerSignalWalletEventType},
        listener->listener,
        wkWalletManagerTakeWeak (listener->manager),
        wkWalletTakeWeak (wallet),
        event };

    eventHandlerSignalEvent(listener->listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Generate Manager Event

typedef struct {
    BREvent base;
    WKListener listener;
    WKWalletManager manager;
    WKWalletManagerEvent event;
} BRListenerSignalManagerEvent;

static void
wkListenerSignalManagerEventDispatcher (BREventHandler ignore,
                                            BRListenerSignalManagerEvent *event) {
    event->listener->managerCallback (event->listener->context,
                                      event->manager,
                                      event->event);
}

static BREventType handleListenerSignalManagerEventType = {
    "CWM: Handle Listener Manager Event",
    sizeof (BRListenerSignalManagerEvent),
    (BREventDispatcher) wkListenerSignalManagerEventDispatcher
};

private_extern void
wkListenerGenerateManagerEvent (const WKWalletManagerListener *listener,
                                    WKWalletManager manager,
                                    WKWalletManagerEvent event) {
    if (NULL == listener || NULL == listener->listener) return;

    BRListenerSignalManagerEvent listenerEvent =
    { { NULL, &handleListenerSignalManagerEventType},
        listener->listener,
        wkWalletManagerTakeWeak (manager),
        event };

    eventHandlerSignalEvent (listener->listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Generate Network Event

typedef struct {
    BREvent base;
    WKListener listener;
    WKNetwork network;
    WKNetworkEvent event;
} BRListenerSignalNetworkEvent;

static void
wkListenerSignalNetworkEventDispatcher (BREventHandler ignore,
                                            BRListenerSignalNetworkEvent *event) {
    event->listener->networkCallback (event->listener->context,
                                      event->network,
                                      event->event);
}

static BREventType handleListenerSignalNetworkEventType = {
    "CWM: Handle Listener Network Event",
    sizeof (BRListenerSignalNetworkEvent),
    (BREventDispatcher) wkListenerSignalNetworkEventDispatcher
};

private_extern void
wkListenerGenerateNetworkEvent (const WKNetworkListener *listener,
                                    WKNetwork network,
                                    WKNetworkEvent event) {
    if (NULL == listener || NULL == listener->listener) return;

    BRListenerSignalNetworkEvent listenerEvent =
    { { NULL, &handleListenerSignalNetworkEventType},
        listener->listener,
        wkNetworkTakeWeak (network),
        event };

    eventHandlerSignalEvent (listener->listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Generate System Event

typedef struct {
    BREvent base;
    WKListener listener;
    WKSystem system;
    WKSystemEvent event;
} BRListenerSignalSystemEvent;

static void
wkListenerSignalSystemEventDispatcher (BREventHandler ignore,
                                           BRListenerSignalSystemEvent *event) {
    event->listener->systemCallback (event->listener->context,
                                     event->system,
                                     event->event);
}

static BREventType handleListenerSignalSystemEventType = {
    "CWM: Handle Listener System Event",
    sizeof (BRListenerSignalSystemEvent),
    (BREventDispatcher) wkListenerSignalSystemEventDispatcher
};

private_extern void
wkListenerGenerateSystemEvent (WKListener listener,
                                   WKSystem system,
                                   WKSystemEvent event) {
    if (NULL == listener) return;

    BRListenerSignalSystemEvent listenerEvent =
    { { NULL, &handleListenerSignalSystemEventType},
        listener,
        wkSystemTakeWeak (system),
        event };

    eventHandlerSignalEvent (listener->handler, (BREvent *) &listenerEvent);
}

// MARK: - Event Type

static const BREventType *
wkListenerEventTypes[] = {
    &handleListenerSignalNetworkEventType,
    &handleListenerSignalTransferEventType,
    &handleListenerSignalWalletEventType,
    &handleListenerSignalManagerEventType,
    &handleListenerSignalSystemEventType
};

static const unsigned int
wkListenerEventTypesCount = (sizeof (wkListenerEventTypes) / sizeof(BREventType*)); //  11

extern WKListener
wkListenerCreate (WKListenerContext context,
                      WKListenerSystemCallback systemCallback,
                      WKListenerNetworkCallback networkCallback,
                      WKListenerWalletManagerCallback managerCallback,
                      WKListenerWalletCallback walletCallback,
                      WKListenerTransferCallback transferCallback) {
    
    WKListener listener = calloc (1, sizeof (struct WKListenerRecord));

    listener->ref = WK_REF_ASSIGN (wkListenerRelease);
    pthread_mutex_init_brd (&listener->lock, PTHREAD_MUTEX_NORMAL_BRD);

    listener->context          = context;
    listener->systemCallback   = systemCallback;
    listener->networkCallback  = networkCallback;
    listener->managerCallback  = managerCallback;
    listener->walletCallback   = walletCallback;
    listener->transferCallback = transferCallback;

    listener->handler = eventHandlerCreate ("Core SYS, Listener",
                                            wkListenerEventTypes,
                                            wkListenerEventTypesCount,
                                            &listener->lock);

    return listener;
}

static void
wkListenerRelease (WKListener listener) {
    eventHandlerStop (listener->handler);
    eventHandlerDestroy (listener->handler);

    pthread_mutex_destroy (&listener->lock);

    memset (listener, 0, sizeof(*listener));
    free (listener);
}

extern void
wkListenerStart (WKListener listener) {
    eventHandlerStart (listener->handler);
}

extern void
wkListenerStop (WKListener listener) {
    eventHandlerStop (listener->handler);
}
