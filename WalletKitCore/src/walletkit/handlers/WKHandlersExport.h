//
//  WKHandlersExport.h
//  WalletKitCore
//
//  Created by Ed Gamble on 05/11/2020.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKHandlersExport_h
#define WKHandlersExport_h

#include "../WKHandlersP.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - BTC Handlers

extern WKAddressHandlers wkAddressHandlersBTC;
extern WKNetworkHandlers wkNetworkHandlersBTC;
extern WKTransferHandlers wkTransferHandlersBTC;
extern WKWalletHandlers wkWalletHandlersBTC;
extern WKWalletSweeperHandlers wkWalletSweeperHandlersBTC;
extern WKExportablePaperWalletHandlers wkExportablePaperWalletHandlersBTC;
extern WKPaymentProtocolHandlers wkPaymentProtocolHandlersBTC;
extern WKFeeBasisHandlers wkFeeBasisHandlersBTC;
extern WKWalletManagerHandlers wkWalletManagerHandlersBTC;

// MARK: - BCH Handlers

extern WKAddressHandlers wkAddressHandlersBCH;
extern WKNetworkHandlers wkNetworkHandlersBCH;
extern WKTransferHandlers wkTransferHandlersBCH;
extern WKWalletHandlers wkWalletHandlersBCH;
extern WKWalletSweeperHandlers wkWalletSweeperHandlersBCH;
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersBCH;
extern WKWalletManagerHandlers wkWalletManagerHandlersBCH;

// MARK: - BSV Handlers

extern WKAddressHandlers wkAddressHandlersBSV;
extern WKNetworkHandlers wkNetworkHandlersBSV;
extern WKTransferHandlers wkTransferHandlersBSV;
extern WKWalletHandlers wkWalletHandlersBSV;
extern WKWalletSweeperHandlers wkWalletSweeperHandlersBSV;
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersBSV;
extern WKWalletManagerHandlers wkWalletManagerHandlersBSV;

// MARK: - ETH Handlers

extern WKAddressHandlers wkAddressHandlersETH;
extern WKNetworkHandlers wkNetworkHandlersETH;
extern WKTransferHandlers wkTransferHandlersETH;
extern WKWalletHandlers wkWalletHandlersETH;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersETH;
extern WKWalletManagerHandlers wkWalletManagerHandlersETH;

// MARK: - XRP Handlers

extern WKAddressHandlers wkAddressHandlersXRP;
extern WKNetworkHandlers wkNetworkHandlersXRP;
extern WKTransferHandlers wkTransferHandlersXRP;
extern WKWalletHandlers wkWalletHandlersXRP;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersXRP;
extern WKWalletManagerHandlers wkWalletManagerHandlersXRP;

// MARK: - HBAR Handlers

extern WKAddressHandlers wkAddressHandlersHBAR;
extern WKNetworkHandlers wkNetworkHandlersHBAR;
extern WKTransferHandlers wkTransferHandlersHBAR;
extern WKWalletHandlers wkWalletHandlersHBAR;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersHBAR;
extern WKWalletManagerHandlers wkWalletManagerHandlersHBAR;

// MARK: - XTZ Handlers

extern WKAddressHandlers wkAddressHandlersXTZ;
extern WKNetworkHandlers wkNetworkHandlersXTZ;
extern WKTransferHandlers wkTransferHandlersXTZ;
extern WKWalletHandlers wkWalletHandlersXTZ;
// Wallet Sweep
// ExportablePaperHandlers
// Payment Protocol
extern WKFeeBasisHandlers wkFeeBasisHandlersXTZ;
extern WKWalletManagerHandlers wkWalletManagerHandlersXTZ;

#ifdef __cplusplus
}
#endif

#endif // WKHandlersExport_h
