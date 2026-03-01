//
//  WKHandlersETH.c
//  WalletKitCore
//
//  Created by Ed Gamble on 06/22/2021.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"

// MARK: - ETH Handlers

extern WKAccountHandlers wkAccountHandlersETH;
extern WKAddressHandlers wkAddressHandlersETH;
extern WKNetworkHandlers wkNetworkHandlersETH;
extern WKTransferHandlers wkTransferHandlersETH;
extern WKWalletHandlers wkWalletHandlersETH;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKWalletConnectorHandlers wkWalletConnectorHandlersETH;
extern WKFeeBasisHandlers wkFeeBasisHandlersETH;
extern WKWalletManagerHandlers wkWalletManagerHandlersETH;

WKHandlers wkHandlersETH = {
    WK_NETWORK_TYPE_ETH,
    &wkAccountHandlersETH,
    &wkNetworkHandlersETH,
    &wkAddressHandlersETH,
    &wkTransferHandlersETH,
    &wkWalletHandlersETH,
    NULL,//WKWalletSweeperHandlers not supported
    NULL,//WKExportablePaperWalletHandlers
    NULL,//WKPaymentProtocolHandlers not supported
    &wkWalletConnectorHandlersETH,
    &wkFeeBasisHandlersETH,
    &wkWalletManagerHandlersETH
};
