//
//  WKSystem.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/11/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKSystem_h
#define WKSystem_h

#include "WKNetwork.h"
#include "WKWalletManager.h"
#include "WKClient.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - System

DECLARE_WK_GIVE_TAKE (WKSystem, wkSystem);

/**
 * Create a system.
 */
extern WKSystem
wkSystemCreate (WKClient client,
                WKListener listener,
                WKAccount account,
                const char *path,
                WKBoolean onMainnet);

/**
 * Check if system is o mainnet.
 */
extern WKBoolean
wkSystemOnMainnet (WKSystem system);

/**
 * Check if the systen is 'reachable' - that is, has internet access.
 */
extern WKBoolean
wkSystemIsReachable (WKSystem system);

/**
 * Set the system's reachable state.  This should be called when the System user knows that
 * internet access has been lost ('airplane mode')
 */
extern void
wkSystemSetReachable (WKSystem system,
                      WKBoolean isReachable);

/**
 * Get the system's path used to store persistent data
 */
extern const char *
wkSystemGetResolvedPath (WKSystem system);

/**
 * Get the system's state
 */
extern WKSystemState
wkSystemGetState (WKSystem system);

// MARK: - System Networks

/**
 * Check if system has `network`
 */
extern WKBoolean
wkSystemHasNetwork (WKSystem system,
                    WKNetwork network);

/**
 * Get the system's networks.  Fills `count` with the number of networks returned.
 */
extern WKNetwork *
wkSystemGetNetworks (WKSystem system,
                     size_t *count);

/**
 * Get the system's network at `index`.  The `index` must be [0,count).
 */
extern WKNetwork
wkSystemGetNetworkAt (WKSystem system,
                      size_t index);

/**
 * Get the system network with `uids`.  Return can be NULL if `uids` is unknown.
 */
extern WKNetwork
wkSystemGetNetworkForUids (WKSystem system,
                           const char *uids);

/**
 * Get the count of system networks.
 */
extern size_t
wkSystemGetNetworksCount (WKSystem system);

// MARK: - System Wallet Managers

/**
 * Check if system as a wallet manager for `network`.
 */
extern WKBoolean
wkSystemHasWalletManager (WKSystem system,
                          WKWalletManager network);

/**
 * Get the system's wallet managers.  Fills `count` with the number of wallet managers returned.
 */
extern WKWalletManager *
wkSystemGetWalletManagers (WKSystem system,
                           size_t *count);

/**
 * Get the system wallet manager at `index`.  The `index` must be `[0,count)`
 */
extern WKWalletManager
wkSystemGetWalletManagerAt (WKSystem system,
                            size_t index);

/**
 * Get the system wallet manager for `network`.  Return can be NULL if `network` does not have
 * a corresponding wallet manager.
 */
extern WKWalletManager
wkSystemGetWalletManagerByNetwork (WKSystem system,
                                   WKNetwork network);

/**
 * Get the count of system wallet managers.
 */
extern size_t
wkSystemGetWalletManagersCount (WKSystem system);

/**
 * Create a wallet manager in `system` for `network`
 *
 * @param system the system
 * @param network the network
 * @param mode the sync mode
 * @param scheme the address scheme
 * @param currencies an array of currencies
 * @param currenciesCount the size of the currencies array
 */
extern WKWalletManager
wkSystemCreateWalletManager (WKSystem system,
                             WKNetwork network,
                             WKSyncMode mode,
                             WKAddressScheme scheme,
                             WKCurrency *currencies,
                             size_t currenciesCount);

/**
 * Start the system.
 *
 * @discussion The system will start generating listener events, such as announcing the system
 * creation and the availability of networks.
 */
extern void
wkSystemStart (WKSystem system);

/**
 * Stop the system.
 *
 * @discussion The system will stop generating events.  If the system is connected, then the
 * system remains connected and, specifically, wallet manager tasks continue.
 */
extern void
wkSystemStop (WKSystem system);

/**
 * Connect all the system wallet manager
 *
 * @discussion Each wallet manager held by the system is individually connected and thus begins
 * searching the corresponding blockchain for transfers.
 */
extern void
wkSystemConnect (WKSystem system);

/**
 * Disconnect all the system wallet managers.
 */
extern void
wkSystemDisconnect (WKSystem system);

#ifdef __cplusplus
}
#endif

#endif /* WKSystem_h */
