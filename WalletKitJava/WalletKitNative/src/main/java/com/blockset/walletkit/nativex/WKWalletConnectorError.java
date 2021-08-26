/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKWalletConnectorError {

    SUCCESS {
        @Override
        public int toCore() { return SUCCESS_VALUE; }
    },

    ERROR_UNSUPPORTED_CONNECTOR {
        @Override
        public int toCore () {
            return ERROR_UNSUPPORTED_CONNECTOR_VALUE;
        }
    };

    public static WKWalletConnectorError fromCore(int nativeValue) {
        switch (nativeValue) {
            case SUCCESS_VALUE:                             return SUCCESS;
            case ERROR_UNSUPPORTED_CONNECTOR_VALUE:         return ERROR_UNSUPPORTED_CONNECTOR;
            default: throw new IllegalArgumentException("Invalid WKWalletConnectorError core value");
        }
    }

    private static final int SUCCESS_VALUE                              = 0;
    private static final int ERROR_UNSUPPORTED_CONNECTOR_VALUE          = 1;

    public abstract int toCore();
}
