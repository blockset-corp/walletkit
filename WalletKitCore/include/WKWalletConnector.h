//
//  WKWalletConnector.h
//  WalletKitCore
//
//  Created by Bryan Goring on 8/23/21
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef WKWalletConnector_h
#define WKWalletConnector_h

#include "WKWalletManager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WALLET_CONNECT_ERROR_UNSUPPORTED_CONNECTOR
    // ...
} WKWalletConnectorError;

typedef struct WKWalletConnectorRecord *WKWalletConnector;

extern WKWalletConnector
wkWalletConnectorCreate (WKWalletManager manager);

extern void
wkWalletConnectorRelease (WKWalletConnector connector);

#ifdef __cplusplus
}
#endif

#endif // WKWalletConnector_h
