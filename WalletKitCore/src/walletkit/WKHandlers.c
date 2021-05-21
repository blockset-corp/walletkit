//
//  WKHandlers.c
//  WalletKitCore
//
//  Created by Ed Gamble on 4/24/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "WKHandlersP.h"

// The specific handlers for each supported currency
#include "handlers/WKHandlersExport.h"

// Must be ordered by WKBlockChainType enumeration values.
static WKHandlers handlers[NUMBER_OF_NETWORK_TYPES] = {
    {
        WK_NETWORK_TYPE_BTC,
        &wkNetworkHandlersBTC,
        &wkAddressHandlersBTC,
        &wkTransferHandlersBTC,
        &wkWalletHandlersBTC,
        &wkWalletSweeperHandlersBTC,
        &wkExportablePaperWalletHandlersBTC,
        &wkPaymentProtocolHandlersBTC,
        &wkFeeBasisHandlersBTC,
        &wkWalletManagerHandlersBTC
    },

    {
        WK_NETWORK_TYPE_BCH,
        &wkNetworkHandlersBCH,
        &wkAddressHandlersBCH,
        &wkTransferHandlersBCH,
        &wkWalletHandlersBCH,
        &wkWalletSweeperHandlersBCH,
        NULL,//WKExportablePaperWalletHandlers
        &wkPaymentProtocolHandlersBTC,
        &wkFeeBasisHandlersBTC,
        &wkWalletManagerHandlersBCH
    },

    {
        WK_NETWORK_TYPE_BSV,
        &wkNetworkHandlersBSV,
        &wkAddressHandlersBSV,
        &wkTransferHandlersBSV,
        &wkWalletHandlersBSV,
        &wkWalletSweeperHandlersBSV,
        NULL,//WKExportablePaperWalletHandlers
        &wkPaymentProtocolHandlersBTC,
        &wkFeeBasisHandlersBTC,
        &wkWalletManagerHandlersBSV
    },

    {
        WK_NETWORK_TYPE_LTC,
        &wkNetworkHandlersLTC,
        &wkAddressHandlersLTC,
        &wkTransferHandlersLTC,
        &wkWalletHandlersLTC,
        &wkWalletSweeperHandlersLTC,
        NULL,//WKExportablePaperWalletHandlers
        &wkPaymentProtocolHandlersBTC,
        &wkFeeBasisHandlersBTC,
        &wkWalletManagerHandlersLTC
    },

    {
        WK_NETWORK_TYPE_ETH,
        &wkNetworkHandlersETH,
        &wkAddressHandlersETH,
        &wkTransferHandlersETH,
        &wkWalletHandlersETH,
        NULL,//WKWalletSweeperHandlers not supported
        NULL,//WKExportablePaperWalletHandlers
        NULL,//WKPaymentProtocolHandlers not supported
        &wkFeeBasisHandlersETH,
        &wkWalletManagerHandlersETH
    },

    {
        WK_NETWORK_TYPE_XRP,
        &wkNetworkHandlersXRP,
        &wkAddressHandlersXRP,
        &wkTransferHandlersXRP,
        &wkWalletHandlersXRP,
        NULL,//WKWalletSweeperHandlers not supported
        NULL,//WKExportablePaperWalletHandlers
        NULL,//WKPaymentProtocolHandlers not supported
        &wkFeeBasisHandlersXRP,
        &wkWalletManagerHandlersXRP
    },

    {
        WK_NETWORK_TYPE_HBAR,
        &wkNetworkHandlersHBAR,
        &wkAddressHandlersHBAR,
        &wkTransferHandlersHBAR,
        &wkWalletHandlersHBAR,
        NULL,//WKWalletSweeperHandlers not supported
        NULL,//WKExportablePaperWalletHandlers
        NULL,//WKPaymentProtocolHandlers not supported
        &wkFeeBasisHandlersHBAR,
        &wkWalletManagerHandlersHBAR
    },
    
    {
        WK_NETWORK_TYPE_XTZ,
        &wkNetworkHandlersXTZ,
        &wkAddressHandlersXTZ,
        &wkTransferHandlersXTZ,
        &wkWalletHandlersXTZ,
        NULL,//WKWalletSweeperHandlers not supported
        NULL,//WKExportablePaperWalletHandlers
        NULL,//WKPaymentProtocolHandlers not supported
        &wkFeeBasisHandlersXTZ,
        &wkWalletManagerHandlersXTZ
    },

    {
        WK_NETWORK_TYPE_XLM,
        &wkNetworkHandlersXLM,
        &wkAddressHandlersXLM,
        &wkTransferHandlersXLM,
        &wkWalletHandlersXLM,
        NULL,//WKWalletSweeperHandlers not supported
        NULL,//WKExportablePaperWalletHandlers
        NULL,//WKPaymentProtocolHandlers not supported
        &wkFeeBasisHandlersXLM,
        &wkWalletManagerHandlersXLM
    },
};

extern const WKHandlers *
wkHandlersLookup (WKNetworkType type) {
    assert (type < NUMBER_OF_NETWORK_TYPES);
    return &handlers[type];
}
