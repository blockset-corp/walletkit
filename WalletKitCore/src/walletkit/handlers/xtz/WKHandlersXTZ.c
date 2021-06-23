//
//  WKHandlersXTZ.c
//  WalletKitCore
//
//  Created by Ed Gamble on 06/22/2021.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"

// MARK: - XTZ Handlers

extern WKAccountHandlers wkAccountHandlersXTZ;
extern WKAddressHandlers wkAddressHandlersXTZ;
extern WKNetworkHandlers wkNetworkHandlersXTZ;
extern WKTransferHandlers wkTransferHandlersXTZ;
extern WKWalletHandlers wkWalletHandlersXTZ;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersXTZ;
extern WKWalletManagerHandlers wkWalletManagerHandlersXTZ;

WKHandlers wkHandlersXTZ = {
    WK_NETWORK_TYPE_XTZ,
    &wkAccountHandlersXTZ,
    &wkNetworkHandlersXTZ,
    &wkAddressHandlersXTZ,
    &wkTransferHandlersXTZ,
    &wkWalletHandlersXTZ,
    NULL,//WKWalletSweeperHandlers not supported
    NULL,//WKExportablePaperWalletHandlers
    NULL,//WKPaymentProtocolHandlers not supported
    &wkFeeBasisHandlersXTZ,
    &wkWalletManagerHandlersXTZ
};
