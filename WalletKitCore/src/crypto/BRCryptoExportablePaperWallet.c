//
//  BRCryptoExportablePaperWallet.c
//  BRCore
//
//  Created by Ed Gamble on 2/23/21
//  Copyright © 2020 breadwallet. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include "BRCryptoExportablePaperWalletP.h"
#include "BRCryptoHandlersP.h"

private_extern BRCryptoExportablePaperWallet
cryptoExportablePaperWalletAllocAndInit (size_t sizeInBytes,
                                         BRCryptoBlockChainType type,
                                         BRCryptoNetwork network,
                                         BRCryptoAddress address,
                                         BRCryptoKey key) {
    assert (sizeInBytes >= sizeof (struct BRCryptoExportablePaperWalletRecord));
    assert (cryptoKeyHasSecret (key));

    BRCryptoExportablePaperWallet wallet = calloc (1, sizeInBytes);

    wallet->type = type;
    wallet->sizeInBytes = sizeInBytes;
    wallet->handlers = cryptoHandlersLookup(type)->exportablePaperWallet;

    wallet->network  = cryptoNetworkTake (network);
    wallet->address  = cryptoAddressTake (address);
    wallet->key      = cryptoKeyTake     (key);

    return wallet;
}

private_extern void
cryptoExportablePaperWalletReleaseInternal (BRCryptoExportablePaperWallet wallet) {
    cryptoNetworkGive (wallet->network);
    cryptoAddressGive (wallet->address);
    cryptoKeyGive     (wallet->key);

    memset (wallet, 0, wallet->sizeInBytes);
    free (wallet);
}

extern BRCryptoExportablePaperWalletStatus
cryptoExportablePaperWalletValidateSupported (BRCryptoNetwork network,
                                              BRCryptoCurrency currency) {
    if (CRYPTO_FALSE == cryptoNetworkHasCurrency (network, currency))
        return CRYPTO_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS;

    const BRCryptoHandlers *handlers = cryptoHandlersLookup (network->type);

    return ((NULL != handlers->exportablePaperWallet &&
             NULL != handlers->exportablePaperWallet->validateSupported)
            ? handlers->exportablePaperWallet->validateSupported (network, currency)
            : CRYPTO_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION);
}

extern BRCryptoExportablePaperWallet
cryptoExportablePaperWalletCreate (BRCryptoNetwork network,
                                   BRCryptoCurrency currency) {
    const BRCryptoHandlers *handlers = cryptoHandlersLookup (network->type);

    return ((NULL != handlers->exportablePaperWallet &&
             NULL != handlers->exportablePaperWallet->create)
            ? handlers->exportablePaperWallet->create (network, currency)
            : NULL);
}

extern void
cryptoExportablePaperWalletRelease (BRCryptoExportablePaperWallet wallet) {
    if (NULL != wallet->handlers->release)
        wallet->handlers->release (wallet);
    else
        cryptoExportablePaperWalletReleaseInternal (wallet);
}

extern BRCryptoKey
cryptoExportablePaperWalletGetKey (BRCryptoExportablePaperWallet wallet) {
    return cryptoKeyTake (wallet->key);
}

extern BRCryptoAddress
cryptoExportablePaperWalletGetAddress (BRCryptoExportablePaperWallet wallet) {
    return cryptoAddressTake (wallet->address);
}
