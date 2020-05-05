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
    }
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
