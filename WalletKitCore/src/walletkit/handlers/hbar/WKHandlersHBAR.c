//
//  WKHandlersHBAR.c
//  WalletKitCore
//
//  Created by Ed Gamble on 06/22/2021.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"

// MARK: - HBAR Handlers

extern WKAccountHandlers wkAccountHandlersHBAR;
extern WKAddressHandlers wkAddressHandlersHBAR;
extern WKNetworkHandlers wkNetworkHandlersHBAR;
extern WKTransferHandlers wkTransferHandlersHBAR;
extern WKWalletHandlers wkWalletHandlersHBAR;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersHBAR;
extern WKWalletManagerHandlers wkWalletManagerHandlersHBAR;

WKHandlers wkHandlersHBAR = {
    WK_NETWORK_TYPE_HBAR,
    &wkAccountHandlersHBAR,
    &wkNetworkHandlersHBAR,
    &wkAddressHandlersHBAR,
    &wkTransferHandlersHBAR,
    &wkWalletHandlersHBAR,
    NULL,//WKWalletSweeperHandlers not supported
    NULL,//WKExportablePaperWalletHandlers
    NULL,//WKPaymentProtocolHandlers not supported
    &wkFeeBasisHandlersHBAR,
    &wkWalletManagerHandlersHBAR
};
