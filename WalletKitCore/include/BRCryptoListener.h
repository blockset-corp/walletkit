//
//  BRCryptoListener.h
//  BRCrypto
//
//  Created by Ed Gamblle on 8/11/20.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoListener_h
#define BRCryptoListener_h

#include "BRCryptoBase.h"
#include "event/BRCryptoTransfer.h"
#include "event/BRCryptoWallet.h"
#include "event/BRCryptoWalletManager.h"
#include "event/BRCryptoNetwork.h"
#include "event/BRCryptoSystem.h"


#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Listener

typedef void (*BRCryptoListenerSystemCallback) (BRCryptoListenerContext context,
                                                BRCryptoSystem system,
                                                BRCryptoSystemEvent event);

typedef void (*BRCryptoListenerNetworkCallback) (BRCryptoListenerContext context,
                                                 BRCryptoNetwork network,
                                                 BRCryptoNetworkEvent event);

typedef void (*BRCryptoListenerWalletManagerCallback) (BRCryptoListenerContext context,
                                                       BRCryptoWalletManager manager,
                                                       BRCryptoWalletManagerEvent event);


typedef void (*BRCryptoListenerWalletCallback) (BRCryptoListenerContext context,
                                                BRCryptoWalletManager manager,
                                                BRCryptoWallet wallet,
                                                BRCryptoWalletEvent event);

typedef void (*BRCryptoListenerTransferCallback) (BRCryptoListenerContext context,
                                                  BRCryptoWalletManager manager,
                                                  BRCryptoWallet wallet,
                                                  BRCryptoTransfer transfer,
                                                  BRCryptoTransferEvent event);

extern BRCryptoListener
cryptoListenerCreate (BRCryptoListenerContext context,
                      BRCryptoListenerSystemCallback systemCallback,
                      BRCryptoListenerNetworkCallback networkCallback,
                      BRCryptoListenerWalletManagerCallback managerCallback,
                      BRCryptoListenerWalletCallback walletCallback,
                      BRCryptoListenerTransferCallback transferCallback);


DECLARE_CRYPTO_GIVE_TAKE (BRCryptoListener, cryptoListener);

// MARK: - Network Listener

typedef struct {
    BRCryptoListener listener;
    BRCryptoSystem system;
} BRCryptoNetworkListener;

#define CRYPTO_NETWORK_LISTENER_EMPTY       ((BRCryptoNetworkListener) { NULL, NULL })

extern void
cryptoListenerGenerateNetworkEvent (const BRCryptoNetworkListener *listener,
                                    BRCryptoNetwork network,
                                    BRCryptoNetworkEvent event);

// MARK: - Transfer Listener

// A Hack: Instead Wallet should listen for CRYPTO_TRANSFER_EVENT_CHANGED
typedef void
(*BRCryptoTransferStateChangedCallback) (BRCryptoWallet wallet,
                                         BRCryptoTransfer transfer,
                                         OwnershipKept BRCryptoTransferState newState);

typedef struct {
    BRCryptoListener listener;
    BRCryptoSystem system;
    BRCryptoWalletManager manager;
    BRCryptoWallet wallet;

    // A Hack: Instead Wallet should listen for CRYPTO_TRANSFER_EVENT_CHANGED
    BRCryptoTransferStateChangedCallback transferChangedCallback;
} BRCryptoTransferListener;

#define CRYPTO_TRANSFER_LISTENER_EMPTY      ((BRCryptoTransferListener) { NULL, NULL, NULL, NULL, NULL })

extern void
cryptoListenerGenerateTransferEvent (const BRCryptoTransferListener *listener,
                                     BRCryptoTransfer transfer,
                                     BRCryptoTransferEvent event);

// MARK: - Wallet Listener

typedef struct {
    BRCryptoListener listener;
    BRCryptoSystem system;
    BRCryptoWalletManager manager;
    BRCryptoTransferStateChangedCallback transferChangedCallback;
} BRCryptoWalletListener;

#define CRYPTO_WALLET_LISTENER_EMPTY       ((BRCryptoWalletListener) { NULL, NULL, NULL, NULL })

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

extern void
cryptoListenerGenerateWalletEvent (const BRCryptoWalletListener *listener,
                                   BRCryptoWallet wallet,
                                   BRCryptoWalletEvent event);

// MARK: - Wallet Manager Listener

typedef struct {
    BRCryptoListener listener;
    BRCryptoSystem system;
} BRCryptoWalletManagerListener;

#define CRYPTO_WALLET_MANAGER_LISTENER_EMPTY       ((BRCryptoWalletManagerListener) { NULL, NULL })

static inline BRCryptoWalletListener
cryptoListenerCreateWalletListener (const BRCryptoWalletManagerListener *listener,
                                    BRCryptoWalletManager manager) {
    return (BRCryptoWalletListener) {
        listener->listener,
        listener->system,
        manager,
    };
}

extern void
cryptoListenerGenerateManagerEvent (const BRCryptoWalletManagerListener *listener,
                                    BRCryptoWalletManager manager,
                                    BRCryptoWalletManagerEvent event);

// MARK: - System Listener

static inline BRCryptoNetworkListener
cryptoListenerCreateNetworkListener (BRCryptoListener listener,
                                     BRCryptoSystem system) {
    return (BRCryptoNetworkListener) {
        listener,
        system
    };
}

static inline BRCryptoWalletManagerListener
cryptoListenerCreateWalletManagerListener (BRCryptoListener listener,
                                           BRCryptoSystem system) {
    return (BRCryptoWalletManagerListener) {
        listener,
        system
    };
}

extern void
cryptoListenerGenerateSystemEvent (BRCryptoListener listener,
                                   BRCryptoSystem system,
                                   BRCryptoSystemEvent event);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoListener_h */
