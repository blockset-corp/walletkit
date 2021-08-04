//
//  WKHandlersAVAX.c
//  WalletKitCore
//
//  Created by Amit Shah on 2021-08-04.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"

// MARK: - AVAX Handlers

extern WKAccountHandlers wkAccountHandlersAVAX;
extern WKAddressHandlers wkAddressHandlersAVAX;
extern WKNetworkHandlers wkNetworkHandlersAVAX;
extern WKTransferHandlers wkTransferHandlersAVAX;
extern WKWalletHandlers wkWalletHandlersAVAX;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersAVAX;
extern WKWalletManagerHandlers wkWalletManagerHandlersAVAX;

WKHandlers wkHandlersAVAX = {
    WK_NETWORK_TYPE_AVAX,
    &wkAccountHandlersAVAX,
    &wkNetworkHandlersAVAX,
    &wkAddressHandlersAVAX,
    &wkTransferHandlersAVAX,
    &wkWalletHandlersAVAX,
    NULL,//WKWalletSweeperHandlers not supported
    NULL,//WKExportablePaperWalletHandlers
    NULL,//WKPaymentProtocolHandlers not supported
    &wkFeeBasisHandlersAVAX,
    &wkWalletManagerHandlersAVAX
};
