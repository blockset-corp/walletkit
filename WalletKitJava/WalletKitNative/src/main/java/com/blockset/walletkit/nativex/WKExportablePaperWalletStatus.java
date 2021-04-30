/*
 * Created by Ehsan Rezaie <ehsan@brd.com> on 11/23/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKExportablePaperWalletStatus {

    SUCCESS {
        @Override
        public int toCore() {
            return SUCCESS_VALUE;
        }
    },

    UNSUPPORTED_CURRENCY {
        @Override
        public int toCore() {
            return UNSUPPORTED_CURRENCY_VALUE;
        }
    },

    INVALID_ARGUMENTS {
        @Override
        public int toCore() {
            return INVALID_ARGUMENTS_VALUE;
        }
    },

    ILLEGAL_OPERATION {
        @Override
        public int toCore() {
            return ILLEGAL_OPERATION_VALUE;
        }
    };

    private static final int SUCCESS_VALUE = 0;
    private static final int UNSUPPORTED_CURRENCY_VALUE = 1;
    private static final int INVALID_ARGUMENTS_VALUE = 2;
    private static final int ILLEGAL_OPERATION_VALUE = 3;

    public static WKExportablePaperWalletStatus fromCore(int nativeValue) {
        switch (nativeValue) {
            case SUCCESS_VALUE:               return SUCCESS;
            case UNSUPPORTED_CURRENCY_VALUE:  return UNSUPPORTED_CURRENCY;
            case INVALID_ARGUMENTS_VALUE:     return INVALID_ARGUMENTS;
            case ILLEGAL_OPERATION_VALUE:     return ILLEGAL_OPERATION;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
