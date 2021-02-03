package com.breadwallet.crypto.explore.handler;

import com.breadwallet.crypto.events.walletmanager.DefaultWalletManagerEventVisitor;
import com.breadwallet.crypto.events.walletmanager.WalletManagerBlockUpdatedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerChangedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerCreatedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerDeletedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncProgressEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncRecommendedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncStartedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncStoppedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletAddedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletChangedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletDeletedEvent;

import java.util.logging.Level;
import java.util.logging.Logger;

import javax.annotation.Nullable;


public class ExploreWalletManagerEventHandler<Void> extends DefaultWalletManagerEventVisitor<Void>  {

    private final Logger logger;

    public ExploreWalletManagerEventHandler(Logger logger) {
        super();
        this.logger = logger;
    }

    @Nullable
    @Override
    public Void visit(WalletManagerBlockUpdatedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager block updated");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerChangedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerCreatedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager created");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerDeletedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager deleted");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerSyncProgressEvent event) {
        logger.log(Level.FINE, "[todo-ev] Manager sync progres...");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerSyncStartedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager sync started....");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerSyncStoppedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager sync stopped");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerSyncRecommendedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager sync advised");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerWalletAddedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager, wallet is added");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerWalletChangedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager, wallet is changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerWalletDeletedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Manager, wallet is deleted");
        return super.visit(event);
    }
}
