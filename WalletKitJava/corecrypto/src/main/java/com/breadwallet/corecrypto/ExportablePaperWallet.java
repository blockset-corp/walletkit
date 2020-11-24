/*
 * Created by Ehsan Rezaie <ehsan@brd.com> on 11/23/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corecrypto;

import com.breadwallet.corenative.cleaner.ReferenceCleaner;
import com.breadwallet.corenative.crypto.BRCryptoCurrency;
import com.breadwallet.corenative.crypto.BRCryptoExportablePaperWallet;
import com.breadwallet.corenative.crypto.BRCryptoExportablePaperWalletStatus;
import com.breadwallet.corenative.crypto.BRCryptoNetwork;
import com.breadwallet.corenative.crypto.BRCryptoWallet;
import com.breadwallet.crypto.blockchaindb.BlockchainDb;
import com.breadwallet.crypto.errors.ExportablePaperWalletError;
import com.breadwallet.crypto.errors.ExportablePaperWalletUnexpectedError;
import com.breadwallet.crypto.errors.ExportablePaperWalletUnsupportedCurrencyError;
import com.breadwallet.crypto.utility.CompletionHandler;
import com.google.common.base.Optional;


final class ExportablePaperWallet implements com.breadwallet.crypto.ExportablePaperWallet {
    /* package */
    static void create(WalletManager manager,
                                        Wallet wallet,
                                        BlockchainDb bdb,
                                        CompletionHandler<ExportablePaperWallet, ExportablePaperWalletError> completion) {
        // check that the requested operation is supported
        ExportablePaperWalletError e = ExportablePaperWallet.validateSupported(manager, wallet);
        if (null != e) {
            completion.handleError(e);
            return;
        }

        ExportablePaperWallet paperWallet = ExportablePaperWallet.createAsBTC(manager, wallet);
        completion.handleData(paperWallet);
    }

    private static ExportablePaperWalletError statusToError(BRCryptoExportablePaperWalletStatus status) {
        switch (status) {
            case CRYPTO_EXPORTABLE_PAPER_WALLET_SUCCESS: return null;
            case CRYPTO_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY: return new ExportablePaperWalletUnsupportedCurrencyError();
            case CRYPTO_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS: return new ExportablePaperWalletUnexpectedError("Invalid argument");
            case CRYPTO_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION: return new ExportablePaperWalletUnexpectedError("Illegal operation");
        }
        return null;
    }

    private static ExportablePaperWalletError validateSupported(WalletManager manager,
                                                                Wallet wallet) {
        Network network = manager.getNetwork();
        Currency currency = wallet.getCurrency();

        BRCryptoWallet coreWallet = wallet.getCoreBRCryptoWallet();
        BRCryptoNetwork coreNetwork = network.getCoreBRCryptoNetwork();
        BRCryptoCurrency coreCurrency = currency.getCoreBRCryptoCurrency();

        return ExportablePaperWallet.statusToError(BRCryptoExportablePaperWallet.validateSupported(coreNetwork, coreCurrency, coreWallet));
    }

    private static ExportablePaperWallet createAsBTC(WalletManager manager,
                                                     Wallet wallet) {
        Network network = manager.getNetwork();
        Currency currency = wallet.getCurrency();

        BRCryptoNetwork coreNetwork = network.getCoreBRCryptoNetwork();
        BRCryptoCurrency coreCurrency = currency.getCoreBRCryptoCurrency();

        BRCryptoExportablePaperWallet core = BRCryptoExportablePaperWallet.createAsBTC(coreNetwork, coreCurrency);
        return ExportablePaperWallet.create(core, manager, wallet);
    }

    private static ExportablePaperWallet create(BRCryptoExportablePaperWallet core, WalletManager manager, Wallet wallet) {
        ExportablePaperWallet paperWallet = new ExportablePaperWallet(core);
        ReferenceCleaner.register(paperWallet, core::give);
        return paperWallet;
    }

    private final BRCryptoExportablePaperWallet core;

    private ExportablePaperWallet(BRCryptoExportablePaperWallet core) {
        this.core = core;
    }

    @Override
    public Optional<Key> getKey() {
        return core.getKey().transform(Key::create);
    }

    @Override
    public Optional<Address> getAddress() {
        return core.getAddress().transform(Address::create);
    }
}
