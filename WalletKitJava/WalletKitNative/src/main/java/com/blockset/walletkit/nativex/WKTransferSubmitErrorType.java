/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 9/18/19.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import static com.google.common.base.Preconditions.checkState;

public enum WKTransferSubmitErrorType {

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

    private static final int UNKNOWN_VALUE = 0;
    private static final int POSIX_VALUE = 1;

    public static WKTransferSubmitErrorType fromCore(int nativeValue) {
        switch (nativeValue) {
            case UNKNOWN_VALUE: return UNKNOWN;
            case POSIX_VALUE:   return POSIX;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
