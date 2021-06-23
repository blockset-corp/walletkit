//
//  WKHandlersXLM.c
//  WalletKitCore
//
//  Created by Ed Gamble on 06/22/2021.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"

// MARK: - XLM Handlers

extern WKAccountHandlers wkAccountHandlersXLM;
extern WKAddressHandlers wkAddressHandlersXLM;
extern WKNetworkHandlers wkNetworkHandlersXLM;
extern WKTransferHandlers wkTransferHandlersXLM;
extern WKWalletHandlers wkWalletHandlersXLM;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersXLM;
extern WKWalletManagerHandlers wkWalletManagerHandlersXLM;

WKHandlers wkHandlersXLM = {
    WK_NETWORK_TYPE_XLM,
    &wkAccountHandlersXLM,
    &wkNetworkHandlersXLM,
    &wkAddressHandlersXLM,
    &wkTransferHandlersXLM,
    &wkWalletHandlersXLM,
    NULL,//WKWalletSweeperHandlers not supported
    NULL,//WKExportablePaperWalletHandlers
    NULL,//WKPaymentProtocolHandlers not supported
    &wkFeeBasisHandlersXLM,
    &wkWalletManagerHandlersXLM
};
