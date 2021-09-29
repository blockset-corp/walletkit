/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKSyncMode {

    API_ONLY {
        @Override
        public int toCore() {
            return API_ONLY_VALUE;
        }
    },

    API_WITH_P2P_SEND {
        @Override
        public int toCore() {
            return API_WITH_P2P_SEND_VALUE;
        }
    },

    P2P_WITH_API_SYNC {
        @Override
        public int toCore() {
            return P2P_WITH_API_SYNC_VALUE;
        }
    },

    P2P_ONLY {
        @Override
        public int toCore() {
            return P2P_ONLY_VALUE;
        }
    };

    private static final int API_ONLY_VALUE = 0;
    private static final int API_WITH_P2P_SEND_VALUE = 1;
    private static final int P2P_WITH_API_SYNC_VALUE = 2;
    private static final int P2P_ONLY_VALUE = 3;

    public static WKSyncMode fromCore(int nativeValue) {
        switch (nativeValue) {
            case API_ONLY_VALUE:          return API_ONLY;
            case API_WITH_P2P_SEND_VALUE: return API_WITH_P2P_SEND;
            case P2P_WITH_API_SYNC_VALUE: return P2P_WITH_API_SYNC;
            case P2P_ONLY_VALUE:          return P2P_ONLY;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
