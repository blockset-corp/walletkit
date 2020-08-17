package com.breadwallet.crypto.events.network;

public final class NetworkUpdatedEvent implements NetworkEvent {

    @Override
    public <T> T accept(NetworkEventVisitor<T> visitor) {
        return visitor.visit(this);
    }

}
