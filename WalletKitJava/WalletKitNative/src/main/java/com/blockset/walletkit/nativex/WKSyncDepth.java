/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKSyncDepth {

    FROM_LAST_CONFIRMED_SEND {
        @Override
        public int toCore() {
            return FROM_LAST_CONFIRMED_SEND_VALUE;
        }
    },

    FROM_LAST_TRUSTED_BLOCK {
        @Override
        public int toCore() {
            return FROM_LAST_TRUSTED_BLOCK_VALUE;
        }
    },

    FROM_CREATION {
        @Override
        public int toCore() {
            return FROM_CREATION_VALUE;
        }
    };

    private static final int FROM_LAST_CONFIRMED_SEND_VALUE = 0;
    private static final int FROM_LAST_TRUSTED_BLOCK_VALUE = 1;
    private static final int FROM_CREATION_VALUE = 2;

    public static WKSyncDepth fromCore(int nativeValue) {
        switch (nativeValue) {
            case FROM_LAST_CONFIRMED_SEND_VALUE: return FROM_LAST_CONFIRMED_SEND;
            case FROM_LAST_TRUSTED_BLOCK_VALUE:  return FROM_LAST_TRUSTED_BLOCK;
            case FROM_CREATION_VALUE:            return FROM_CREATION;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
