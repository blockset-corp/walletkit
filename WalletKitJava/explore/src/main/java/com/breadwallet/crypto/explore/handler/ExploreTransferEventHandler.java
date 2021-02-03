package com.breadwallet.crypto.explore.handler;

import com.breadwallet.crypto.events.transfer.DefaultTransferEventVisitor;
import com.breadwallet.crypto.events.transfer.TransferChangedEvent;
import com.breadwallet.crypto.events.transfer.TransferCreatedEvent;
import com.breadwallet.crypto.events.transfer.TransferDeletedEvent;

import java.util.logging.Logger;
import java.util.logging.Level;

import javax.annotation.Nullable;


public class ExploreTransferEventHandler<Void> extends DefaultTransferEventVisitor<Void> {

    private final Logger logger;

    public ExploreTransferEventHandler(Logger logger) {
        super();
        this.logger = logger;
    }

    @Nullable
    @Override
    public Void visit(TransferChangedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Transfer changed");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(TransferCreatedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Transfer created");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(TransferDeletedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Transfer deleted");
        return super.visit(event);
    }
}
