//
//  BRCryptoSystemP.h
//  BRCore
//
//  Created by Ed Gamble on 8/11/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoSystemP_h
#define BRCryptoSystemP_h

#include "support/BRArray.h"

#include "BRCryptoSystem.h"
#include "BRCryptoBaseP.h"

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - System

struct BRCryptoSystemRecord {
    BRCryptoRef ref;
    pthread_mutex_t lock;

    BRCryptoBoolean onMainnet;
    BRCryptoBoolean isReachable;
    BRCryptoSystemState state;

    BRCryptoClient client;
    BRCryptoListener listener;

    BRCryptoAccount account;
    char *path;

    BRFileService fileService;
    
    BRArrayOf (BRCryptoNetwork) networks;
    BRArrayOf (BRCryptoWalletManager) managers;
};


private_extern void
cryptoSystemSetState (BRCryptoSystem system,
                      BRCryptoSystemState state);

private_extern void
cryptoSystemSetReachable (BRCryptoSystem system,
                          BRCryptoBoolean isReachable);

private_extern void
cryptoSystemAddNetwork (BRCryptoSystem system,
                        BRCryptoNetwork network);

private_extern void
cryptoSystemRemNetwork (BRCryptoSystem system,
                        BRCryptoNetwork network);

private_extern void
cryptoSystemAddWalletManager (BRCryptoSystem system,
                              BRCryptoWalletManager manager);

private_extern void
cryptoSystemRemWalletManager (BRCryptoSystem system,
                              BRCryptoWalletManager manager);

private_extern void
cryptoSystemHandleCurrencyBundles (BRCryptoSystem system,
                                   OwnershipKept BRArrayOf (BRCryptoClientCurrencyBundle) bundles);

static inline void
cryptoSystemGenerateEvent (BRCryptoSystem system,
                           BRCryptoSystemEvent event) {
    cryptoListenerGenerateSystemEvent (system->listener, system, event);
}

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoSystemP_h */
