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

public class WKWalletManagerState extends Structure {

    public int typeEnum;
    public u_union u;

    public static class u_union extends Union {

        public disconnected_struct disconnected;

        public static class disconnected_struct extends Structure {

            public WKWalletManagerDisconnectReason reason;

            public disconnected_struct() {
                super();
            }

            protected List<String> getFieldOrder() {
                return Arrays.asList("reason");
            }

            public disconnected_struct(WKWalletManagerDisconnectReason reason) {
                super();
                this.reason = reason;
            }

            public disconnected_struct(Pointer peer) {
                super(peer);
            }

            public static class ByReference extends disconnected_struct implements Structure.ByReference {

            }

            public static class ByValue extends disconnected_struct implements Structure.ByValue {

            }
        }

        public u_union() {
            super();
        }

        public u_union(disconnected_struct state) {
            super();
            this.disconnected = state;
            setType(disconnected_struct.class);
        }

        public u_union(Pointer peer) {
            super(peer);
        }

        public static class ByReference extends u_union implements Structure.ByReference {

        }

        public static class ByValue extends u_union implements Structure.ByValue {

        }
    }

    public WKWalletManagerState() {
        super();
    }

    public WKWalletManagerStateType type() {
        return WKWalletManagerStateType.fromCore(typeEnum);
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("typeEnum", "u");
    }

    public WKWalletManagerState(int type, u_union u) {
        super();
        this.typeEnum = type;
        this.u = u;
    }

    public WKWalletManagerState(Pointer peer) {
        super(peer);
    }

    @Override
    public void read() {
        super.read();
        switch (type()){
            case DISCONNECTED:
                u.setType(u_union.disconnected_struct.class);
                u.read();
                break;
        }
    }

    public static class ByReference extends WKWalletManagerState implements Structure.ByReference {

    }

    public static class ByValue extends WKWalletManagerState implements Structure.ByValue {

    }
}
