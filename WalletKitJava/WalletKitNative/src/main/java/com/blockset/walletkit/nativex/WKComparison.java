/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKComparison {

    LT {
        @Override
        public int toCore() {
            return WK_COMPARE_LT_VALUE;
        }
    },

    EQ {
        @Override
        public int toCore() {
            return WK_COMPARE_EQ_VALUE;
        }
    },

    GT {
        @Override
        public int toCore() {
            return WK_COMPARE_GT_VALUE;
        }
    };

    private static final int WK_COMPARE_LT_VALUE = 0;
    private static final int WK_COMPARE_EQ_VALUE = 1;
    private static final int WK_COMPARE_GT_VALUE = 2;

    public static WKComparison fromCore(int nativeValue) {
        switch (nativeValue) {
            case WK_COMPARE_LT_VALUE: return LT;
            case WK_COMPARE_EQ_VALUE: return EQ;
            case WK_COMPARE_GT_VALUE: return GT;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
