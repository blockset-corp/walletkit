/*
 * Created by Ehsan Rezaie <ehsan@brd.com> on 11/23/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKCurrency;
import com.blockset.walletkit.nativex.WKExportablePaperWallet;
import com.blockset.walletkit.nativex.WKExportablePaperWalletStatus;
import com.blockset.walletkit.nativex.WKNetwork;
import com.blockset.walletkit.errors.ExportablePaperWalletError;
import com.blockset.walletkit.errors.ExportablePaperWalletUnexpectedError;
import com.blockset.walletkit.errors.ExportablePaperWalletUnsupportedCurrencyError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;


final class ExportablePaperWallet implements com.blockset.walletkit.ExportablePaperWallet {
    /* package */
    static void create(WalletManager manager,
                       CompletionHandler<com.blockset.walletkit.ExportablePaperWallet, ExportablePaperWalletError> completion) {
        // check that the requested operation is supported
        ExportablePaperWalletError e = ExportablePaperWallet.validateSupported(manager);
        if (null != e) {
            completion.handleError(e);
            return;
        }

        Optional<WKExportablePaperWallet> paperWallet = WKExportablePaperWallet.create (
                manager.getNetwork().getCoreBRCryptoNetwork(),
                manager.getCurrency().getCoreBRCryptoCurrency());

        if (paperWallet.isPresent())
            completion.handleData(ExportablePaperWallet.create(paperWallet.get()));
        else
            completion.handleError(ExportablePaperWallet.statusToError(
                    WKExportablePaperWalletStatus.ILLEGAL_OPERATION));
    }

    private static ExportablePaperWalletError statusToError(WKExportablePaperWalletStatus status) {
        switch (status) {
            case SUCCESS: return null;
            case UNSUPPORTED_CURRENCY: return new ExportablePaperWalletUnsupportedCurrencyError();
            case INVALID_ARGUMENTS: return new ExportablePaperWalletUnexpectedError("Invalid argument");
            case ILLEGAL_OPERATION: return new ExportablePaperWalletUnexpectedError("Illegal operation");
        }
        return null;
    }

    private static ExportablePaperWalletError validateSupported(WalletManager manager) {
        Network network = manager.getNetwork();
        Currency currency = manager.getCurrency();

        WKNetwork coreNetwork = network.getCoreBRCryptoNetwork();
        WKCurrency coreCurrency = currency.getCoreBRCryptoCurrency();

        return ExportablePaperWallet.statusToError(WKExportablePaperWallet.validateSupported(coreNetwork, coreCurrency));
    }

    private static ExportablePaperWallet create(WKExportablePaperWallet core) {
        ExportablePaperWallet paperWallet = new ExportablePaperWallet(core);
        ReferenceCleaner.register(paperWallet, core::give);
        return paperWallet;
    }

    private final WKExportablePaperWallet core;

    private ExportablePaperWallet(WKExportablePaperWallet core) {
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
