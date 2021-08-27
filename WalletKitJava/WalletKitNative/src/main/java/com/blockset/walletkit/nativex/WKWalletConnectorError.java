/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKWalletConnectorError {

    UNSUPPORTED_CONNECTOR {
        @Override
        public int toCore () {
            return UNSUPPORTED_CONNECTOR_VALUE;
        }
    };

    public static WKWalletConnectorError fromCore(int nativeValue) {
        switch (nativeValue) {
            case UNSUPPORTED_CONNECTOR_VALUE:         return UNSUPPORTED_CONNECTOR;
            default: throw new IllegalArgumentException("Invalid WKWalletConnectorError core value");
        }
    }

    private static final int UNSUPPORTED_CONNECTOR_VALUE          = 0;

    public abstract int toCore();
}
