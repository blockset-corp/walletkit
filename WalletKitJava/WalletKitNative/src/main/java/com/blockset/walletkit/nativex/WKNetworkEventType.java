/*
 * Created by Michael Carrara.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKNetworkEventType {
    CREATED {
        @Override
        public int toCore() {
            return CREATED_VALUE;
        }
    },

    FEES_UPDATED {
        @Override
        public int toCore() {
            return FEES_UPDATED_VALUE;
        }
    },

    CURRENCIES_UPDATED {
        @Override
        public int toCore() {
            return CURRENCIES_UPDATED_VALUE;
        }
    },

    DELETED {
        @Override
        public int toCore() {
            return DELETED_VALUE;
        }
    };

    private static final int CREATED_VALUE = 0;
    private static final int FEES_UPDATED_VALUE = 1;
    private static final int CURRENCIES_UPDATED_VALUE = 2;
    private static final int DELETED_VALUE = 3;

    public static WKNetworkEventType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CREATED_VALUE: return CREATED;
            case FEES_UPDATED_VALUE: return FEES_UPDATED;
            case CURRENCIES_UPDATED_VALUE: return CURRENCIES_UPDATED;
            case DELETED_VALUE: return DELETED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();

}
