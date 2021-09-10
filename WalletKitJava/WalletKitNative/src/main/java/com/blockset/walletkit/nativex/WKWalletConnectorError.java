/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

public enum WKWalletConnectorError {

    /** Cannot support WalletConnect on this network
     *
     */
    UNSUPPORTED_CONNECTOR {
        @Override
        public int toCore() {
            return WK_WALLET_CONNECTOR_STATUS_UNSUPPORTED_CONNECTOR_VALUE;
        }
    },
    
    /** Indicates that internal connector setup is incorrect.
     */
    ILLEGAL_OPERATION {
        @Override
        public int toCore() {
            return WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION_VALUE;
        }
    },
    
    /** Indicates that one or more required transaction arguments
     *  have not been supplied.
     */
    INVALID_TRANSACTION_ARGUMENTS {
        @Override
        public int toCore() {
            return WK_WALLET_CONNECTOR_STATUS_INVALID_TRANSACTION_ARGUMENTS_VALUE;
        }
    },

    /** Indicates that the digest is invalid or failed to be produced.
     *
     */
    INVALID_DIGEST {
        @Override
        public int toCore() {
            return WK_WALLET_CONNECTOR_STATUS_INVALID_DIGEST_VALUE;
        }
    },

    /** Indicates that the signature is invalid or failed to be completed.
     */
    INVALID_SIGNATURE {
        @Override
        public int toCore() {
            return WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE_VALUE;
        }
    },

    /** Indicates that the serialization returned is invalid or failed to process.
     */
    INVALID_SERIALIZATION {
        @Override
        public int toCore() {
            return WK_WALLET_CONNECTOR_STATUS_INVALID_SERIALIZATION_VALUE;
        }
    };
    
    public static WKWalletConnectorError fromCore(int nativeValue) {
        switch (nativeValue) {
            case WK_WALLET_CONNECTOR_STATUS_UNSUPPORTED_CONNECTOR_VALUE:            return UNSUPPORTED_CONNECTOR;
            case WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION_VALUE:                return ILLEGAL_OPERATION;
            case WK_WALLET_CONNECTOR_STATUS_INVALID_TRANSACTION_ARGUMENTS_VALUE:    return INVALID_TRANSACTION_ARGUMENTS;
            case WK_WALLET_CONNECTOR_STATUS_INVALID_DIGEST_VALUE:                   return INVALID_DIGEST;
            case WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE_VALUE:                return INVALID_SIGNATURE;
            case WK_WALLET_CONNECTOR_STATUS_INVALID_SERIALIZATION_VALUE:            return INVALID_SERIALIZATION;

            case WK_WALLET_CONNECTOR_STATUS_OK_VALUE: throw new IllegalArgumentException("Invalid WKWalletConnectorError for Success");
            default: throw new IllegalArgumentException("Invalid WKWalletConnectorError core value");
        }
    }

    // Valid WalletConnect native errors follow
    private static final int WK_WALLET_CONNECTOR_STATUS_OK_VALUE                            = 0;
    private static final int WK_WALLET_CONNECTOR_STATUS_UNSUPPORTED_CONNECTOR_VALUE         = 1;
    private static final int WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION_VALUE             = 2;
    private static final int WK_WALLET_CONNECTOR_STATUS_INVALID_TRANSACTION_ARGUMENTS_VALUE = 3;
    private static final int WK_WALLET_CONNECTOR_STATUS_INVALID_DIGEST_VALUE                = 4;
    private static final int WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE_VALUE             = 5;
    private static final int WK_WALLET_CONNECTOR_STATUS_INVALID_SERIALIZATION_VALUE         = 6;

    public abstract int toCore();
}
