package com.breadwallet.crypto.explore.handler;

import com.breadwallet.crypto.Wallet;
import com.breadwallet.crypto.events.wallet.DefaultWalletEventVisitor;
import com.breadwallet.crypto.events.wallet.WalletBalanceUpdatedEvent;
import com.breadwallet.crypto.events.wallet.WalletChangedEvent;
import com.breadwallet.crypto.events.wallet.WalletCreatedEvent;
import com.breadwallet.crypto.events.wallet.WalletDeletedEvent;
import com.breadwallet.crypto.events.wallet.WalletFeeBasisUpdatedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferAddedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferChangedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferDeletedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferSubmittedEvent;

import java.util.logging.Level;
import java.util.logging.Logger;

import javax.annotation.Nullable;

public class ExploreWalletEventHandler<Void> extends DefaultWalletEventVisitor<Void> {

    private final Logger logger;
    private final Wallet wallet;

    public ExploreWalletEventHandler(Wallet wallet, Logger logger) {
        super();
        this.wallet = wallet;
        this.logger = logger;
    }

    @Nullable
    @Override
    public Void visit(WalletBalanceUpdatedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Wallet balance updated");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletChangedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Wallet changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletCreatedEvent event) {
        logger.log(Level.INFO, String.format("Wallet %s created",
                                             wallet.getTarget()));
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletDeletedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Wallet deleted");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletFeeBasisUpdatedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Wallet fee basis changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletTransferAddedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Wallet transfer added");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletTransferChangedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Transfer changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletTransferDeletedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Transfer changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletTransferSubmittedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Transfer changed");
        return super.visit(event);
    }
}
