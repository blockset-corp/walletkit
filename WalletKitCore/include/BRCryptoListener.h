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


#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Listener


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
                      BRCryptoListenerNetworkCallback networkCallback,
                      BRCryptoListenerWalletManagerCallback managerCallback,
                      BRCryptoListenerWalletCallback walletCallback,
                      BRCryptoListenerTransferCallback transferCallback);


DECLARE_CRYPTO_GIVE_TAKE (BRCryptoListener, cryptoListener);

// MARK: - Network Listener

typedef struct {
    BRCryptoListener listener;
} BRCryptoNetworkListener;

extern void
cryptoListenerGenerateNetworkEvent (const BRCryptoNetworkListener *listener,
                                    BRCryptoNetwork network,
                                    BRCryptoNetworkEvent event);

// MARK: - Transfer Listener

typedef struct {
    BRCryptoListener listener;
    BRCryptoWalletManager manager;
    BRCryptoWallet wallet;
} BRCryptoTransferListener;

extern void
cryptoListenerGenerateTransferEvent (const BRCryptoTransferListener *listener,
                                     BRCryptoTransfer transfer,
                                     BRCryptoTransferEvent event);

// MARK: - Wallet Listener

typedef struct {
    BRCryptoListener listener;
    BRCryptoWalletManager manager;
} BRCryptoWalletListener;

static inline BRCryptoTransferListener
cryptoListenerCreateTransferListener (const BRCryptoWalletListener *listener,
                                      BRCryptoWallet wallet) {
    return (BRCryptoTransferListener) {
        listener->listener,
        listener->manager,
        wallet
    };
}

extern void
cryptoListenerGenerateWalletEvent (const BRCryptoWalletListener *listener,
                                   BRCryptoWallet wallet,
                                   BRCryptoWalletEvent event);

// MARK: - Wallet Manager Listener

typedef struct {
    BRCryptoListener listener;
} BRCryptoWalletManagerListener;

static inline BRCryptoWalletListener
cryptoListenerCreateWalletListener (const BRCryptoWalletManagerListener *listener,
                                    BRCryptoWalletManager manager) {
    return (BRCryptoWalletListener) {
        listener->listener,
        manager,
    };
}

extern void
cryptoListenerGenerateManagerEvent (const BRCryptoWalletManagerListener *listener,
                                    BRCryptoWalletManager manager,
                                    BRCryptoWalletManagerEvent event);


#ifdef __cplusplus
}
#endif

#endif /* BRCryptoListener_h */
