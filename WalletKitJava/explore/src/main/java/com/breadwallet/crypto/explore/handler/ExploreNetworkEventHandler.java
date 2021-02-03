package com.breadwallet.crypto.explore.handler;

import com.breadwallet.crypto.events.network.DefaultNetworkEventVisitor;
import com.breadwallet.crypto.events.network.NetworkCreatedEvent;
import com.breadwallet.crypto.events.network.NetworkDeletedEvent;
import com.breadwallet.crypto.events.network.NetworkFeesUpdatedEvent;
import com.breadwallet.crypto.events.network.NetworkUpdatedEvent;
import com.breadwallet.crypto.explore.ExploreConstants;

import java.util.logging.ConsoleHandler;
import java.util.logging.Logger;
import java.util.logging.Level;

import javax.annotation.Nullable;

public class ExploreNetworkEventHandler<Void> extends DefaultNetworkEventVisitor<Void> {

    private final static Logger Log;
    static {
        Log = Logger.getLogger(ExploreConstants.ExploreTag
                               + ExploreTransferEventHandler.class.getName());
    }

    public Void visit(NetworkCreatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Network created");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(NetworkUpdatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Network updated");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(NetworkFeesUpdatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Network fees updated");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(NetworkDeletedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Network deleted");
        return null;
    }
}
