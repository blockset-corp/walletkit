//
//  BRCryptoSystem.h
//  BRCore
//
//  Created by Ed Gamble on 8/11/20.
//  Copyright © 2019 breadwallet. All rights reserved.
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

extern BRCryptoSystem
cryptoSystemCreate (BRCryptoClient client,
                    BRCryptoListener listener,
                    BRCryptoAccount account,
                    const char *path,
                    BRCryptoBoolean onMainnet);

extern BRCryptoBoolean
cryptoSystemOnMainnet (BRCryptoSystem system);

extern BRCryptoBoolean
cryptoSystemIsReachable (BRCryptoSystem system);

extern void
cryptoSystemSetReachable (BRCryptoSystem system,
                          BRCryptoBoolean isReachable);

extern const char *
cryptoSystemGetResolvedPath (BRCryptoSystem system);

extern BRCryptoSystemState
cryptoSystemGetState (BRCryptoSystem system);

// MARK: - System Networks

extern BRCryptoBoolean
cryptoSystemHasNetwork (BRCryptoSystem system,
                        BRCryptoNetwork network);

extern BRCryptoNetwork *
cryptoSystemGetNetworks (BRCryptoSystem system,
                         size_t *count);

extern BRCryptoNetwork
cryptoSystemGetNetworkAt (BRCryptoSystem system,
                          size_t index);

extern BRCryptoNetwork
cryptoSystemGetNetworkForUids (BRCryptoSystem system,
                               const char *uids);

extern size_t
cryptoSystemGetNetworksCount (BRCryptoSystem system);

// MARK: - System Wallet Managers

extern BRCryptoBoolean
cryptoSystemHasWalletManager (BRCryptoSystem system,
                              BRCryptoWalletManager network);

extern BRCryptoWalletManager *
cryptoSystemGetWalletManagers (BRCryptoSystem system,
                               size_t *count);

extern BRCryptoWalletManager
cryptoSystemGetWalletManagerAt (BRCryptoSystem system,
                                size_t index);

extern BRCryptoWalletManager
cryptoSystemGetWalletManagerByNetwork (BRCryptoSystem system,
                                       BRCryptoNetwork network);

extern size_t
cryptoSystemGetWalletManagersCount (BRCryptoSystem system);

extern BRCryptoWalletManager
cryptoSystemCreateWalletManager (BRCryptoSystem system,
                                 BRCryptoNetwork network,
                                 BRCryptoSyncMode mode,
                                 BRCryptoAddressScheme scheme,
                                 BRCryptoCurrency *currencies,
                                 size_t currenciesCount);

extern void
cryptoSystemStart (BRCryptoSystem system);

extern void
cryptoSystemStop (BRCryptoSystem system);

extern void
cryptoSystemConnect (BRCryptoSystem system);

extern void
cryptoSystemDisconnect (BRCryptoSystem system);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoSystem_h */
