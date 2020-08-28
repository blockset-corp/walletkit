//
//  BRCryptoHandlersExport.h
//  Core
//
//  Created by Ed Gamble on 05/11/2020.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
#ifndef BRCryptoHandlersExport_h
#define BRCryptoHandlersExport_h

#include "../BRCryptoHandlersP.h"

#ifdef __cplusplus
extern "C" {
#endif

// MARK: - BTC Handlers

extern BRCryptoAddressHandlers cryptoAddressHandlersBTC;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersBTC;
extern BRCryptoTransferHandlers cryptoTransferHandlersBTC;
extern BRCryptoWalletHandlers cryptoWalletHandlersBTC;
extern BRCryptoWalletSweeperHandlers cryptoWalletSweeperHandlersBTC;
extern BRCryptoPaymentProtocolHandlers cryptoPaymentProtocolHandlersBTC;
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC;

// MARK: - BCH Handlers

extern BRCryptoAddressHandlers cryptoAddressHandlersBCH;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersBCH;
extern BRCryptoTransferHandlers cryptoTransferHandlersBCH;
extern BRCryptoWalletHandlers cryptoWalletHandlersBCH;
extern BRCryptoWalletSweeperHandlers cryptoWalletSweeperHandlersBCH;
// Payment Protocol
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBCH;

// MARK: - BSV Handlers

extern BRCryptoAddressHandlers cryptoAddressHandlersBSV;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersBSV;
extern BRCryptoTransferHandlers cryptoTransferHandlersBSV;
extern BRCryptoWalletHandlers cryptoWalletHandlersBSV;
extern BRCryptoWalletSweeperHandlers cryptoWalletSweeperHandlersBSV;
// Payment Protocol
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBSV;

// MARK: - ETH Handlers

extern BRCryptoAddressHandlers cryptoAddressHandlersETH;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersETH;
extern BRCryptoTransferHandlers cryptoTransferHandlersETH;
extern BRCryptoWalletHandlers cryptoWalletHandlersETH;
// Wallet Sweep
// Payment Protocol
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersETH;

// MARK: - XRP Handlers

extern BRCryptoAddressHandlers cryptoAddressHandlersXRP;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersXRP;
extern BRCryptoTransferHandlers cryptoTransferHandlersXRP;
extern BRCryptoWalletHandlers cryptoWalletHandlersXRP;
// Wallet Sweep
// Payment Protocol
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersXRP;

// MARK: - HBAR Handlers

extern BRCryptoAddressHandlers cryptoAddressHandlersHBAR;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersHBAR;
extern BRCryptoTransferHandlers cryptoTransferHandlersHBAR;
extern BRCryptoWalletHandlers cryptoWalletHandlersHBAR;
// Wallet Sweep
// Payment Protocol
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersHBAR;

// MARK: - XTZ Handlers

extern BRCryptoAddressHandlers cryptoAddressHandlersXTZ;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersXTZ;
extern BRCryptoTransferHandlers cryptoTransferHandlersXTZ;
extern BRCryptoWalletHandlers cryptoWalletHandlersXTZ;
// Wallet Sweep
// Payment Protocol
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersXTZ;

#ifdef __cplusplus
}
#endif

#endif // BRCryptoHandlersExport_h
