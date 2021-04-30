/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.events.system;

import com.blockset.walletkit.SystemState;

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
