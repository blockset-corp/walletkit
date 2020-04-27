//
//  BRCryptoGenericP.h
//  
//
//  Created by Ed Gamble on 4/24/20.
//

#ifndef BRCryptoGenericP_h
#define BRCryptoGenericP_h

#include "BRCryptoAddressP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoWalletP.h"
#include "BRCryptoWalletManagerP.h"

typedef struct {
    BRCryptoBlockChainType type;
    const BRCryptoNetworkHandlers  *network;
    const BRCryptoAddressHandlers  *address;
    const BRCryptoTransferHandlers *transfer;
    const BRCryptoWalletHandlers   *wallet;
    const BRCryptoWalletManagerHandlers *manager;
} BRCryptoGenericHandlers;

const BRCryptoGenericHandlers *
cryptoGenericHandlersLookup (BRCryptoBlockChainType type);

#endif /* BRCryptoGenericP_h */
