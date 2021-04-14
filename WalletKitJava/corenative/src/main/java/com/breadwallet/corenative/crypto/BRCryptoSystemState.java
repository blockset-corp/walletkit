/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

public enum BRCryptoSystemState {

    CRYPTO_SYSTEM_STATE_CREATED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_STATE_CREATED_VALUE;
        }
    },

     CRYPTO_SYSTEM_STATE_DELETED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_STATE_DELETED_VALUE;
        }
    };

    private static final int CRYPTO_SYSTEM_STATE_CREATED_VALUE   = 0;
    private static final int CRYPTO_SYSTEM_STATE_DELETED_VALUE   = 1;

    public static BRCryptoSystemState fromCore(int nativeValue) {
        switch (nativeValue) {
            case CRYPTO_SYSTEM_STATE_CREATED_VALUE:   return CRYPTO_SYSTEM_STATE_CREATED;
            case CRYPTO_SYSTEM_STATE_DELETED_VALUE:   return CRYPTO_SYSTEM_STATE_DELETED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
