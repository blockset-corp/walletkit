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
    };

//    CRYPTO_SYSTEM_EVENT_NETWORK_ADDED,
//    CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED,
//    CRYPTO_SYSTEM_EVENT_NETWORK_DELETED,
//
//    CRYPTO_SYSTEM_EVENT_MANAGER_ADDED,
//    CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED,
//    CRYPTO_SYSTEM_EVENT_MANAGER_DELETED,

    private static final int CRYPTO_SYSTEM_EVENT_CREATED_VALUE = 0;
    private static final int CRYPTO_SYSTEM_EVENT_CHANGED_VALUE = 1;
    private static final int CRYPTO_SYSTEM_EVENT_DELETED_VALUE = 2;

    public static BRCryptoSystemEventType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CRYPTO_SYSTEM_EVENT_CREATED_VALUE: return CRYPTO_SYSTEM_EVENT_CREATED;
            case CRYPTO_SYSTEM_EVENT_CHANGED_VALUE: return CRYPTO_SYSTEM_EVENT_CHANGED;
            case CRYPTO_SYSTEM_EVENT_DELETED_VALUE: return CRYPTO_SYSTEM_EVENT_DELETED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
