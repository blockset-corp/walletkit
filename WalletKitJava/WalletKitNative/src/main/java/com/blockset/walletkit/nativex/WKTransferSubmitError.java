/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 9/18/19.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

public class WKTransferSubmitError extends Structure {

    public enum Type {
        ACCOUNT {
            @Override
            public int toCore() {
                return ACCOUNT_VALUE;
            }
        },

        SIGNATURE {
            @Override
            public int toCore() {
                return SIGNATURE_VALUE;
            }
        },

        INSUFFICIENT_BALANCE {
            @Override
            public int toCore() {
                return INSUFFICIENT_BALANCE_VALUE;
            }
        },

        INSUFFICIENT_NETWORK_FEE {
            @Override
            public int toCore() {
                return INSUFFICIENT_NETWORK_FEE_VALUE;
            }
        },

        INSUFFICIENT_NETWORK_COST_UNIT {
            @Override
            public int toCore() {
                return INSUFFICIENT_NETWORK_COST_UNIT_VALUE;
            }
        },

        INSUFFICIENT_FEE {
            @Override
            public int toCore() {
                return INSUFFICIENT_FEE_VALUE;
            }
        },

        NONCE_TOO_LOW {
            @Override
            public int toCore() {
                return NONCE_TOO_LOW_VALUE;
            }
        },

        NONCE_INVALID {
            @Override
            public int toCore() {
                return NONCE_INVALID_VALUE;
            }
        },

        TRANSACTION_EXPIRED {
            @Override
            public int toCore() {
                return TRANSACTION_EXPIRED_VALUE;
            }
        },

        TRANSACTION_DUPLICATE {
            @Override
            public int toCore() {
                return TRANSACTION_DUPLICATE_VALUE;
            }
        },

        TRANSACTION {
            @Override
            public int toCore() {
                return TRANSACTION_VALUE;
            }
        },

        UNKNOWN {
            @Override
            public int toCore() {
                return UNKNOWN_VALUE;
            }
        },

        CLIENT_BAD_REQUEST {
            @Override
            public int toCore() {
                return CLIENT_BAD_REQUEST_VALUE;
            }
        },

        CLIENT_PERMISSION {
            @Override
            public int toCore() {
                return CLIENT_PERMISSION_VALUE;
            }
        },

        CLIENT_RESOURCE {
            @Override
            public int toCore() {
                return CLIENT_RESOURCE_VALUE;
            }
        },

        CLIENT_BAD_RESPONSE {
            @Override
            public int toCore() {
                return CLIENT_BAD_RESPONSE_VALUE;
            }
        },

        CLIENT_UNAVAILABLE {
            @Override
            public int toCore() {
                return CLIENT_UNAVAILABLE_VALUE;
            }
        },

        LOST_NETWORK {
            @Override
            public int toCore() {
                return LOST_NETWORK_VALUE;
            }

        };

        public static final int ACCOUNT_VALUE               = 0;
        public static final int SIGNATURE_VALUE             = 1;
        public static final int INSUFFICIENT_BALANCE_VALUE  = 2;
        public static final int INSUFFICIENT_NETWORK_FEE_VALUE       = 3;
        public static final int INSUFFICIENT_NETWORK_COST_UNIT_VALUE = 4;
        public static final int INSUFFICIENT_FEE_VALUE      = 5;
        public static final int NONCE_TOO_LOW_VALUE         = 6;
        public static final int NONCE_INVALID_VALUE         = 7;
        public static final int TRANSACTION_EXPIRED_VALUE   = 8;
        public static final int TRANSACTION_DUPLICATE_VALUE = 9;
        public static final int TRANSACTION_VALUE           = 10;
        public static final int UNKNOWN_VALUE               = 11;
        public static final int CLIENT_BAD_REQUEST_VALUE    = 12;
        public static final int CLIENT_PERMISSION_VALUE     = 13;
        public static final int CLIENT_RESOURCE_VALUE       = 14;
        public static final int CLIENT_BAD_RESPONSE_VALUE   = 15;
        public static final int CLIENT_UNAVAILABLE_VALUE    = 16;
        public static final int LOST_NETWORK_VALUE          = 17;

        private static WKTransferSubmitError.Type fromCore(int core) {
            switch (core) {
                case ACCOUNT_VALUE:               return ACCOUNT;
                case SIGNATURE_VALUE:             return SIGNATURE;
                case INSUFFICIENT_BALANCE_VALUE:  return INSUFFICIENT_BALANCE;
                case INSUFFICIENT_NETWORK_FEE_VALUE:       return INSUFFICIENT_NETWORK_FEE;
                case INSUFFICIENT_NETWORK_COST_UNIT_VALUE: return INSUFFICIENT_NETWORK_COST_UNIT;
                case INSUFFICIENT_FEE_VALUE:      return INSUFFICIENT_FEE;
                case NONCE_TOO_LOW_VALUE:         return NONCE_TOO_LOW;
                case NONCE_INVALID_VALUE:         return NONCE_INVALID;
                case TRANSACTION_EXPIRED_VALUE:   return TRANSACTION_EXPIRED;
                case TRANSACTION_DUPLICATE_VALUE: return TRANSACTION_DUPLICATE;
                case TRANSACTION_VALUE:           return TRANSACTION;
                case UNKNOWN_VALUE:               return UNKNOWN;
                case CLIENT_BAD_REQUEST_VALUE:    return CLIENT_BAD_REQUEST;
                case CLIENT_PERMISSION_VALUE:     return CLIENT_PERMISSION;
                case CLIENT_RESOURCE_VALUE:       return CLIENT_RESOURCE;
                case CLIENT_BAD_RESPONSE_VALUE:   return CLIENT_BAD_RESPONSE;
                case CLIENT_UNAVAILABLE_VALUE:    return CLIENT_UNAVAILABLE;
                case LOST_NETWORK_VALUE:          return LOST_NETWORK;
                default: throw new IllegalArgumentException("Invalid core value");
            }
        }

        public abstract int toCore();
    }

    private static int WK_TRANSFER_STATUS_DETAILS_LENGTH = (63+1);


    public int type;
    public byte[] details = new byte[WK_TRANSFER_STATUS_DETAILS_LENGTH];

    public WKTransferSubmitError() {
        super();
    }

    public WKTransferSubmitError(Pointer pointer) {
        super(pointer);
    }

    public WKTransferSubmitError(int type, byte[] details) {
        super();
        this.type = type;
        this.details = details;
    }

    public WKTransferSubmitError (Type type, String details) {
        super (WKNativeLibraryDirect.wkTransferSubmitErrorCreate(
                type.toCore(),
                details));
    }
     public Type getType () {
        return Type.fromCore(this.type);
    }

    public String getDetails () {
        return new String (this.details);
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("type", "details");
    }

    public static class ByReference extends WKTransferSubmitError implements Structure.ByReference {}

    public static class ByValue extends WKTransferSubmitError implements Structure.ByValue {}
}
