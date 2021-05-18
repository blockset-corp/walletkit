/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.events.network;

public final class NetworkUpdatedEvent implements NetworkEvent {

    @Override
    public <T> T accept(NetworkEventVisitor<T> visitor) {
        return visitor.visit(this);
    }

}
