//
//  BRCryptoClientP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 04/28/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoListenerP_h
#define BRCryptoListenerP_h

#include "BRCryptoListener.h"
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
    BRCryptoListener listener;
    BRCryptoSystem system;
} BRCryptoNetworkListener;

#define CRYPTO_NETWORK_LISTENER_EMPTY       ((BRCryptoNetworkListener) { NULL, NULL })

private_extern void
cryptoListenerGenerateNetworkEvent (const BRCryptoNetworkListener *listener,
                                    BRCryptoNetwork network,
                                    BRCryptoNetworkEvent event);


// MARK: - Transfer Listener

// A Hack: Instead Wallet should listen for CRYPTO_TRANSFER_EVENT_CHANGED
typedef void
(*BRCryptoTransferStateChangedCallback) (BRCryptoWallet wallet,
                                         BRCryptoTransfer transfer,
                                         OwnershipKept BRCryptoTransferState newState);


/**
 * A Transfer Listener
 *
 * @discussion A TransferListener includes the system, manager and wallet holding the transfer
 */
typedef struct {
    BRCryptoListener listener;
    BRCryptoSystem system;
    BRCryptoWalletManager manager;
    BRCryptoWallet wallet;

    // A Hack: Instead Wallet should listen for CRYPTO_TRANSFER_EVENT_CHANGED
    BRCryptoTransferStateChangedCallback transferChangedCallback;
} BRCryptoTransferListener;

#define CRYPTO_TRANSFER_LISTENER_EMPTY      ((BRCryptoTransferListener) { NULL, NULL, NULL, NULL, NULL })

private_extern void
cryptoListenerGenerateTransferEvent (const BRCryptoTransferListener *listener,
                                     BRCryptoTransfer transfer,
                                     BRCryptoTransferEvent event);

// MARK: - Wallet Listener

/**
 * A Wallet Listener
 *
 * @discussion A WalletListener includes the system and manager holding the wallet
 */
typedef struct {
    BRCryptoListener listener;
    BRCryptoSystem system;
    BRCryptoWalletManager manager;
    BRCryptoTransferStateChangedCallback transferChangedCallback;
} BRCryptoWalletListener;

#define CRYPTO_WALLET_LISTENER_EMPTY       ((BRCryptoWalletListener) { NULL, NULL, NULL, NULL })

private_extern void
cryptoListenerGenerateWalletEvent (const BRCryptoWalletListener *listener,
                                   BRCryptoWallet wallet,
                                   BRCryptoWalletEvent event);

/**
 * Create a TransferListener derived from a `wallet` and (wallet)`listener`
 */
static inline BRCryptoTransferListener
cryptoListenerCreateTransferListener (const BRCryptoWalletListener *listener,
                                      BRCryptoWallet wallet,
                                      // A Hack: Instead Wallet should listen for CRYPTO_TRANSFER_EVENT_CHANGED
                                      BRCryptoTransferStateChangedCallback transferChangedCallback) {
    return (BRCryptoTransferListener) {
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
    BRCryptoListener listener;
    BRCryptoSystem system;
} BRCryptoWalletManagerListener;

#define CRYPTO_WALLET_MANAGER_LISTENER_EMPTY       ((BRCryptoWalletManagerListener) { NULL, NULL })

private_extern void
cryptoListenerGenerateManagerEvent (const BRCryptoWalletManagerListener *listener,
                                    BRCryptoWalletManager manager,
                                    BRCryptoWalletManagerEvent event);

/**
 * Create a WalletListener derived from a wallet `manager` and a (wallet manager) `listener`
 */
static inline BRCryptoWalletListener
cryptoListenerCreateWalletListener (const BRCryptoWalletManagerListener *listener,
                                    BRCryptoWalletManager manager) {
    return (BRCryptoWalletListener) {
        listener->listener,
        listener->system,
        manager,
    };
}

// MARK: - System Listener

private_extern void
cryptoListenerGenerateSystemEvent (BRCryptoListener listener,
                                   BRCryptoSystem system,
                                   BRCryptoSystemEvent event);

/**
 * Create a NetworkListener from `system` and `listener`
 */
static inline BRCryptoNetworkListener
cryptoListenerCreateNetworkListener (BRCryptoListener listener,
                                     BRCryptoSystem system) {
    return (BRCryptoNetworkListener) {
        listener,
        system
    };
}

/**
 * Create a WalletManagerListener from a `system` and `listener`
 */
static inline BRCryptoWalletManagerListener
cryptoListenerCreateWalletManagerListener (BRCryptoListener listener,
                                           BRCryptoSystem system) {
    return (BRCryptoWalletManagerListener) {
        listener,
        system
    };
}


// MARK: Crypto Listener

struct BRCryptoListenerRecord {
    BRCryptoRef ref;
    pthread_mutex_t lock;
    BREventHandler handler;

    BRCryptoListenerContext context;

    BRCryptoListenerSystemCallback        systemCallback;
    BRCryptoListenerNetworkCallback       networkCallback;
    BRCryptoListenerWalletManagerCallback managerCallback;
    BRCryptoListenerWalletCallback        walletCallback;
    BRCryptoListenerTransferCallback      transferCallback;
};

extern void
cryptoListenerStart (BRCryptoListener listener);

extern void
cryptoListenerStop (BRCryptoListener listener);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoListenerP_h */
