//
//  WKHandlersP.h
//  WalletKitCore
//
//  Created by Ed Gamble on 4/24/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef WKHandlersP_h
#define WKHandlersP_h

#include "WKAddressP.h"
#include "WKNetworkP.h"
#include "WKTransferP.h"
#include "WKWalletP.h"
#include "WKWalletManagerP.h"
#include "WKWalletSweeperP.h"
#include "WKExportablePaperWalletP.h"
#include "WKPaymentP.h"
#include "WKFeeBasisP.h"

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
    WKNetworkType type;
    const WKNetworkHandlers  *network;
    const WKAddressHandlers  *address;
    const WKTransferHandlers *transfer;
    const WKWalletHandlers   *wallet;
    const WKWalletSweeperHandlers *sweeper;                           // NULLable
    const WKExportablePaperWalletHandlers *exportablePaperWallet;     // NULLable
    const WKPaymentProtocolHandlers *payment;
    const WKFeeBasisHandlers *feeBasis;
    const WKWalletManagerHandlers *manager;
} WKHandlers;

extern const WKHandlers *
wkHandlersLookup (WKNetworkType type);

#ifdef __cplusplus
}
#endif

#endif /* WKHandlersP_h */
