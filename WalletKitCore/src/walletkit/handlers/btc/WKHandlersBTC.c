//
//  WKHandlersBTC.c
//  WalletKitCore
//
//  Created by Ed Gamble on 06/22/2021.
//  Copyright © 2021 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

#include "walletkit/WKHandlersP.h"

// MARK: - BTC Handlers

extern WKAccountHandlers wkAccountHandlersBTC;
extern WKAddressHandlers wkAddressHandlersBTC;
extern WKNetworkHandlers wkNetworkHandlersBTC;
extern WKTransferHandlers wkTransferHandlersBTC;
extern WKWalletHandlers wkWalletHandlersBTC;
extern WKWalletSweeperHandlers wkWalletSweeperHandlersBTC;
extern WKExportablePaperWalletHandlers wkExportablePaperWalletHandlersBTC;
extern WKPaymentProtocolHandlers wkPaymentProtocolHandlersBTC;
extern WKFeeBasisHandlers wkFeeBasisHandlersBTC;
extern WKWalletManagerHandlers wkWalletManagerHandlersBTC;

WKHandlers wkHandlersBTC = {
    WK_NETWORK_TYPE_BTC,
    &wkAccountHandlersBTC,
    &wkNetworkHandlersBTC,
    &wkAddressHandlersBTC,
    &wkTransferHandlersBTC,
    &wkWalletHandlersBTC,
    &wkWalletSweeperHandlersBTC,
    &wkExportablePaperWalletHandlersBTC,
    &wkPaymentProtocolHandlersBTC,
    NULL,//WKWalletConnector
    &wkFeeBasisHandlersBTC,
    &wkWalletManagerHandlersBTC
};

// MARK: - BCH Handlers

extern WKAccountHandlers wkAccountHandlersBCH;
extern WKAddressHandlers wkAddressHandlersBCH;
extern WKNetworkHandlers wkNetworkHandlersBCH;
extern WKTransferHandlers wkTransferHandlersBCH;
extern WKWalletHandlers wkWalletHandlersBCH;
extern WKWalletSweeperHandlers wkWalletSweeperHandlersBCH;
// ExportablePaperHandlers
// Payment Protocol
// Fee Basis
extern WKWalletManagerHandlers wkWalletManagerHandlersBCH;

WKHandlers wkHandlersBCH = {
    WK_NETWORK_TYPE_BCH,
    &wkAccountHandlersBCH,
    &wkNetworkHandlersBCH,
    &wkAddressHandlersBCH,
    &wkTransferHandlersBCH,
    &wkWalletHandlersBCH,
    &wkWalletSweeperHandlersBCH,
    NULL,//WKExportablePaperWalletHandlers
    &wkPaymentProtocolHandlersBTC,
    NULL,//WKWalletConnector
    &wkFeeBasisHandlersBTC,
    &wkWalletManagerHandlersBCH
};

// MARK: - BSV Handlers

extern WKAccountHandlers wkAccountHandlersBSV;
extern WKAddressHandlers wkAddressHandlersBSV;
extern WKNetworkHandlers wkNetworkHandlersBSV;
extern WKTransferHandlers wkTransferHandlersBSV;
extern WKWalletHandlers wkWalletHandlersBSV;
extern WKWalletSweeperHandlers wkWalletSweeperHandlersBSV;
// ExportablePaperHandlers
// Payment Protocol
// Fee Basis
extern WKWalletManagerHandlers wkWalletManagerHandlersBSV;

WKHandlers wkHandlersBSV = {
    WK_NETWORK_TYPE_BSV,
    &wkAccountHandlersBSV,
    &wkNetworkHandlersBSV,
    &wkAddressHandlersBSV,
    &wkTransferHandlersBSV,
    &wkWalletHandlersBSV,
    &wkWalletSweeperHandlersBSV,
    NULL,//WKExportablePaperWalletHandlers
    NULL,//WKPaymentProtocolHandlers,
    NULL,//WKWalletConnector
    &wkFeeBasisHandlersBTC,
    &wkWalletManagerHandlersBSV
};

// MARK: - LTC Handlers

extern WKAccountHandlers wkAccountHandlersLTC;
extern WKAddressHandlers wkAddressHandlersLTC;
extern WKNetworkHandlers wkNetworkHandlersLTC;
extern WKTransferHandlers wkTransferHandlersLTC;
extern WKWalletHandlers wkWalletHandlersLTC;
extern WKWalletSweeperHandlers wkWalletSweeperHandlersLTC;
// ExportablePaperHandlers
// Payment Protocol
// Fee Basis
extern WKWalletManagerHandlers wkWalletManagerHandlersLTC;

WKHandlers wkHandlersLTC = {
    WK_NETWORK_TYPE_LTC,
    &wkAccountHandlersLTC,
    &wkNetworkHandlersLTC,
    &wkAddressHandlersLTC,
    &wkTransferHandlersLTC,
    &wkWalletHandlersLTC,
    &wkWalletSweeperHandlersLTC,
    NULL,//WKExportablePaperWalletHandlers
    &wkPaymentProtocolHandlersBTC,
    NULL,//WKWalletConnector
    &wkFeeBasisHandlersBTC,
    &wkWalletManagerHandlersLTC
};

// MARK: - DOGE Handlers

extern WKAccountHandlers wkAccountHandlersDOGE;
extern WKAddressHandlers wkAddressHandlersDOGE;
extern WKNetworkHandlers wkNetworkHandlersDOGE;
extern WKTransferHandlers wkTransferHandlersDOGE;
extern WKWalletHandlers wkWalletHandlersDOGE;
extern WKWalletSweeperHandlers wkWalletSweeperHandlersDOGE;
// ExportablePaperHandlers
// Payment Protocol
// Fee Basis
extern WKWalletManagerHandlers wkWalletManagerHandlersDOGE;

WKHandlers wkHandlersDOGE = {
    WK_NETWORK_TYPE_DOGE,
    &wkAccountHandlersDOGE,
    &wkNetworkHandlersDOGE,
    &wkAddressHandlersDOGE,
    &wkTransferHandlersDOGE,
    &wkWalletHandlersDOGE,
    &wkWalletSweeperHandlersDOGE,
    NULL,//WKExportablePaperWalletHandlers
    &wkPaymentProtocolHandlersBTC,
    NULL,//WKWalletConnector
    &wkFeeBasisHandlersBTC,
    &wkWalletManagerHandlersDOGE
};

