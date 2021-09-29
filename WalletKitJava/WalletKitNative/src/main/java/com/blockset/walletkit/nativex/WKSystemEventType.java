/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKSystemEventType {
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

    NETWORK_ADDED {
        @Override
        public int toCore() {
            return NETWORK_ADDED_VALUE;
        }
    },

    NETWORK_CHANGED {
        @Override
        public int toCore() {
            return NETWORK_CHANGED_VALUE;
        }
    },

    NETWORK_DELETED {
        @Override
        public int toCore() {
            return NETWORK_DELETED_VALUE;
        }
    },

    MANAGER_ADDED {
        @Override
        public int toCore() {
            return MANAGER_ADDED_VALUE;
        }
    },

    MANAGER_CHANGED {
        @Override
        public int toCore() {
            return MANAGER_CHANGED_VALUE;
        }
    },

    MANAGER_DELETED {
        @Override
        public int toCore() {
            return MANAGER_DELETED_VALUE;
        }
    },

    DISCOVERED_NETWORKS {
        @Override
        public int toCore() {
            return DISCOVERED_NETWORKS_VALUE;
        }
    };

    private static final int CREATED_VALUE = 0;
    private static final int CHANGED_VALUE = 1;
    private static final int DELETED_VALUE = 2;

    private static final int NETWORK_ADDED_VALUE   = 3;
    private static final int NETWORK_CHANGED_VALUE = 4;
    private static final int NETWORK_DELETED_VALUE = 5;

    private static final int MANAGER_ADDED_VALUE   = 6;
    private static final int MANAGER_CHANGED_VALUE = 7;
    private static final int MANAGER_DELETED_VALUE = 8;

    private static final int DISCOVERED_NETWORKS_VALUE = 9;

    public static WKSystemEventType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CREATED_VALUE: return CREATED;
            case CHANGED_VALUE: return CHANGED;
            case DELETED_VALUE: return DELETED;

            case NETWORK_ADDED_VALUE:   return NETWORK_ADDED;
            case NETWORK_CHANGED_VALUE: return NETWORK_CHANGED;
            case NETWORK_DELETED_VALUE: return NETWORK_DELETED;

            case MANAGER_ADDED_VALUE:   return MANAGER_ADDED;
            case MANAGER_CHANGED_VALUE: return MANAGER_CHANGED;
            case MANAGER_DELETED_VALUE: return MANAGER_DELETED;

            case DISCOVERED_NETWORKS_VALUE: return DISCOVERED_NETWORKS;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
