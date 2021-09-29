/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKTransferAttributeValidationError {

    REQUIRED_BUT_NOT_PROVIDED {
        @Override
        public int toCore () {
            return REQUIRED_BUT_NOT_PROVIDED_VALUE;
        }
    },

    MISMATCHED_TYPE {
            @Override
            public int toCore () {
                return MISMATCHED_TYPE_VALUE;
            }
    },

    RELATIONSHIP_INCONSISTENCY {
                @Override
                public int toCore () {
                    return RELATIONSHIP_INCONSISTENCY_VALUE;
                }
    };

    private static final int REQUIRED_BUT_NOT_PROVIDED_VALUE = 0;
    private static final int MISMATCHED_TYPE_VALUE = 1;
    private static final int RELATIONSHIP_INCONSISTENCY_VALUE = 2;

    public static WKTransferAttributeValidationError fromCore(int nativeValue) {
        switch (nativeValue) {
            case REQUIRED_BUT_NOT_PROVIDED_VALUE:
                return REQUIRED_BUT_NOT_PROVIDED;
            case MISMATCHED_TYPE_VALUE:
                return MISMATCHED_TYPE;
            case RELATIONSHIP_INCONSISTENCY_VALUE:
                return RELATIONSHIP_INCONSISTENCY;
            default:
                throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}