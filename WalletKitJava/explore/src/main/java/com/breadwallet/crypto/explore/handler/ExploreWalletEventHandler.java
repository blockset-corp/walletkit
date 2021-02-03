package com.breadwallet.crypto.explore.handler;

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
import com.breadwallet.crypto.explore.ExploreConstants;

import java.util.logging.ConsoleHandler;
import java.util.logging.Level;
import java.util.logging.Logger;

import javax.annotation.Nullable;

public class ExploreWalletEventHandler<Void> extends DefaultWalletEventVisitor<Void> {

    private final static Logger Log;
    static {
        Log = Logger.getLogger(ExploreConstants.ExploreTag
                               + ExploreWalletEventHandler.class.getName());
    }

    public ExploreWalletEventHandler() {
        super();
    }

    @Nullable
    @Override
    public Void visit(WalletBalanceUpdatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Wallet balance updated");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletChangedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Wallet changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletCreatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Wallet created");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletDeletedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Wallet deleted");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletFeeBasisUpdatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Wallet fee basis changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletTransferAddedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Wallet transfer added");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletTransferChangedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Transfer changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletTransferDeletedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Transfer changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(WalletTransferSubmittedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Transfer changed");
        return super.visit(event);
    }
}
