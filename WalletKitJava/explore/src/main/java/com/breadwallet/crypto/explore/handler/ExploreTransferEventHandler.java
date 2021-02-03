package com.breadwallet.crypto.explore.handler;

import com.breadwallet.crypto.events.transfer.DefaultTransferEventVisitor;
import com.breadwallet.crypto.events.transfer.TransferChangedEvent;
import com.breadwallet.crypto.events.transfer.TransferCreatedEvent;
import com.breadwallet.crypto.events.transfer.TransferDeletedEvent;
import com.breadwallet.crypto.explore.ExploreConstants;

import java.util.logging.ConsoleHandler;
import java.util.logging.Logger;
import java.util.logging.Level;

import javax.annotation.Nullable;

public class ExploreTransferEventHandler<Void> extends DefaultTransferEventVisitor<Void> {

    private final static Logger Log;
    static {
        Log = Logger.getLogger(ExploreConstants.ExploreTag
                               + ExploreTransferEventHandler.class.getName());
    }

    public ExploreTransferEventHandler() {
        super();
    }

    @Nullable
    @Override
    public Void visit(TransferChangedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Transfer changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(TransferCreatedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Transfer created");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(TransferDeletedEvent event) {
        Log.log(Level.FINE, "[todo-ev] Transfer deleted");
        return super.visit(event);
    }
}
