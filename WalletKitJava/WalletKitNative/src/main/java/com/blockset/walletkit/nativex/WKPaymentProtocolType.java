/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/29/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKPaymentProtocolType {

    BITPAY {
        @Override
        public int toCore() {
            return BITPAY_VALUE;
        }
    },

    BIP70 {
        @Override
        public int toCore() {
            return BIP70_VALUE;
        }
    };

    private static final int BITPAY_VALUE  = 0;
    private static final int BIP70_VALUE   = 1;

    public static WKPaymentProtocolType fromCore(int nativeValue) {
        switch (nativeValue) {
            case BITPAY_VALUE: return BITPAY;
            case BIP70_VALUE:  return BIP70;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
