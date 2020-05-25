//
//  BRCryptoGeneric.c
//  
//
//  Created by Ed Gamble on 4/24/20.
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
        &cryptoWalletManagerHandlersBTC
    },

    {
        CRYPTO_NETWORK_TYPE_BCH,
        &cryptoNetworkHandlersBCH,
        &cryptoAddressHandlersBCH,
        &cryptoTransferHandlersBCH,
        &cryptoWalletHandlersBCH,
        &cryptoWalletSweeperHandlersBCH,
        &cryptoWalletManagerHandlersBCH
    },

    {
         CRYPTO_NETWORK_TYPE_ETH,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    {
        CRYPTO_NETWORK_TYPE_XRP,
        &cryptoNetworkHandlersXRP,
        &cryptoAddressHandlersXRP,
        &cryptoTransferHandlersXRP,
        &cryptoWalletHandlersXRP,
        NULL,//BRCryptoWalletSweeperHandlers not supported
        &cryptoWalletManagerHandlersXRP
    },

    {
        CRYPTO_NETWORK_TYPE_HBAR,
        &cryptoNetworkHandlersHBAR,
        &cryptoAddressHandlersHBAR,
        &cryptoTransferHandlersHBAR,
        &cryptoWalletHandlersHBAR,
        NULL,//BRCryptoWalletSweeperHandlers not supported
        &cryptoWalletManagerHandlersHBAR
    },
};

extern const BRCryptoHandlers *
cryptoHandlersLookup (BRCryptoBlockChainType type) {
    assert (type < NUMBER_OF_NETWORK_TYPES);
    return &handlers[type];
}
