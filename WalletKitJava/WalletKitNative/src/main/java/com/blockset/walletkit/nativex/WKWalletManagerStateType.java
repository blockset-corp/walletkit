/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 5/31/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKWalletManagerStateType {

    CREATED {
        @Override
        public int toCore() {
            return CREATED_VALUE;
        }
    },

    DISCONNECTED {
        @Override
        public int toCore() {
            return DISCONNECTED_VALUE;
        }
    },

    CONNECTED {
        @Override
        public int toCore() {
            return CONNECTED_VALUE;
        }
    },

    SYNCING {
        @Override
        public int toCore() {
            return SYNCING_VALUE;
        }
    },

    DELETED {
        @Override
        public int toCore() {
            return DELETED_VALUE;
        }
    };

    private static final int CREATED_VALUE = 0;
    private static final int DISCONNECTED_VALUE = 1;
    private static final int CONNECTED_VALUE = 2;
    private static final int SYNCING_VALUE = 3;
    private static final int DELETED_VALUE = 4;

    public static WKWalletManagerStateType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CREATED_VALUE:      return CREATED;
            case DISCONNECTED_VALUE: return DISCONNECTED;
            case CONNECTED_VALUE:    return CONNECTED;
            case SYNCING_VALUE:      return SYNCING;
            case DELETED_VALUE:      return DELETED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
