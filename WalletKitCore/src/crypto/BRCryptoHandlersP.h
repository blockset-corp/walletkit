//
//  BRCryptoHandlersP.h
//  
//
//  Created by Ed Gamble on 4/24/20.
//

#ifndef BRCryptoHandlersP_h
#define BRCryptoHandlersP_h

#include "BRCryptoAddressP.h"
#include "BRCryptoNetworkP.h"
#include "BRCryptoTransferP.h"
#include "BRCryptoWalletP.h"
#include "BRCryptoWalletManagerP.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Handlers for Built-in Currencies
 *
 * Generally none of these should be NULL; however, for debug purposes, if `network` is NULL then
 * that currency will not be handled and no others handlers will be referenced.
 */
typedef struct {
    BRCryptoBlockChainType type;
    const BRCryptoNetworkHandlers  *network;
    const BRCryptoAddressHandlers  *address;
    const BRCryptoTransferHandlers *transfer;
    const BRCryptoWalletHandlers   *wallet;
    const BRCryptoWalletManagerHandlers *manager;
} BRCryptoHandlers;

extern const BRCryptoHandlers *
cryptoHandlersLookup (BRCryptoBlockChainType type);

#ifdef __cplusplus
}
#endif

#endif /* BRCryptoHandlersP_h */
