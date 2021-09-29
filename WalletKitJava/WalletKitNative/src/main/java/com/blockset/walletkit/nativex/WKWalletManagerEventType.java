/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKWalletManagerEventType {

    CREATED {
        @Override
        public int toCore() {
            return CREATED_VALUE;
        }
    },

    CHANGED {
        @Override
        public int toCore() {
            return CHANGED_VALUE;
        }
    },

    DELETED {
        @Override
        public int toCore() {
            return DELETED_VALUE;
        }
    },

    WALLET_ADDED {
        @Override
        public int toCore() {
            return WALLET_ADDED_VALUE;
        }
    },

    WALLET_CHANGED {
        @Override
        public int toCore() {
            return WALLET_CHANGED_VALUE;
        }
    },

    WALLET_DELETED {
        @Override
        public int toCore() {
            return WALLET_DELETED_VALUE;
        }
    },

    SYNC_STARTED {
        @Override
        public int toCore() {
            return SYNC_STARTED_VALUE;
        }
    },

    SYNC_CONTINUES {
        @Override
        public int toCore() {
            return SYNC_CONTINUES_VALUE;
        }
    },

    SYNC_STOPPED {
        @Override
        public int toCore() {
            return SYNC_STOPPED_VALUE;
        }
    },

    SYNC_RECOMMENDED {
        @Override
        public int toCore() {
            return SYNC_RECOMMENDED_VALUE;
        }
    },

    BLOCK_HEIGHT_UPDATED {
        @Override
        public int toCore() {
            return BLOCK_HEIGHT_UPDATED_VALUE;
        }
    };

    private static final int CREATED_VALUE              = 0;
    private static final int CHANGED_VALUE              = 1;
    private static final int DELETED_VALUE              = 2;
    private static final int WALLET_ADDED_VALUE         = 3;
    private static final int WALLET_CHANGED_VALUE       = 4;
    private static final int WALLET_DELETED_VALUE       = 5;
    private static final int SYNC_STARTED_VALUE         = 6;
    private static final int SYNC_CONTINUES_VALUE       = 7;
    private static final int SYNC_STOPPED_VALUE         = 8;
    private static final int SYNC_RECOMMENDED_VALUE     = 9;
    private static final int BLOCK_HEIGHT_UPDATED_VALUE = 10;

    public static WKWalletManagerEventType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CREATED_VALUE:              return CREATED;
            case CHANGED_VALUE:              return CHANGED;
            case DELETED_VALUE:              return DELETED;
            case WALLET_ADDED_VALUE:         return WALLET_ADDED;
            case WALLET_CHANGED_VALUE:       return WALLET_CHANGED;
            case WALLET_DELETED_VALUE:       return WALLET_DELETED;
            case SYNC_STARTED_VALUE:         return SYNC_STARTED;
            case SYNC_CONTINUES_VALUE:       return SYNC_CONTINUES;
            case SYNC_STOPPED_VALUE:         return SYNC_STOPPED;
            case SYNC_RECOMMENDED_VALUE:     return SYNC_RECOMMENDED;
            case BLOCK_HEIGHT_UPDATED_VALUE: return BLOCK_HEIGHT_UPDATED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
