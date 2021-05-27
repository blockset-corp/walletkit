package com.breadwallet.corenative.crypto;

public enum BRCryptoNetworkEventType {
    CRYPTO_NETWORK_EVENT_CREATED {
        @Override
        public int toCore() {
            return CRYPTO_NETWORK_EVENT_CREATED_VALUE;
        }
    },

    CRYPTO_NETWORK_EVENT_FEES_UPDATED {
        @Override
        public int toCore() {
            return CRYPTO_NETWORK_EVENT_FEES_UPDATED_VALUE;
        }
    },

    CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED {
        @Override
        public int toCore() {
            return CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED_VALUE;
        }
    },

    CRYPTO_NETWORK_EVENT_DELETED {
        @Override
        public int toCore() {
            return CRYPTO_NETWORK_EVENT_DELETED_VALUE;
        }
    };

    private static final int CRYPTO_NETWORK_EVENT_CREATED_VALUE = 0;
    private static final int CRYPTO_NETWORK_EVENT_FEES_UPDATED_VALUE = 1;
    private static final int CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED_VALUE = 2;
    private static final int CRYPTO_NETWORK_EVENT_DELETED_VALUE = 3;

    public static BRCryptoNetworkEventType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CRYPTO_NETWORK_EVENT_CREATED_VALUE: return CRYPTO_NETWORK_EVENT_CREATED;
            case CRYPTO_NETWORK_EVENT_FEES_UPDATED_VALUE: return CRYPTO_NETWORK_EVENT_FEES_UPDATED;
            case CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED_VALUE: return CRYPTO_NETWORK_EVENT_CURRENCIES_UPDATED;
            case CRYPTO_NETWORK_EVENT_DELETED_VALUE: return CRYPTO_NETWORK_EVENT_DELETED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();

}
