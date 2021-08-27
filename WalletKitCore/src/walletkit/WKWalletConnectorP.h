//
//  WKWalletConnectorP.h
//  WalletKitCore
//
//  Created by Bryan Goring on 8/23/21
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletConnectorP_h
#define WKWalletConnectorP_h

#include "WKWalletConnector.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - Connector Handlers

typedef WKWalletConnector
(*WKWalletConnectorCreateHandler) (WKWalletManager manager);

typedef void
(*WKWalletConnectorReleaseHandler) (WKWalletConnector connector);

// ...

typedef struct {
    WKWalletConnectorCreateHandler create;
    WKWalletConnectorReleaseHandler release;
    // ...
} WKWalletConnectorHandlers;

// MARK: - Connector

struct WKWalletConnectorRecord {
    WKNetworkType type;
    const WKWalletConnectorHandlers *handlers;
    size_t sizeInBytes;

    WKWalletManager manager;
    // ...
};

private_extern WKWalletConnector
wkWalletConnectorAllocAndInit (size_t sizeInBytes,
                               WKNetworkType type,
                               WKWalletManager manager);

#ifdef __cplusplus
}
#endif

#endif /* WKWalletConnectorP_h */
