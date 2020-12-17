//
//  BRCryptoSystemEvent.h
//  BRCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoSystemEvent_h
#define BRCryptoSystemEvent_h

#include "BRCryptoBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: System State & Event

typedef enum {
    CRYPTO_SYSTEM_STATE_CREATED,
    CRYPTO_SYSTEM_STATE_DELETED
} BRCryptoSystemState;

typedef enum {
    CRYPTO_SYSTEM_EVENT_CREATED,
    CRYPTO_SYSTEM_EVENT_CHANGED,
    CRYPTO_SYSTEM_EVENT_DELETED,

    CRYPTO_SYSTEM_EVENT_NETWORK_ADDED,
    CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED,
    CRYPTO_SYSTEM_EVENT_NETWORK_DELETED,

    CRYPTO_SYSTEM_EVENT_MANAGER_ADDED,
    CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED,
    CRYPTO_SYSTEM_EVENT_MANAGER_DELETED,

    CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS,
    
} BRCryptoSystemEventType;

extern const char *
cryptoSystemEventTypeString (BRCryptoSystemEventType t);

typedef struct {
    BRCryptoSystemEventType type;
    union {
        struct {
            BRCryptoSystemState old;
            BRCryptoSystemState new;
        } state;

        BRCryptoNetwork network;
        BRCryptoWalletManager manager;
    } u;
} BRCryptoSystemEvent;


#ifdef __cplusplus
}
#endif

#endif /* BRCryptoSystemEvent_h */
