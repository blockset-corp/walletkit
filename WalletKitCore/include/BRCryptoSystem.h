//
//  BRCryptoSystem.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/11/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoSystem_h
#define BRCryptoSystem_h

#include "BRCryptoNetwork.h"
#include "BRCryptoWalletManager.h"
#include "BRCryptoClient.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - System

DECLARE_CRYPTO_GIVE_TAKE (BRCryptoSystem, cryptoSystem);

/**
 * Create a system.
 */
extern BRCryptoSystem
cryptoSystemCreate (BRCryptoClient client,
                    BRCryptoListener listener,
                    BRCryptoAccount account,
                    const char *path,
                    BRCryptoBoolean onMainnet);

/**
 * Check if system is o mainnet.
 */
extern BRCryptoBoolean
cryptoSystemOnMainnet (BRCryptoSystem system);

/**
 * Check if the systen is 'reachable' - that is, has internet access.
 */
extern BRCryptoBoolean
cryptoSystemIsReachable (BRCryptoSystem system);

/**
 * Set the system's reachable state.  This should be called when the System user knows that
 * internet access has been lost ('airplane mode')
 */
extern void
cryptoSystemSetReachable (BRCryptoSystem system,
                          BRCryptoBoolean isReachable);

/**
 * Get the system's path used to store persistent data
 */
extern const char *
cryptoSystemGetResolvedPath (BRCryptoSystem system);

/**
 * Get the system's state
 */
extern BRCryptoSystemState
cryptoSystemGetState (BRCryptoSystem system);

// MARK: - System Networks

/**
 * Check if system has `network`
 */
extern BRCryptoBoolean
cryptoSystemHasNetwork (BRCryptoSystem system,
                        BRCryptoNetwork network);

/**
 * Get the system's networks.  Fills `count` with the number of networks returned.
 */
extern BRCryptoNetwork *
cryptoSystemGetNetworks (BRCryptoSystem system,
                         size_t *count);

/**
 * Get the system's network at `index`.  The `index` must be [0,count).
 */
extern BRCryptoNetwork
cryptoSystemGetNetworkAt (BRCryptoSystem system,
                          size_t index);

/**
 * Get the system network with `uids`.  Return can be NULL if `uids` is unknown.
 */
extern BRCryptoNetwork
cryptoSystemGetNetworkForUids (BRCryptoSystem system,
                               const char *uids);

/**
 * Get the count of system networks.
 */
extern size_t
cryptoSystemGetNetworksCount (BRCryptoSystem system);

// MARK: - System Wallet Managers

/**
 * Check if system as a wallet manager for `network`.
 */
extern BRCryptoBoolean
cryptoSystemHasWalletManager (BRCryptoSystem system,
                              BRCryptoWalletManager network);

/**
 * Get the system's wallet managers.  Fills `count` with the number of wallet managers returned.
 */
extern BRCryptoWalletManager *
cryptoSystemGetWalletManagers (BRCryptoSystem system,
                               size_t *count);

/**
 * Get the system wallet manager at `index`.  The `index` must be `[0,count)`
 */
extern BRCryptoWalletManager
cryptoSystemGetWalletManagerAt (BRCryptoSystem system,
                                size_t index);

/**
 * Get the system wallet manager for `network`.  Return can be NULL if `network` does not have
 * a corresponding wallet manager.
 */
extern BRCryptoWalletManager
cryptoSystemGetWalletManagerByNetwork (BRCryptoSystem system,
                                       BRCryptoNetwork network);

/**
 * Get the count of system wallet managers.
 */
extern size_t
cryptoSystemGetWalletManagersCount (BRCryptoSystem system);

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
extern BRCryptoWalletManager
cryptoSystemCreateWalletManager (BRCryptoSystem system,
                                 BRCryptoNetwork network,
                                 BRCryptoSyncMode mode,
                                 BRCryptoAddressScheme scheme,
                                 BRCryptoCurrency *currencies,
                                 size_t currenciesCount);

/**
 * Start the system.
 *
 * @discussion The system will start generating listener events, such as announcing the system
 * creation and the availability of networks.
 */
extern void
cryptoSystemStart (BRCryptoSystem system);

/**
 * Stop the system.
 *
 * @discussion The system will stop generating events.  If the system is connected, then the
 * system remains connected and, specifically, wallet manager tasks continue.
 */
extern void
cryptoSystemStop (BRCryptoSystem system);

/**
 * Connect all the system wallet manager
 *
 * @discussion Each wallet manager held by the system is individually connected and thus begins
 * searching the corresponding blockchain for transfers.
 */
extern void
cryptoSystemConnect (BRCryptoSystem system);

/**
 * Disconnect all the system wallet managers.
 */
extern void
cryptoSystemDisconnect (BRCryptoSystem system);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoSystem_h */
