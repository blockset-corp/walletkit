package com.breadwallet.crypto.events.network;

public final class NetworkDeletedEvent implements NetworkEvent {

    @Override
    public <T> T accept(NetworkEventVisitor<T> visitor) {
        return visitor.visit(this);
    }

}
