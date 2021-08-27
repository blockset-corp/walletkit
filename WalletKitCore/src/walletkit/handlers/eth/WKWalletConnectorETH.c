//
//  WKWalletConnectorETH.c
//  WalletKitCore
//
//  Created by Bryan Goring on 8/23/21.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"
#include "WKETH.h"

static WKWalletConnector
wkWalletConnectorCreateETH (WKWalletManager manager) {
    WKWalletConnector connector =
    wkWalletConnectorAllocAndInit (sizeof (struct WKWalletConnectorETHRecord),
                                   manager->type,
                                   manager);

    // ...

    return connector;
}

static void
wkWalletConnectorReleaseETH (WKWalletConnector connector) {
    // ...
    return;
}

WKWalletConnectorHandlers wkWalletConnectorHandlersETH = {
    wkWalletConnectorCreateETH,
    wkWalletConnectorReleaseETH
};
