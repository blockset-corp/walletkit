/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKWalletEventType {

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

    TRANSFER_ADDED {
        @Override
        public int toCore() {
            return TRANSFER_ADDED_VALUE;
        }
    },

    TRANSFER_CHANGED {
        @Override
        public int toCore() {
            return TRANSFER_CHANGED_VALUE;
        }
    },

    TRANSFER_SUBMITTED {
        @Override
        public int toCore() {
            return TRANSFER_SUBMITTED_VALUE;
        }
    },

    TRANSFER_DELETED {
        @Override
        public int toCore() {
            return TRANSFER_DELETED_VALUE;
        }
    },

    BALANCE_UPDATED {
        @Override
        public int toCore() {
            return BALANCE_UPDATED_VALUE;
        }
    },

    FEE_BASIS_UPDATED {
        @Override
        public int toCore() {
            return FEE_BASIS_UPDATED_VALUE;
        }
    },

    FEE_BASIS_ESTIMATED {
        @Override
        public int toCore() {
            return FEE_BASIS_ESTIMATED_VALUE;
        }
    };

    private static final int CREATED_VALUE              = 0;
    private static final int CHANGED_VALUE              = 1;
    private static final int DELETED_VALUE              = 2;
    private static final int TRANSFER_ADDED_VALUE       = 3;
    private static final int TRANSFER_CHANGED_VALUE     = 4;
    private static final int TRANSFER_SUBMITTED_VALUE   = 5;
    private static final int TRANSFER_DELETED_VALUE     = 6;
    private static final int BALANCE_UPDATED_VALUE      = 7;
    private static final int FEE_BASIS_UPDATED_VALUE    = 8;
    private static final int FEE_BASIS_ESTIMATED_VALUE  = 9;

    public static WKWalletEventType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CREATED_VALUE:             return CREATED;
            case CHANGED_VALUE:             return CHANGED;
            case DELETED_VALUE:             return DELETED;
            case TRANSFER_ADDED_VALUE:      return TRANSFER_ADDED;
            case TRANSFER_CHANGED_VALUE:    return TRANSFER_CHANGED;
            case TRANSFER_SUBMITTED_VALUE:  return TRANSFER_SUBMITTED;
            case TRANSFER_DELETED_VALUE:    return TRANSFER_DELETED;
            case BALANCE_UPDATED_VALUE:     return BALANCE_UPDATED;
            case FEE_BASIS_UPDATED_VALUE:   return FEE_BASIS_UPDATED;
            case FEE_BASIS_ESTIMATED_VALUE: return FEE_BASIS_ESTIMATED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
