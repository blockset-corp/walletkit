/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 5/31/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
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

public class WKSystemEvent extends Structure {

    public int typeEnum;
    public u_union u;

    public static class u_union extends Union {

        public state_struct state;
        public WKNetwork network;
        public WKWalletManager walletManager;

        public static class state_struct extends Structure {

            public int oldState;
            public int newState;

            public state_struct() {
                super();
            }

            protected List<String> getFieldOrder() {
                return Arrays.asList("oldState", "newState");
            }

            public state_struct(int oldState, int newState) {
                super();
                this.oldState = oldState;
                this.newState = newState;
            }

            public state_struct(Pointer peer) {
                super(peer);
            }

            public WKSystemState oldState () {
                return WKSystemState.fromCore(oldState);
            }

            public WKSystemState newState () {
                return WKSystemState.fromCore(newState);
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

        public u_union(WKNetwork network) {
            super ();
            this.network = network;
            setType(WKNetwork.class);
        }

        public u_union (WKWalletManager walletManager) {
            super ();
            this.walletManager = walletManager;
            setType(WKWalletManager.class);
        }

        public u_union(Pointer peer) {
            super(peer);
        }

        public static class ByReference extends u_union implements Structure.ByReference {
        }

        public static class ByValue extends u_union implements Structure.ByValue {
        }
    }

    public WKSystemEvent() {
        super();
    }

    public WKSystemEventType type() {
        return WKSystemEventType.fromCore(typeEnum);
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("typeEnum", "u");
    }

    public WKSystemEvent(int type, u_union u) {
        super();
        this.typeEnum = type;
        this.u = u;
    }

    public WKSystemEvent(Pointer peer) {
        super(peer);
    }

    @Override
    public void read() {
        super.read();
        switch (type()) {
            case CHANGED:
                u.setType(u_union.state_struct.class);
                u.read();
                break;
        }
    }

    public static class ByReference extends WKSystemEvent implements Structure.ByReference {
    }

    public static class ByValue extends WKSystemEvent implements Structure.ByValue {
    }
}
