/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKTransferStateType {

    CREATED {
        @Override
        public int toCore() {
            return CREATED_VALUE;
        }
    },

    SIGNED {
        @Override
        public int toCore() {
            return SIGNED_VALUE;
        }
    },

    SUBMITTED {
        @Override
        public int toCore() {
            return SUBMITTED_VALUE;
        }
    },

    INCLUDED {
        @Override
        public int toCore() {
            return INCLUDED_VALUE;
        }
    },

    ERRORED {
        @Override
        public int toCore() {
            return ERRORED_VALUE;
        }
    },

    DELETED {
        @Override
        public int toCore() {
            return DELETED_VALUE;
        }
    };

    private static final int CREATED_VALUE   = 0;
    private static final int SIGNED_VALUE    = 1;
    private static final int SUBMITTED_VALUE = 2;
    private static final int INCLUDED_VALUE  = 3;
    private static final int ERRORED_VALUE   = 4;
    private static final int DELETED_VALUE   = 5;

    public static WKTransferStateType fromCore(int nativeValue) {
        switch (nativeValue) {
            case CREATED_VALUE:   return CREATED;
            case SIGNED_VALUE:    return SIGNED;
            case SUBMITTED_VALUE: return SUBMITTED;
            case INCLUDED_VALUE:  return INCLUDED;
            case ERRORED_VALUE:   return ERRORED;
            case DELETED_VALUE:   return DELETED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
