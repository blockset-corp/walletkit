package com.breadwallet.corenative.crypto;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Union;

import java.util.Arrays;
import java.util.List;

public class BRCryptoSystemEvent extends Structure {

    public int typeEnum;
    public u_union u;

    public static class u_union extends Union {

        public state_struct state;

        public static class state_struct extends Structure {

            public BRCryptoSystemState oldState;
            public BRCryptoSystemState newState;

            public state_struct() {
                super();
            }

            protected List<String> getFieldOrder() {
                return Arrays.asList("oldState", "newState");
            }

            public state_struct(BRCryptoSystemState oldState, BRCryptoSystemState newState) {
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

    public BRCryptoSystemEvent() {
        super();
    }

    public BRCryptoSystemEventType type() {
        return BRCryptoSystemEventType.fromCore(typeEnum);
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("typeEnum", "u");
    }

    public BRCryptoSystemEvent(int type, u_union u) {
        super();
        this.typeEnum = type;
        this.u = u;
    }

    public BRCryptoSystemEvent(Pointer peer) {
        super(peer);
    }

    @Override
    public void read() {
        super.read();
        if (type() == BRCryptoSystemEventType.CRYPTO_SYSTEM_EVENT_CHANGED) {
            u.setType(u_union.state_struct.class);
            u.read();
        }
    }

    public static class ByReference extends BRCryptoSystemEvent implements Structure.ByReference {

    }

    public static class ByValue extends BRCryptoSystemEvent implements Structure.ByValue {

    }
}
