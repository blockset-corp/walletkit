
//
//  BRCryptoExportablePaperWalletBTC.c
//  BRCore
//
//  Created by Ed Gamble on 2/24/21
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoBTC.h"
#include "crypto/BRCryptoExportablePaperWalletP.h"
#include "crypto/BRCryptoKeyP.h"


static BRCryptoExportablePaperWalletStatus
cryptoExportablePaperWalletValidateSupporteBTC (BRCryptoNetwork network,
                                                BRCryptoCurrency currency) {
    return CRYPTO_EXPORTABLE_PAPER_WALLET_SUCCESS;
}

static BRCryptoAddress
cryptoExportablePaperWalletCreateAddressBTC (BRCryptoNetwork network,
                                             BRCryptoKey key) {
    BRAddressParams addrParamsBTC = cryptoNetworkAsBTC (network)->addrParams;
    BRKey          *keyBTC        = cryptoKeyGetCore (key);

    // encode using legacy format (only supported method for BTC)
    size_t addrLength = BRKeyLegacyAddr (keyBTC, NULL, 0, addrParamsBTC);

    char addr [addrLength + 1];
    BRKeyLegacyAddr (keyBTC, addr, addrLength, addrParamsBTC);
    addr[addrLength] = '\0';

    return cryptoAddressCreateFromStringAsBTC (addrParamsBTC, addr);
}

static BRCryptoExportablePaperWallet
cryptoExportablePaperWalletCreateBTC (BRCryptoNetwork network,
                                      BRCryptoCurrency currency) {
    BRKey keyBTC;
    if (1 != BRKeyGenerateRandom (&keyBTC, 1)) return NULL;

    BRCryptoKey     key     = cryptoKeyCreateFromKey (&keyBTC);
    BRCryptoAddress address = cryptoExportablePaperWalletCreateAddressBTC (network, key);

    BRCryptoExportablePaperWallet wallet =
    cryptoExportablePaperWalletAllocAndInit (sizeof (struct BRCryptoExportablePaperWalletRecord),
                                             network->type,
                                             network,
                                             address,
                                             key);

    cryptoAddressGive (address);
    cryptoKeyGive     (key);

    return wallet;
}

BRCryptoExportablePaperWalletHandlers cryptoExportablePaperWalletHandlersBTC = {
    cryptoExportablePaperWalletCreateBTC,
    NULL,
    cryptoExportablePaperWalletValidateSupporteBTC
};

// BRCryptoExportablePaperWalletHandlers cryptoExportablePaperWalletHandlersBCH;
// BRCryptoExportablePaperWalletHandlers cryptoExportablePaperWalletHandlersBSV;
