//
//  BRCryptoHandlers.c
//  Core
//
//  Created by Ed Gamble on 4/24/20.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#include "BRCryptoHandlersP.h"

// The specific handlers for each supported currency
#include "handlers/BRCryptoHandlersExport.h"

// Must be ordered by BRCryptoBlockChainType enumeration values.
static BRCryptoHandlers handlers[NUMBER_OF_NETWORK_TYPES] = {
    {
        CRYPTO_NETWORK_TYPE_BTC,
        &cryptoNetworkHandlersBTC,
        &cryptoAddressHandlersBTC,
        &cryptoTransferHandlersBTC,
        &cryptoWalletHandlersBTC,
        &cryptoWalletSweeperHandlersBTC,
        &cryptoExportablePaperWalletHandlersBTC,
        &cryptoPaymentProtocolHandlersBTC,
        &cryptoFeeBasisHandlersBTC,
        &cryptoWalletManagerHandlersBTC
    },

    {
        CRYPTO_NETWORK_TYPE_BCH,
        &cryptoNetworkHandlersBCH,
        &cryptoAddressHandlersBCH,
        &cryptoTransferHandlersBCH,
        &cryptoWalletHandlersBCH,
        &cryptoWalletSweeperHandlersBCH,
        NULL,//BRCryptoExportablePaperWalletHandlers
        &cryptoPaymentProtocolHandlersBTC,
        &cryptoFeeBasisHandlersBTC,
        &cryptoWalletManagerHandlersBCH
    },

    {
        CRYPTO_NETWORK_TYPE_BSV,
        &cryptoNetworkHandlersBSV,
        &cryptoAddressHandlersBSV,
        &cryptoTransferHandlersBSV,
        &cryptoWalletHandlersBSV,
        &cryptoWalletSweeperHandlersBSV,
        NULL,//BRCryptoExportablePaperWalletHandlers
        NULL , //BRCryptoPaymentProtocolHandlers
        &cryptoFeeBasisHandlersBTC,
        &cryptoWalletManagerHandlersBSV
    },

    {
        CRYPTO_NETWORK_TYPE_ETH,
        &cryptoNetworkHandlersETH,
        &cryptoAddressHandlersETH,
        &cryptoTransferHandlersETH,
        &cryptoWalletHandlersETH,
        NULL,//BRCryptoWalletSweeperHandlers not supported
        NULL,//BRCryptoExportablePaperWalletHandlers
        NULL,//BRCryptoPaymentProtocolHandlers not supported
        &cryptoFeeBasisHandlersETH,
        &cryptoWalletManagerHandlersETH
    },

    {
        CRYPTO_NETWORK_TYPE_XRP,
        &cryptoNetworkHandlersXRP,
        &cryptoAddressHandlersXRP,
        &cryptoTransferHandlersXRP,
        &cryptoWalletHandlersXRP,
        NULL,//BRCryptoWalletSweeperHandlers not supported
        NULL,//BRCryptoExportablePaperWalletHandlers
        NULL,//BRCryptoPaymentProtocolHandlers not supported
        &cryptoFeeBasisHandlersXRP,
        &cryptoWalletManagerHandlersXRP
    },

    {
        CRYPTO_NETWORK_TYPE_HBAR,
        &cryptoNetworkHandlersHBAR,
        &cryptoAddressHandlersHBAR,
        &cryptoTransferHandlersHBAR,
        &cryptoWalletHandlersHBAR,
        NULL,//BRCryptoWalletSweeperHandlers not supported
        NULL,//BRCryptoExportablePaperWalletHandlers
        NULL,//BRCryptoPaymentProtocolHandlers not supported
        &cryptoFeeBasisHandlersHBAR,
        &cryptoWalletManagerHandlersHBAR
    },
    
    {
        CRYPTO_NETWORK_TYPE_XTZ,
        &cryptoNetworkHandlersXTZ,
        &cryptoAddressHandlersXTZ,
        &cryptoTransferHandlersXTZ,
        &cryptoWalletHandlersXTZ,
        NULL,//BRCryptoWalletSweeperHandlers not supported
        NULL,//BRCryptoExportablePaperWalletHandlers
        NULL,//BRCryptoPaymentProtocolHandlers not supported
        &cryptoFeeBasisHandlersXTZ,
        &cryptoWalletManagerHandlersXTZ
    },
};

extern const BRCryptoHandlers *
cryptoHandlersLookup (BRCryptoBlockChainType type) {
    assert (type < NUMBER_OF_NETWORK_TYPES);
    return &handlers[type];
}
