/*
 * Created by Ehsan Rezaie <ehsan@brd.com> on 11/23/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

public enum BRCryptoExportablePaperWalletStatus {

    CRYPTO_EXPORTABLE_PAPER_WALLET_SUCCESS {
        @Override
        public int toCore() {
            return CRYPTO_EXPORTABLE_PAPER_WALLET_SUCCESS_VALUE;
        }
    },

    CRYPTO_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY {
        @Override
        public int toCore() {
            return CRYPTO_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY_VALUE;
        }
    },

    CRYPTO_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS {
        @Override
        public int toCore() {
            return CRYPTO_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS_VALUE;
        }
    },

    CRYPTO_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION  {
        @Override
        public int toCore() {
            return CRYPTO_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION_VALUE;
        }
    };

    private static final int CRYPTO_EXPORTABLE_PAPER_WALLET_SUCCESS_VALUE                = 0;
    private static final int CRYPTO_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY_VALUE   = 1;
    private static final int CRYPTO_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS_VALUE      = 2;
    private static final int CRYPTO_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION_VALUE      = 3;

    public static BRCryptoExportablePaperWalletStatus fromCore(int nativeValue) {
        switch (nativeValue) {
            case CRYPTO_EXPORTABLE_PAPER_WALLET_SUCCESS_VALUE:               return CRYPTO_EXPORTABLE_PAPER_WALLET_SUCCESS;
            case CRYPTO_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY_VALUE:  return CRYPTO_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY;
            case CRYPTO_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS_VALUE:     return CRYPTO_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS;
            case CRYPTO_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION_VALUE:     return CRYPTO_EXPORTABLE_PAPER_WALLET_ILLEGAL_OPERATION;
            default: throw new IllegalArgumentException("Invalid core value");
        }
    }

    public abstract int toCore();
}
