//
//  WKClientP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 04/28/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKListenerP_h
#define WKListenerP_h

#include "WKListener.h"
#include "support/event/BREvent.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Network Listener

/**
 * A Network Listener
 *
 * @discussion A NetworkListener includes the system holding the network.
 */
typedef struct {
    WKListener listener;
    WKSystem system;
} WKNetworkListener;

#define WK_NETWORK_LISTENER_EMPTY       ((WKNetworkListener) { NULL, NULL })

private_extern void
wkListenerGenerateNetworkEvent (const WKNetworkListener *listener,
                                    WKNetwork network,
                                    WKNetworkEvent event);


// MARK: - Transfer Listener

// A Hack: Instead Wallet should listen for WK_TRANSFER_EVENT_CHANGED
typedef void
(*WKTransferStateChangedCallback) (WKWallet wallet,
                                         WKTransfer transfer,
                                         OwnershipKept WKTransferState newState);


/**
 * A Transfer Listener
 *
 * @discussion A TransferListener includes the system, manager and wallet holding the transfer
 */
typedef struct {
    WKListener listener;
    WKSystem system;
    WKWalletManager manager;
    WKWallet wallet;

    // A Hack: Instead Wallet should listen for WK_TRANSFER_EVENT_CHANGED
    WKTransferStateChangedCallback transferChangedCallback;
} WKTransferListener;

#define WK_TRANSFER_LISTENER_EMPTY      ((WKTransferListener) { NULL, NULL, NULL, NULL, NULL })

private_extern void
wkListenerGenerateTransferEvent (const WKTransferListener *listener,
                                     WKTransfer transfer,
                                     WKTransferEvent event);

// MARK: - Wallet Listener

/**
 * A Wallet Listener
 *
 * @discussion A WalletListener includes the system and manager holding the wallet
 */
typedef struct {
    WKListener listener;
    WKSystem system;
    WKWalletManager manager;
    WKTransferStateChangedCallback transferChangedCallback;
} WKWalletListener;

#define WK_WALLET_LISTENER_EMPTY       ((WKWalletListener) { NULL, NULL, NULL, NULL })

private_extern void
wkListenerGenerateWalletEvent (const WKWalletListener *listener,
                                   WKWallet wallet,
                                   WKWalletEvent event);

/**
 * Create a TransferListener derived from a `wallet` and (wallet)`listener`
 */
static inline WKTransferListener
wkListenerCreateTransferListener (const WKWalletListener *listener,
                                      WKWallet wallet,
                                      // A Hack: Instead Wallet should listen for WK_TRANSFER_EVENT_CHANGED
                                      WKTransferStateChangedCallback transferChangedCallback) {
    return (WKTransferListener) {
        listener->listener,
        listener->system,
        listener->manager,
        wallet,
        transferChangedCallback
    };
}

// MARK: - Wallet Manager Listener

/**
 * A Wallet Manager Listener
 *
 * @discussion A WalletManagerListener includes the system holding the manager.
 */
typedef struct {
    WKListener listener;
    WKSystem system;
} WKWalletManagerListener;

#define WK_WALLET_MANAGER_LISTENER_EMPTY       ((WKWalletManagerListener) { NULL, NULL })

private_extern void
wkListenerGenerateManagerEvent (const WKWalletManagerListener *listener,
                                    WKWalletManager manager,
                                    WKWalletManagerEvent event);

/**
 * Create a WalletListener derived from a wallet `manager` and a (wallet manager) `listener`
 */
static inline WKWalletListener
wkListenerCreateWalletListener (const WKWalletManagerListener *listener,
                                    WKWalletManager manager) {
    return (WKWalletListener) {
        listener->listener,
        listener->system,
        manager,
    };
}

// MARK: - System Listener

private_extern void
wkListenerGenerateSystemEvent (WKListener listener,
                                   WKSystem system,
                                   WKSystemEvent event);

/**
 * Create a NetworkListener from `system` and `listener`
 */
static inline WKNetworkListener
wkListenerCreateNetworkListener (WKListener listener,
                                     WKSystem system) {
    return (WKNetworkListener) {
        listener,
        system
    };
}

/**
 * Create a WalletManagerListener from a `system` and `listener`
 */
static inline WKWalletManagerListener
wkListenerCreateWalletManagerListener (WKListener listener,
                                           WKSystem system) {
    return (WKWalletManagerListener) {
        listener,
        system
    };
}


// MARK: Crypto Listener

struct WKListenerRecord {
    WKRef ref;
    pthread_mutex_t lock;
    BREventHandler handler;

    WKListenerContext context;

    WKListenerSystemCallback        systemCallback;
    WKListenerNetworkCallback       networkCallback;
    WKListenerWalletManagerCallback managerCallback;
    WKListenerWalletCallback        walletCallback;
    WKListenerTransferCallback      transferCallback;
};

extern void
wkListenerStart (WKListener listener);

extern void
wkListenerStop (WKListener listener);

#ifdef __cplusplus
}
#endif

#endif /* WKListenerP_h */
