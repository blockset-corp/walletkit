/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKNetworkType {

    BTC {
        @Override
        public int toCore() {
            return BTC_VALUE;
        }
    },

    BCH {
        @Override
        public int toCore() {
            return BCH_VALUE;
        }
    },

    BSV {
        @Override
        public int toCore() {
            return BSV_VALUE;
        }
    },

    LTC {
        @Override
        public int toCore() {
            return LTC_VALUE;
        }
    },

    DOGE {
        @Override
        public int toCore() {
            return DOGE_VALUE;
        }
    },

    ETH {
        @Override
        public int toCore() {
            return ETH_VALUE;
        }
    },

    XRP {
        @Override
        public int toCore() {
            return XRP_VALUE;
        }
    },

    HBAR {
        @Override
        public int toCore() {
            return HBAR_VALUE;
        }
    },

    XTZ {
        @Override
        public int toCore() {
            return XTZ_VALUE;
        }
    },

    XLM {
        @Override
        public int toCore() {
            return XLM_VALUE;
        }
    }/* New __SYMBOL__ toCore() */;

    private static final int BTC_VALUE = 0;
    private static final int BCH_VALUE = 1;
    private static final int BSV_VALUE = 2;
    private static final int LTC_VALUE = 3;
    private static final int DOGE_VALUE = 4;
    private static final int ETH_VALUE = 5;
    private static final int XRP_VALUE = 6;
    private static final int HBAR_VALUE = 7;
    private static final int XTZ_VALUE = 8;
    private static final int XLM_VALUE = 9;
    /* private static final int __SYMBOL___VALUE = 10 */

    public static WKNetworkType fromCore(int nativeValue) {
        switch (nativeValue) {
            case BTC_VALUE: return BTC;
            case BCH_VALUE: return BCH;
            case BSV_VALUE: return BSV;
            case LTC_VALUE: return LTC;
            case DOGE_VALUE:return DOGE;
            case ETH_VALUE: return ETH;
            case XRP_VALUE: return XRP;
            case HBAR_VALUE:return HBAR;
            case XTZ_VALUE: return XTZ;
            case XLM_VALUE: return XLM;
            /* case __SYMBOL___VALUE: return __SYMBOL__; */
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
