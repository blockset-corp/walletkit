package com.breadwallet.crypto.events.system;

import com.breadwallet.crypto.SystemState;

public class SystemChangedEvent implements SystemEvent {

    private final SystemState oldState;
    private final SystemState newState;

    public SystemChangedEvent(SystemState oldState, SystemState newState) {
        this.oldState = oldState;
        this.newState = newState;
    }

    public SystemState getOldState() {
        return oldState;
    }

    public SystemState getNewState() {
        return newState;
    }

    @Override
    public <T> T accept(SystemEventVisitor<T> visitor) {
        return visitor.visit(this);
    }
}
