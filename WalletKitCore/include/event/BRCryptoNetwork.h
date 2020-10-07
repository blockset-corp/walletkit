//
//  BRCryptoNetworkEvent.h
//  BRCore
//
//  Created by Ed Gamble on 8/12/20.
//  Copyright © 2019 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BRCryptoNetworkEvent_h
#define BRCryptoNetworkEvent_h

#include "BRCryptoBase.h"

#ifdef __cplusplus
extern "C" {
#endif

/// MARK: - Network Event

typedef enum {
    CRYPTO_NETWORK_EVENT_CREATED,
    CRYPTO_NETWORK_EVENT_FEES_UPDATED,
    CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED,
    CRYPTO_NETWORK_EVENT_DELETED,
} BRCryptoNetworkEventType;

extern const char *
cryptoNetworkEventTypeString (BRCryptoNetworkEventType t);

typedef struct {
    BRCryptoNetworkEventType type;
    // No union; no data (at this time).
} BRCryptoNetworkEvent;

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoNetworkEvent_h */
