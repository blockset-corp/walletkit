//
//  WKSystemP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/11/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKSystemP_h
#define WKSystemP_h

#include "support/BRArray.h"

#include "WKSystem.h"
#include "WKBaseP.h"
#include "WKListenerP.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - System

struct WKSystemRecord {
    WKRef ref;
    pthread_mutex_t lock;

    WKBoolean onMainnet;
    WKBoolean isReachable;
    WKSystemState state;

    WKClient client;
    WKListener listener;

    WKAccount account;
    char *path;

    BRFileService fileService;
    
    BRArrayOf (WKNetwork) networks;
    BRArrayOf (WKWalletManager) managers;
};


private_extern void
wkSystemSetState (WKSystem system,
                      WKSystemState state);

private_extern void
wkSystemSetReachable (WKSystem system,
                          WKBoolean isReachable);

private_extern void
wkSystemAddNetwork (WKSystem system,
                        WKNetwork network);

private_extern void
wkSystemRemNetwork (WKSystem system,
                        WKNetwork network);

private_extern void
wkSystemAddWalletManager (WKSystem system,
                              WKWalletManager manager);

private_extern void
wkSystemRemWalletManager (WKSystem system,
                              WKWalletManager manager);

private_extern void
wkSystemHandleCurrencyBundles (WKSystem system,
                                   OwnershipKept BRArrayOf (WKClientCurrencyBundle) bundles);

static inline void
wkSystemGenerateEvent (WKSystem system,
                           WKSystemEvent event) {
    wkListenerGenerateSystemEvent (system->listener, system, event);
}

#ifdef __cplusplus
}
#endif

#endif /* WKSystemP_h */
