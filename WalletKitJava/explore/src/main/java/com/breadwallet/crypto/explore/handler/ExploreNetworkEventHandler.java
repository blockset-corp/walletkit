package com.breadwallet.crypto.explore.handler;

import com.breadwallet.crypto.Network;
import com.breadwallet.crypto.events.network.DefaultNetworkEventVisitor;
import com.breadwallet.crypto.events.network.NetworkCreatedEvent;
import com.breadwallet.crypto.events.network.NetworkDeletedEvent;
import com.breadwallet.crypto.events.network.NetworkFeesUpdatedEvent;
import com.breadwallet.crypto.events.network.NetworkUpdatedEvent;

import java.util.logging.Logger;
import java.util.logging.Level;

import javax.annotation.Nullable;


public class ExploreNetworkEventHandler<Void> extends DefaultNetworkEventVisitor<Void> {

    private final Logger logger;
    private final Network net;

    public ExploreNetworkEventHandler(Network net, Logger logger) {
        super();
        this.net = net;
        this.logger = logger;
    }

    public Void visit(NetworkCreatedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Network created");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(NetworkUpdatedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Network updated");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(NetworkFeesUpdatedEvent event) {
        logger.log(Level.FINER, "[todo-ev] Network fees updated");
        return super.visit(event);
    }

    @Nullable
    @Override
    public Void visit(NetworkDeletedEvent event) {
        logger.log(Level.INFO, String.format("Network %s deleted",
                                             net.getName()));
        return null;
    }
}
