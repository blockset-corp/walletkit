/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 5/31/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKWalletManagerDisconnectReasonType {

    REQUESTED {
        @Override
        public int toCore() {
            return REQUESTED_VALUE;
        }
    },

    UNKNOWN {
        @Override
        public int toCore() {
            return UNKNOWN_VALUE;
        }
    },

    POSIX {
        @Override
        public int toCore() {
            return POSIX_VALUE;
        }
    };

    private static final int REQUESTED_VALUE = 0;
    private static final int UNKNOWN_VALUE = 1;
    private static final int POSIX_VALUE = 2;

    public static WKWalletManagerDisconnectReasonType fromCore(int nativeValue) {
        switch (nativeValue) {
            case REQUESTED_VALUE: return REQUESTED;
            case UNKNOWN_VALUE:   return UNKNOWN;
            case POSIX_VALUE:     return POSIX;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
