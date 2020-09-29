package com.breadwallet.crypto.events.system;

public class SystemDeletedEvent implements SystemEvent {
    @Override
    public <T> T accept(SystemEventVisitor<T> visitor) {
        return visitor.visit(this);
    }
}
