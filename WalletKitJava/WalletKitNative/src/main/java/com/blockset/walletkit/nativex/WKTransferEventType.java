/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKTransferEventType {

    CREATED {
        @Override
        public int toCore() {
            return CREATED_VALUE;
        }
    },

    CHANGED {
        @Override
        public int toCore() {
            return CHANGED_VALUE;
        }
    },

    DELETED {
        @Override
        public int toCore() {
            return DELETED_VALUE;
        }
    };

    private static final int CREATED_VALUE = 0;
    private static final int CHANGED_VALUE = 1;
    private static final int DELETED_VALUE = 2;

    public static WKTransferEventType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CREATED_VALUE: return CREATED;
            case CHANGED_VALUE: return CHANGED;
            case DELETED_VALUE: return DELETED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
