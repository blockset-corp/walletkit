//
//  BRCryptoGeneric.c
//  
//
//  Created by Ed Gamble on 4/24/20.
//

#include "BRCryptoGenericP.h"
#include "generic/btc/BRCryptoBTC.h"
//#include "generic/eth/BRCryptoETH.h"
//#include "generic/xrp/BRCryptoXRP.h"

static BRCryptoGenericHandlers handlers[NUMBER_OF_NETWORK_TYPES] = {
    {
        CRYPTO_NETWORK_TYPE_BTC,
        &cryptoNetworkHandlersBTC,
        &cryptoAddressHandlersBTC,
        &cryptoTransferHandlersBTC,
        &cryptoWalletHandlersBTC,
        &cryptoWalletManagerHandlersBTC
    },

    {
         CRYPTO_NETWORK_TYPE_BCH,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    {
         CRYPTO_NETWORK_TYPE_ETH,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    {
         CRYPTO_NETWORK_TYPE_XRP,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    {
         CRYPTO_NETWORK_TYPE_HBAR,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },
};

const BRCryptoGenericHandlers *
cryptoGenericHandlersLookup (BRCryptoBlockChainType type) {
    for (size_t index = 0; index < NUMBER_OF_NETWORK_TYPES; index++)
        if (type == handlers[index].type)
            return &handlers[index];

    // Never here
    assert (0);
    return NULL;
}
