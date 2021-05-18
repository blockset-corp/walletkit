/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKTransferDirection {

    SENT {
        @Override
        public int toCore() {
            return SENT_VALUE;
        }
    },

    RECEIVED {
        @Override
        public int toCore() {
            return RECEIVED_VALUE;
        }
    },

    RECOVERED {
        @Override
        public int toCore() {
            return RECOVERED_VALUE;
        }
    };

    private static final int SENT_VALUE = 0;
    private static final int RECEIVED_VALUE = 1;
    private static final int RECOVERED_VALUE = 2;

    public static WKTransferDirection fromCore(int nativeValue) {
        switch (nativeValue) {
            case SENT_VALUE:      return SENT;
            case RECEIVED_VALUE:  return RECEIVED;
            case RECOVERED_VALUE: return RECOVERED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
