/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Union;
import java.util.Arrays;
import java.util.List;

public class WKTransferEvent extends Structure {

    public int typeEnum;
    public u_union u;

    public static class u_union extends Union {

        public state_struct state;

        public static class state_struct extends Structure {

            public WKTransferState oldState;
            public WKTransferState newState;

            public state_struct() {
                super();
            }

            protected List<String> getFieldOrder() {
                return Arrays.asList("oldState", "newState");
            }

            public state_struct(WKTransferState oldState, WKTransferState newState) {
                super();
                this.oldState = oldState;
                this.newState = newState;
            }

            public state_struct(Pointer peer) {
                super(peer);
            }

            public static class ByReference extends state_struct implements Structure.ByReference {

            }
            public static class ByValue extends state_struct implements Structure.ByValue {

            }
        }

        public u_union() {
            super();
        }

        public u_union(state_struct state) {
            super();
            this.state = state;
            setType(state_struct.class);
        }

        public u_union(Pointer peer) {
            super(peer);
        }

        public static class ByReference extends u_union implements Structure.ByReference {

        }

        public static class ByValue extends u_union implements Structure.ByValue {

        }
    }

    public WKTransferEvent() {
        super();
    }

    public WKTransferEventType type() {
        return WKTransferEventType.fromCore(typeEnum);
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("typeEnum", "u");
    }

    public WKTransferEvent(int type, u_union u) {
        super();
        this.typeEnum = type;
        this.u = u;
    }

    public WKTransferEvent(Pointer peer) {
        super(peer);
    }

    @Override
    public void read() {
        super.read();
        if (type() == WKTransferEventType.CHANGED) {
            u.setType(u_union.state_struct.class);
            u.read();
        }
    }

    public static class ByReference extends WKTransferEvent implements Structure.ByReference {

    }

    public static class ByValue extends WKTransferEvent implements Structure.ByValue {

    }
}
