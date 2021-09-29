/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKStatus {

    SUCCESS {
        @Override
        public int toCore() {
            return SUCCESS_VALUE;
        }
    },
    FAILED {
        @Override
        public int toCore() {
            return FAILED_VALUE;
        }
    },

    // Reference access
    UNKNOWN_NODE {
        @Override
        public int toCore() {
            return UNKNOWN_NODE_VALUE;
        }
    },
    UNKNOWN_TRANSFER {
        @Override
        public int toCore() {
            return UNKNOWN_TRANSFER_VALUE;
        }
    },
    UNKNOWN_ACCOUNT {
        @Override
        public int toCore() {
            return UNKNOWN_ACCOUNT_VALUE;
        }
    },
    UNKNOWN_WALLET {
        @Override
        public int toCore() {
            return UNKNOWN_WALLET_VALUE;
        }
    },
    UNKNOWN_BLOCK {
        @Override
        public int toCore() {
            return UNKNOWN_BLOCK_VALUE;
        }
    },
    UNKNOWN_LISTENER {
        @Override
        public int toCore() {
            return UNKNOWN_LISTENER_VALUE;
        }
    },

    // Node
    NODE_NOT_CONNECTED {
        @Override
        public int toCore() {
            return NODE_NOT_CONNECTED_VALUE;
        }
    },

    // Transfer
    TRANSFER_HASH_MISMATCH {
        @Override
        public int toCore() {
            return TRANSFER_HASH_MISMATCH_VALUE;
        }
    },
    TRANSFER_SUBMISSION {
        @Override
        public int toCore() {
            return TRANSFER_SUBMISSION_VALUE;
        }
    },

    // Numeric
    NUMERIC_PARSE {
        @Override
        public int toCore() {
            return NUMERIC_PARSE_VALUE;
        }
    };

    private static final int SUCCESS_VALUE = 0;
    private static final int FAILED_VALUE                  = 1;

    private static final int UNKNOWN_NODE_VALUE            = 10000;
    private static final int UNKNOWN_TRANSFER_VALUE        = 10001;
    private static final int UNKNOWN_ACCOUNT_VALUE         = 10002;
    private static final int UNKNOWN_WALLET_VALUE          = 10003;
    private static final int UNKNOWN_BLOCK_VALUE           = 10004;
    private static final int UNKNOWN_LISTENER_VALUE        = 10005;

    private static final int NODE_NOT_CONNECTED_VALUE      = 20000;

    private static final int TRANSFER_HASH_MISMATCH_VALUE  = 30000;
    private static final int TRANSFER_SUBMISSION_VALUE     = 30001;

    private static final int NUMERIC_PARSE_VALUE           = 40000;

    public static WKStatus fromCore(int nativeValue) {
        switch (nativeValue) {
            case SUCCESS_VALUE:                      return SUCCESS;
            case FAILED_VALUE:                 return FAILED;

            case UNKNOWN_NODE_VALUE:           return UNKNOWN_NODE;
            case UNKNOWN_TRANSFER_VALUE:       return UNKNOWN_TRANSFER;
            case UNKNOWN_ACCOUNT_VALUE:        return UNKNOWN_ACCOUNT;
            case UNKNOWN_WALLET_VALUE:         return UNKNOWN_WALLET;
            case UNKNOWN_BLOCK_VALUE:          return UNKNOWN_BLOCK;
            case UNKNOWN_LISTENER_VALUE:       return UNKNOWN_LISTENER;

            case NODE_NOT_CONNECTED_VALUE:     return NODE_NOT_CONNECTED;

            case TRANSFER_HASH_MISMATCH_VALUE: return TRANSFER_HASH_MISMATCH;
            case TRANSFER_SUBMISSION_VALUE:    return TRANSFER_SUBMISSION;

            case NUMERIC_PARSE_VALUE:          return NUMERIC_PARSE;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
