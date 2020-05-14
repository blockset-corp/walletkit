
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
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBTC;

extern BRCryptoAddressHandlers cryptoAddressHandlersBCH;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersBCH;
extern BRCryptoTransferHandlers cryptoTransferHandlersBCH;
extern BRCryptoWalletHandlers cryptoWalletHandlersBCH;
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersBCH;

// MARK: - XRP Handlers

extern BRCryptoAddressHandlers cryptoAddressHandlersXRP;
extern BRCryptoNetworkHandlers cryptoNetworkHandlersXRP;
extern BRCryptoTransferHandlers cryptoTransferHandlersXRP;
extern BRCryptoWalletHandlers cryptoWalletHandlersXRP;
extern BRCryptoWalletManagerHandlers cryptoWalletManagerHandlersXRP;

#ifdef __cplusplus
}
#endif

#endif // BRCryptoHandlersExport_h
