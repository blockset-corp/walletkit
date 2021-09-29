//
//  WKNetworkEvent.h
//  WalletKitCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKNetworkEvent_h
#define WKNetworkEvent_h

#include "WKBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Network Event

typedef enum {
    WK_NETWORK_EVENT_CREATED,
    WK_NETWORK_EVENT_FEES_UPDATED,
    WK_NETWORK_EVENT_CURRENCIES_UPDATED,
    WK_NETWORK_EVENT_DELETED,
} WKNetworkEventType;

extern const char *
wkNetworkEventTypeString (WKNetworkEventType t);

typedef struct {
    WKNetworkEventType type;
    // No union; no data (at this time).
} WKNetworkEvent;

#ifdef __cplusplus
}
#endif

#endif /* WKNetworkEvent_h */
