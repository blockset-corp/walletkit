/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.TransferFeeBasis;
import com.blockset.walletkit.errors.QueryError;
import com.blockset.walletkit.nativex.WKClientTransactionBundle;
import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKAddress;
import com.blockset.walletkit.nativex.WKKey;
import com.blockset.walletkit.nativex.WKWallet;
import com.blockset.walletkit.nativex.WKWalletManager;
import com.blockset.walletkit.nativex.WKWalletSweeper;
import com.blockset.walletkit.nativex.WKWalletSweeperStatus;
import com.blockset.walletkit.NetworkFee;
import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.systemclient.Transaction;
import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.errors.WalletSweeperError;
import com.blockset.walletkit.errors.WalletSweeperInsufficientFundsError;
import com.blockset.walletkit.errors.WalletSweeperInvalidKeyError;
import com.blockset.walletkit.errors.WalletSweeperInvalidSourceWalletError;
import com.blockset.walletkit.errors.WalletSweeperNoTransfersFoundError;
import com.blockset.walletkit.errors.WalletSweeperQueryError;
import com.blockset.walletkit.errors.WalletSweeperUnableToSweepError;
import com.blockset.walletkit.errors.WalletSweeperUnexpectedError;
import com.blockset.walletkit.errors.WalletSweeperUnsupportedCurrencyError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.logging.Level;

import static com.google.common.base.Preconditions.checkState;

/* package */
final class WalletSweeper implements com.blockset.walletkit.WalletSweeper {

    /* package */
    static void create(WalletManager manager,
                       Wallet wallet,
                       Key key,
                       SystemClient sc,
                       CompletionHandler<com.blockset.walletkit.WalletSweeper, WalletSweeperError> completion) {
        // check that the requested operation is supported
        WalletSweeperError e = WalletSweeper.validateSupported(manager, wallet, key);
        if (null != e) {
            completion.handleError(e);
            return;
        }

        // Well, this is implied by 'validateSupported', I think.  And, there is
        // no other 'createAsXYZ' function.  When there is, we'll need something more.
        WalletSweeper.createAsBtc(manager, wallet, key).initAsBtc(sc, completion);
    }

    private static WalletSweeperError statusToError(WKWalletSweeperStatus status) {
        switch (status) {
            case SUCCESS: return null;
            case UNSUPPORTED_CURRENCY: return new WalletSweeperUnsupportedCurrencyError();

            case INVALID_KEY: return new WalletSweeperInvalidKeyError();
            case INVALID_SOURCE_WALLET: return new WalletSweeperInvalidSourceWalletError();
            case INSUFFICIENT_FUNDS: return new WalletSweeperInsufficientFundsError();
            case UNABLE_TO_SWEEP: return new WalletSweeperUnableToSweepError();
            case NO_TRANSFERS_FOUND: return new WalletSweeperNoTransfersFoundError();

            case INVALID_ARGUMENTS: return new WalletSweeperUnexpectedError("Invalid argument");
            case INVALID_TRANSACTION: return new WalletSweeperUnexpectedError("Invalid transaction");

            case ILLEGAL_OPERATION: return new WalletSweeperUnexpectedError("Illegal operation");

        }
        return null;
    }

    private static WalletSweeperError validateSupported(WalletManager manager,
                                                        Wallet wallet,
                                                        Key key) {
        WKKey coreKey = key.getBRCryptoKey();
        WKWallet coreWallet = wallet.getCoreBRCryptoWallet();
        WKWalletManager coreManager = manager.getCoreBRCryptoWalletManager();

        return WalletSweeper.statusToError(WKWalletSweeper.validateSupported(coreManager, coreWallet, coreKey));
    }

    private static WalletSweeper createAsBtc(WalletManager manager,
                                             Wallet wallet,
                                             Key key) {
        WKKey coreKey = key.getBRCryptoKey();
        WKWallet coreWallet = wallet.getCoreBRCryptoWallet();
        WKWalletManager coreManager = manager.getCoreBRCryptoWalletManager();

        WKWalletSweeper core = WKWalletSweeper.createAsBtc(coreManager, coreWallet, coreKey);
        return WalletSweeper.create(core, manager, wallet);
    }

    private static WalletSweeper create(WKWalletSweeper core, WalletManager manager, Wallet wallet) {
        WalletSweeper sweeper = new WalletSweeper(core, manager, wallet);
        ReferenceCleaner.register(sweeper, core::give);
        return sweeper;
    }

    private final WKWalletSweeper core;
    private final WalletManager manager;
    private final Wallet wallet;

    private WalletSweeper(WKWalletSweeper core, WalletManager manager, Wallet wallet) {
        this.core = core;
        this.manager = manager;
        this.wallet = wallet;
    }

    @Override
    public Optional<Amount> getBalance() {
        return core.getBalance().transform(Amount::create);
    }

    @Override
    public void estimate(NetworkFee fee,
                         CompletionHandler<TransferFeeBasis, FeeEstimationError> completion) {
        wallet.estimateFee(this, fee, completion);
    }

    @Override
    public Optional<Transfer> submit(TransferFeeBasis feeBasis) {
        Optional<Transfer> maybeTransfer = wallet.createTransfer(this, feeBasis);
        if (maybeTransfer.isPresent()) {
            Transfer transfer = maybeTransfer.get();
            Key key = Key.create(core.getKey());
            manager.submit(transfer, key);
        }
        return maybeTransfer;
    }

    /* package */
    WKWalletSweeper getCoreBRWalletSweeper() {
        return core;
    }

    private void initAsBtc(SystemClient sc,
                           CompletionHandler<com.blockset.walletkit.WalletSweeper, WalletSweeperError> completion) {
        Network network = manager.getNetwork();

        sc.getTransactions(
                network.getUids(),
                Collections.singletonList(getAddress()),
                UnsignedLong.ZERO,
                network.getHeight(),
                true,
                false,
                false,
                null,
                new CompletionHandler<List<Transaction>, QueryError>() {

                    @Override
                    public void handleData(List<Transaction> transactions) {
                        WalletSweeperError error = null;

                        // Collect all the bundles derived from transactions
                        List<WKClientTransactionBundle> bundles = new ArrayList<>();

                        for (Transaction transaction : transactions) {
                            System.makeTransactionBundle (transaction)
                                    .transform (bundles::add);
                        }

                        for (WKClientTransactionBundle bundle : bundles) {
                            error = handleTransactionAsBtc(bundle);
                            if (null != error) break /* for */ ;
                        }

                        // Release all the bundles
                        for (WKClientTransactionBundle bundle : bundles) {
                            bundle.release();
                        }

                        // If no error, then validate
                        if (null == error) error = validate();

                        if (null != error) completion.handleError(error);
                        else completion.handleData(WalletSweeper.this);
                    }

                    @Override
                    public void handleError(QueryError e) {
                        completion.handleError(new WalletSweeperQueryError(e));
                    }
                });
    }

    private String getAddress() {
        Optional<String> maybeAddress = core.getAddress().transform(WKAddress::toString);
        checkState(maybeAddress.isPresent());
        return maybeAddress.get();
    }

    private WalletSweeperError handleTransactionAsBtc(WKClientTransactionBundle bundle) {
        return statusToError(core.handleTransactionAsBtc(bundle));
    }

    private WalletSweeperError validate() {
        return statusToError(core.validate());
    }
}
