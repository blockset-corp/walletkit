/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/29/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKPaymentProtocolError {

    NONE {
        @Override
        public int toCore() {
            return NONE_VALUE;
        }
    },

    CERT_MISSING {
        @Override
        public int toCore() {
            return CERT_MISSING_VALUE;
        }
    },

    CERT_NOT_TRUSTED {
        @Override
        public int toCore() {
            return CERT_NOT_TRUSTED_VALUE;
        }
    },

    SIGNATURE_TYPE_NOT_SUPPORTED {
        @Override
        public int toCore() {
            return SIGNATURE_TYPE_NOT_SUPPORTED_VALUE;
        }
    },

    SIGNATURE_VERIFICATION_FAILED {
        @Override
        public int toCore() {
            return SIGNATURE_VERIFICATION_FAILED_VALUE;
        }
    },

    EXPIRED {
        @Override
        public int toCore() {
            return EXPIRED_VALUE;
        }
    };

    private static final int NONE_VALUE                           = 0;
    private static final int CERT_MISSING_VALUE                   = 1;
    private static final int CERT_NOT_TRUSTED_VALUE               = 2;
    private static final int SIGNATURE_TYPE_NOT_SUPPORTED_VALUE   = 3;
    private static final int SIGNATURE_VERIFICATION_FAILED_VALUE  = 4;
    private static final int EXPIRED_VALUE                        = 5;

    public static WKPaymentProtocolError fromCore(int nativeValue) {
        switch (nativeValue) {
            case NONE_VALUE:                          return NONE;
            case CERT_MISSING_VALUE:                  return CERT_MISSING;
            case CERT_NOT_TRUSTED_VALUE:              return CERT_NOT_TRUSTED;
            case SIGNATURE_TYPE_NOT_SUPPORTED_VALUE:  return SIGNATURE_TYPE_NOT_SUPPORTED;
            case SIGNATURE_VERIFICATION_FAILED_VALUE: return SIGNATURE_VERIFICATION_FAILED;
            case EXPIRED_VALUE:                       return EXPIRED;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
