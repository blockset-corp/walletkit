//
//  WKHandlers__SYMBOL__.c
//  WalletKitCore
//
//  Created by Ed Gamble on 06/22/2021.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"

// MARK: - __SYMBOL__ Handlers

extern WKAccountHandlers wkAccountHandlers__SYMBOL__;
extern WKAddressHandlers wkAddressHandlers__SYMBOL__;
extern WKNetworkHandlers wkNetworkHandlers__SYMBOL__;
extern WKTransferHandlers wkTransferHandlers__SYMBOL__;
extern WKWalletHandlers wkWalletHandlers__SYMBOL__;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlers__SYMBOL__;
extern WKWalletManagerHandlers wkWalletManagerHandlers__SYMBOL__;

WKHandlers wkHandlers__SYMBOL__ = {
    WK_NETWORK_TYPE___SYMBOL__,
    &wkAccountHandlers__SYMBOL__,
    &wkNetworkHandlers__SYMBOL__,
    &wkAddressHandlers__SYMBOL__,
    &wkTransferHandlers__SYMBOL__,
    &wkWalletHandlers__SYMBOL__,
    NULL,//WKWalletSweeperHandlers not supported
    NULL,//WKExportablePaperWalletHandlers
    NULL,//WKPaymentProtocolHandlers not supported
    &wkFeeBasisHandlers__SYMBOL__,
    &wkWalletManagerHandlers__SYMBOL__
};
