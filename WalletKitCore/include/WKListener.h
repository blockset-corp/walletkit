//
//  WKListener.h
//  WK
//
//  Created by Ed Gamblle on 8/11/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKListener_h
#define WKListener_h

#include "WKBase.h"
#include "event/WKTransfer.h"
#include "event/WKWallet.h"
#include "event/WKWalletManager.h"
#include "event/WKNetwork.h"
#include "event/WKSystem.h"


#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Listener

/**
 * Announce a SystemEvent
 */
typedef void (*WKListenerSystemCallback) (WKListenerContext context,
                                          OwnershipGiven WKSystem system,
                                          OwnershipGiven WKSystemEvent event);

/**
 * Announce a NetworkEvent
 */
typedef void (*WKListenerNetworkCallback) (WKListenerContext context,
                                           OwnershipGiven WKNetwork network,
                                           OwnershipGiven WKNetworkEvent event);

/**
 * Announce a WalletManagerEvent
 */
typedef void (*WKListenerWalletManagerCallback) (WKListenerContext context,
                                                 OwnershipGiven WKWalletManager manager,
                                                 OwnershipGiven WKWalletManagerEvent event);

/**
 * Announce a WalletEvent
 */
typedef void (*WKListenerWalletCallback) (WKListenerContext context,
                                          OwnershipGiven WKWalletManager manager,
                                          OwnershipGiven WKWallet wallet,
                                          OwnershipGiven WKWalletEvent event);

/**
 * Announce a TransferEvent
 */
typedef void (*WKListenerTransferCallback) (WKListenerContext context,
                                            OwnershipGiven WKWalletManager manager,
                                            OwnershipGiven WKWallet wallet,
                                            OwnershipGiven WKTransfer transfer,
                                            OwnershipGiven WKTransferEvent event);

/**
 * Create a Listener of System, Network, WalletManger, Wallet and Transfer events.
 */
extern WKListener
wkListenerCreate (WKListenerContext context,
                  WKListenerSystemCallback systemCallback,
                  WKListenerNetworkCallback networkCallback,
                  WKListenerWalletManagerCallback managerCallback,
                  WKListenerWalletCallback walletCallback,
                  WKListenerTransferCallback transferCallback);


DECLARE_WK_GIVE_TAKE (WKListener, wkListener);

#ifdef __cplusplus
}
#endif

#endif /* WKListener_h */
