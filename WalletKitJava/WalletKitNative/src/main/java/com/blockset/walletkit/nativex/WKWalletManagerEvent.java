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

public class WKWalletManagerEvent extends Structure {

    public int typeEnum;
    public u_union u;

    public static class u_union extends Union {

        public state_struct state;
        public WKWallet wallet;
        public syncContinues_struct syncContinues;
        public syncStopped_struct syncStopped;
        public syncRecommended_struct syncRecommended;
        public long blockHeight;

        public static class state_struct extends Structure {

            public WKWalletManagerState oldValue;
            public WKWalletManagerState newValue;

            public state_struct() {
                super();
            }

            protected List<String> getFieldOrder() {
                return Arrays.asList("oldValue", "newValue");
            }

            public state_struct(WKWalletManagerState oldValue, WKWalletManagerState newValue) {
                super();
                this.oldValue = oldValue;
                this.newValue = newValue;
            }

            public state_struct(Pointer peer) {
                super(peer);
            }

            public static class ByReference extends state_struct implements Structure.ByReference {

            }

            public static class ByValue extends state_struct implements Structure.ByValue {

            }
        }

        public static class syncContinues_struct extends Structure {

            public int timestamp;
            public float percentComplete;
            public syncContinues_struct() {
                super();
            }

            protected List<String> getFieldOrder() {
                return Arrays.asList("timestamp", "percentComplete");
            }

            public syncContinues_struct(int timestamp, float percentComplete) {
                super();
                this.timestamp = timestamp;
                this.percentComplete = percentComplete;
            }

            public syncContinues_struct(Pointer peer) {
                super(peer);
            }

            public static class ByReference extends syncContinues_struct implements Structure.ByReference {

            }

            public static class ByValue extends syncContinues_struct implements Structure.ByValue {

            }
        }

        public static class syncStopped_struct extends Structure {

            public WKSyncStoppedReason reason;

            public syncStopped_struct() {
                super();
            }

            protected List<String> getFieldOrder() {
                return Arrays.asList("reason");
            }

            public syncStopped_struct(WKSyncStoppedReason reason) {
                super();
                this.reason = reason;
            }

            public syncStopped_struct(Pointer peer) {
                super(peer);
            }

            public static class ByReference extends syncStopped_struct implements Structure.ByReference {

            }

            public static class ByValue extends syncStopped_struct implements Structure.ByValue {

            }
        }

        public static class syncRecommended_struct extends Structure {

            public int depthEnum;

            public syncRecommended_struct() {
                super();
            }

            protected List<String> getFieldOrder() {
                return Arrays.asList("depthEnum");
            }

            public syncRecommended_struct(int depth) {
                super();
                this.depthEnum = depth;
            }

            public syncRecommended_struct(Pointer peer) {
                super(peer);
            }

            public WKSyncDepth depth() {
                return WKSyncDepth.fromCore(depthEnum);
            }

            public static class ByReference extends syncRecommended_struct implements Structure.ByReference {

            }

            public static class ByValue extends syncRecommended_struct implements Structure.ByValue {

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

        public u_union(WKWallet wallet) {
            super();
            this.wallet = wallet;
            setType(WKWallet.class);
        }

        public u_union(syncContinues_struct syncContinues) {
            super();
            this.syncContinues = syncContinues;
            setType(syncContinues_struct.class);
        }

        public u_union(syncStopped_struct syncStopped) {
            super();
            this.syncStopped = syncStopped;
            setType(syncStopped_struct.class);
        }

        public u_union(syncRecommended_struct syncRecommended) {
            super();
            this.syncRecommended = syncRecommended;
            setType(syncRecommended_struct.class);
        }

        public u_union(long blockHeight) {
            super();
            this.blockHeight = blockHeight;
            setType(long.class);
        }

        public u_union(Pointer peer) {
            super(peer);
        }

        public static class ByReference extends u_union implements Structure.ByReference {

        }

        public static class ByValue extends u_union implements Structure.ByValue {

        }
    }

    public WKWalletManagerEvent() {
        super();
    }

    public WKWalletManagerEventType type() {
        return WKWalletManagerEventType.fromCore(typeEnum);
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("typeEnum", "u");
    }

    public WKWalletManagerEvent(int type, u_union u) {
        super();
        this.typeEnum = type;
        this.u = u;
    }

    public WKWalletManagerEvent(Pointer peer) {
        super(peer);
    }

    @Override
    public void read() {
        super.read();
        switch (type()){
            case BLOCK_HEIGHT_UPDATED:
                u.setType(long.class);
                u.read();
                break;
            case CHANGED:
                u.setType(u_union.state_struct.class);
                u.read();
                break;
            case SYNC_CONTINUES:
                u.setType(u_union.syncContinues_struct.class);
                u.read();
                break;
            case SYNC_STOPPED:
                u.setType(u_union.syncStopped_struct.class);
                u.read();
                break;
            case SYNC_RECOMMENDED:
                u.setType(u_union.syncRecommended_struct.class);
                u.read();
                break;
            case WALLET_ADDED:
            case WALLET_CHANGED:
            case WALLET_DELETED:
                u.setType(WKWallet.class);
                u.read();
                break;
        }
    }

    public static class ByReference extends WKWalletManagerEvent implements Structure.ByReference {

    }

    public static class ByValue extends WKWalletManagerEvent implements Structure.ByValue {

    }
}
