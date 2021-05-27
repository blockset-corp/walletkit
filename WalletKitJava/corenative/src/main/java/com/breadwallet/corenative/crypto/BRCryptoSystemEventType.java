package com.breadwallet.corenative.crypto;

public enum BRCryptoSystemEventType {
    CRYPTO_SYSTEM_EVENT_CREATED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_CREATED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_CHANGED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_CHANGED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_DELETED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_DELETED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_NETWORK_ADDED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_NETWORK_ADDED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_NETWORK_DELETED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_NETWORK_DELETED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_MANAGER_ADDED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_MANAGER_ADDED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_MANAGER_DELETED {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_MANAGER_DELETED_VALUE;
        }
    },

    CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS {
        @Override
        public int toCore() {
            return CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS_VALUE;
        }
    };

    private static final int CRYPTO_SYSTEM_EVENT_CREATED_VALUE = 0;
    private static final int CRYPTO_SYSTEM_EVENT_CHANGED_VALUE = 1;
    private static final int CRYPTO_SYSTEM_EVENT_DELETED_VALUE = 2;

    private static final int CRYPTO_SYSTEM_EVENT_NETWORK_ADDED_VALUE   = 3;
    private static final int CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED_VALUE = 4;
    private static final int CRYPTO_SYSTEM_EVENT_NETWORK_DELETED_VALUE = 5;

    private static final int CRYPTO_SYSTEM_EVENT_MANAGER_ADDED_VALUE   = 6;
    private static final int CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED_VALUE = 7;
    private static final int CRYPTO_SYSTEM_EVENT_MANAGER_DELETED_VALUE = 8;

    private static final int CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS_VALUE = 9;

    public static BRCryptoSystemEventType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CRYPTO_SYSTEM_EVENT_CREATED_VALUE: return CRYPTO_SYSTEM_EVENT_CREATED;
            case CRYPTO_SYSTEM_EVENT_CHANGED_VALUE: return CRYPTO_SYSTEM_EVENT_CHANGED;
            case CRYPTO_SYSTEM_EVENT_DELETED_VALUE: return CRYPTO_SYSTEM_EVENT_DELETED;

            case CRYPTO_SYSTEM_EVENT_NETWORK_ADDED_VALUE:   return CRYPTO_SYSTEM_EVENT_NETWORK_ADDED;
            case CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED_VALUE: return CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED;
            case CRYPTO_SYSTEM_EVENT_NETWORK_DELETED_VALUE: return CRYPTO_SYSTEM_EVENT_NETWORK_DELETED;

            case CRYPTO_SYSTEM_EVENT_MANAGER_ADDED_VALUE:   return CRYPTO_SYSTEM_EVENT_MANAGER_ADDED;
            case CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED_VALUE: return CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED;
            case CRYPTO_SYSTEM_EVENT_MANAGER_DELETED_VALUE: return CRYPTO_SYSTEM_EVENT_MANAGER_DELETED;

            case CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS_VALUE: return CRYPTO_SYSTEM_EVENT_DISCOVERED_NETWORKS;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
