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
import com.breadwallet.crypto.explore.ExploreConstants;

import java.util.logging.ConsoleHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.annotation.Nullable;

public class ExploreWalletManagerEventHandler<Void> extends DefaultWalletManagerEventVisitor<Void>  {

    private final static Logger Log;
    static {
        Log = Logger.getLogger(ExploreConstants.ExploreTag
                               + ExploreWalletManagerEventHandler.class.getName());
    }

    public ExploreWalletManagerEventHandler() {
        super();
    }

    @Nullable
    @Override
    public Void visit(WalletManagerBlockUpdatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager block updated");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerChangedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerCreatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager created");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerDeletedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager deleted");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerSyncProgressEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager sync progres...");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerSyncStartedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager sync started....");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerSyncStoppedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager sync stopped");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerSyncRecommendedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager sync advised");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerWalletAddedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager, wallet is added");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerWalletChangedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager, wallet is changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletManagerWalletDeletedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Manager, wallet is deleted");
        return super.visit(event);
    }
}
