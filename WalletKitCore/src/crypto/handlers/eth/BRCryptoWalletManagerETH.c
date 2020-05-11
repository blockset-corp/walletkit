#include "BRCryptoBTC.h"

#include "../../BRCryptoAccountP.h"
#include "../../BRCryptoNetworkP.h"
#include "../../BRCryptoKeyP.h"
#include "../../BRCryptoClientP.h"
#include "../../BRCryptoWalletManagerP.h"

// #include "bitcoin/BRWallet.h"
// #include "bitcoin/BRPeerManager.h"

static void
cryptoWalletManagerInstallETHTokensForCurrencies (BRCryptoWalletManager cwm) {
    BRCryptoNetwork  network    = cryptoNetworkTake (cwm->network);
    BRCryptoCurrency currency   = cryptoNetworkGetCurrency(network);
    BRCryptoUnit     unitForFee = cryptoNetworkGetUnitAsBase (network, currency);

    size_t currencyCount = cryptoNetworkGetCurrencyCount (network);
    for (size_t index = 0; index < currencyCount; index++) {
        BRCryptoCurrency c = cryptoNetworkGetCurrencyAt (network, index);
        if (c != currency) {
            BRCryptoUnit unitDefault = cryptoNetworkGetUnitAsDefault (network, c);

#ifdef REFACTOR
            switch (cwm->type) {
                case BLOCK_CHAIN_TYPE_BTC:
                    break;
                case BLOCK_CHAIN_TYPE_ETH: {
                    const char *address = cryptoCurrencyGetIssuer(c);
                    if (NULL != address) {
                        BREthereumGas      ethGasLimit = ethGasCreate(TOKEN_BRD_DEFAULT_GAS_LIMIT);
                        BREthereumGasPrice ethGasPrice = ethGasPriceCreate(ethEtherCreate(uint256Create(TOKEN_BRD_DEFAULT_GAS_PRICE_IN_WEI_UINT64)));

                        // This has the perhaps surprising side-effect of updating the properties
                        // of an existing token.  That is, `address` is used to locate a token and
                        // if found it is updated.  Either created or updated the token will be
                        // persistently saved.
                        //
                        // Argubably EWM should create a wallet for the token.  But, it doesn't.
                        // So we'll call `ewmGetWalletHoldingToken()` to get a wallet.

                        BREthereumToken token = ewmCreateToken (cwm->u.eth,
                                                                address,
                                                                cryptoCurrencyGetCode (c),
                                                                cryptoCurrencyGetName(c),
                                                                cryptoCurrencyGetUids(c), // description
                                                                cryptoUnitGetBaseDecimalOffset(unitDefault),
                                                                ethGasLimit,
                                                                ethGasPrice);
                        assert (NULL != token); (void) &token;
                    }
                    break;
                }
                case BLOCK_CHAIN_TYPE_GEN:
                    break;
            }
#endif
            cryptoUnitGive(unitDefault);
        }
        cryptoCurrencyGive(c);
    }
    cryptoUnitGive(unitForFee);
    cryptoCurrencyGive(currency);
    cryptoNetworkGive(network);
}


static void
cryptoWalletManagerEstimateFeeBasisHandlerETH (BRCryptoWalletManager cwm,
                                               BRCryptoWallet  wallet,
                                               BRCryptoCookie cookie,
                                               BRCryptoAddress target,
                                               BRCryptoAmount amount,
                                               BRCryptoNetworkFee networkFee) {
    BRCryptoTransfer transfer = ...;

    cryptoClientQRYEstimateTransferFee (cwm->qryManager, cookie, transfer, networkFee);
    return;
}
