/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKWalletSweeperStatus {

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

    INVALID_KEY {
        @Override
        public int toCore() {
            return INVALID_KEY_VALUE;
        }
    },

    INVALID_ARGUMENTS {
        @Override
        public int toCore() {
            return INVALID_ARGUMENTS_VALUE;
        }
    },

    INVALID_TRANSACTION {
        @Override
        public int toCore() {
            return INVALID_TRANSACTION_VALUE;
        }
    },

    INVALID_SOURCE_WALLET {
        @Override
        public int toCore() {
            return INVALID_SOURCE_WALLET_VALUE;
        }
    },

    NO_TRANSFERS_FOUND {
        @Override
        public int toCore() {
            return NO_TRANSFERS_FOUND_VALUE;
        }
    },

    INSUFFICIENT_FUNDS{
        @Override
        public int toCore() {
            return INSUFFICIENT_FUNDS_VALUE;
        }
    },

    UNABLE_TO_SWEEP  {
        @Override
        public int toCore() {
            return UNABLE_TO_SWEEP_VALUE;
        }
    },

    ILLEGAL_OPERATION  {
        @Override
        public int toCore() {
            return ILLEGAL_OPERATION_VALUE;
        }
    };

    private static final int SUCCESS_VALUE                = 0;
    private static final int UNSUPPORTED_CURRENCY_VALUE   = 1;
    private static final int INVALID_KEY_VALUE            = 2;
    private static final int INVALID_ARGUMENTS_VALUE      = 3;
    private static final int INVALID_TRANSACTION_VALUE    = 4;
    private static final int INVALID_SOURCE_WALLET_VALUE  = 5;
    private static final int NO_TRANSFERS_FOUND_VALUE     = 6;
    private static final int INSUFFICIENT_FUNDS_VALUE     = 7;
    private static final int UNABLE_TO_SWEEP_VALUE        = 8;
    private static final int ILLEGAL_OPERATION_VALUE      = 9;

    public static WKWalletSweeperStatus fromCore(int nativeValue) {
        switch (nativeValue) {
            case SUCCESS_VALUE:               return SUCCESS;
            case UNSUPPORTED_CURRENCY_VALUE:  return UNSUPPORTED_CURRENCY;
            case INVALID_KEY_VALUE:           return INVALID_KEY;
            case INVALID_ARGUMENTS_VALUE:     return INVALID_ARGUMENTS;
            case INVALID_TRANSACTION_VALUE:   return INVALID_TRANSACTION;
            case INVALID_SOURCE_WALLET_VALUE: return INVALID_SOURCE_WALLET;
            case NO_TRANSFERS_FOUND_VALUE:    return NO_TRANSFERS_FOUND;
            case INSUFFICIENT_FUNDS_VALUE:    return INSUFFICIENT_FUNDS;
            case UNABLE_TO_SWEEP_VALUE:       return UNABLE_TO_SWEEP;
            case ILLEGAL_OPERATION_VALUE:     return ILLEGAL_OPERATION;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
