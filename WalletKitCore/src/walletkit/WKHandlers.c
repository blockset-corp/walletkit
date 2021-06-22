//
//  WKHandlers.c
//  WalletKitCore
//
//  Created by Ed Gamble on 4/24/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKHandlersP.h"

// The specific handlers for each supported currency
#define DEFINE_HANDLERS(type, name)    extern WKHandlers wkHandlers ## name ;
#include "WKConfig.h"
#undef DEFINE_HANDLERS

// Must be ordered by WKBlockChainType enumeration values.
static WKHandlers *handlers[NUMBER_OF_NETWORK_TYPES];

static pthread_once_t  wkHandlersLookupOnce = PTHREAD_ONCE_INIT;

static void wkHandlersLookupInit (void) {
#define DEFINE_HANDLERS(type, name)   handlers[type] =  &wkHandlers ## name;
#include "WKConfig.h"
#undef DEFINE_HANDLERS
}

extern const WKHandlers *
wkHandlersLookup (WKNetworkType type) {
    pthread_once (&wkHandlersLookupOnce, wkHandlersLookupInit);
    assert (type < NUMBER_OF_NETWORK_TYPES);
    return handlers[type];
}
