//
//  BRCryptoClientP.h
//  BRCore
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
