//
//  WKHandlersXRP.c
//  WalletKitCore
//
//  Created by Ed Gamble on 06/22/2021.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"

// MARK: - XRP Handlers

extern WKAccountHandlers wkAccountHandlersXRP;
extern WKAddressHandlers wkAddressHandlersXRP;
extern WKNetworkHandlers wkNetworkHandlersXRP;
extern WKTransferHandlers wkTransferHandlersXRP;
extern WKWalletHandlers wkWalletHandlersXRP;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersXRP;
extern WKWalletManagerHandlers wkWalletManagerHandlersXRP;

WKHandlers wkHandlersXRP = {
    WK_NETWORK_TYPE_XRP,
    &wkAccountHandlersXRP,
    &wkNetworkHandlersXRP,
    &wkAddressHandlersXRP,
    &wkTransferHandlersXRP,
    &wkWalletHandlersXRP,
    NULL,//WKWalletSweeperHandlers not supported
    NULL,//WKExportablePaperWalletHandlers
    NULL,//WKPaymentProtocolHandlers not supported
    NULL,//WKWalletConnector
    &wkFeeBasisHandlersXRP,
    &wkWalletManagerHandlersXRP
};
