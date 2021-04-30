/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKSystemState {

    CREATED {
        @Override
        public int toCore() {
            return CREATED_VALUE;
        }
    },

     DELETED {
        @Override
        public int toCore() {
            return DELETED_VALUE;
        }
    };

    private static final int CREATED_VALUE   = 0;
    private static final int DELETED_VALUE   = 1;

    public static WKSystemState fromCore(int nativeValue) {
        switch (nativeValue) {
            case CREATED_VALUE:   return CREATED;
            case DELETED_VALUE:   return DELETED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
