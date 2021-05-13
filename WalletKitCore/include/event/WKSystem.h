//
//  WKSystemEvent.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKSystemEvent_h
#define WKSystemEvent_h

#include "WKBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: System State & Event

typedef enum {
    WK_SYSTEM_STATE_CREATED,
    WK_SYSTEM_STATE_DELETED
} WKSystemState;

typedef enum {
    WK_SYSTEM_EVENT_CREATED,
    WK_SYSTEM_EVENT_CHANGED,
    WK_SYSTEM_EVENT_DELETED,
    
    WK_SYSTEM_EVENT_NETWORK_ADDED,
    WK_SYSTEM_EVENT_NETWORK_CHANGED,
    WK_SYSTEM_EVENT_NETWORK_DELETED,
    
    WK_SYSTEM_EVENT_MANAGER_ADDED,
    WK_SYSTEM_EVENT_MANAGER_CHANGED,
    WK_SYSTEM_EVENT_MANAGER_DELETED,
    
    WK_SYSTEM_EVENT_DISCOVERED_NETWORKS,
    
} WKSystemEventType;

extern const char *
wkSystemEventTypeString (WKSystemEventType t);

typedef struct {
    WKSystemEventType type;
    union {
        struct {
            WKSystemState old;
            WKSystemState new;
        } state;
        
        WKNetwork network;
        WKWalletManager manager;
    } u;
} WKSystemEvent;


#ifdef __cplusplus
}
#endif

#endif /* WKSystemEvent_h */
