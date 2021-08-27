/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.errors;

public abstract class WalletConnectorError extends Exception {

    WalletConnectorError() { super(); }
    WalletConnectorError(String message) { super(message); }
    WalletConnectorError(Throwable throwable) { super(throwable); }

    public static class UnsupportedConnector extends WalletConnectorError {
        public UnsupportedConnector() { super(); }
        public UnsupportedConnector(String message) { super(message); }
        public UnsupportedConnector(Throwable throwable) { super(throwable); }
    }
    public static class UnknownEntity extends WalletConnectorError {
        public UnknownEntity() { super(); }
        public UnknownEntity(String message) { super(message); }
        public UnknownEntity(Throwable throwable) { super(throwable); }
    }
    public static class InvalidTransactionArguments extends WalletConnectorError {
        public InvalidTransactionArguments() { super(); }
        public InvalidTransactionArguments(String message) { super(message); }
        public InvalidTransactionArguments(Throwable throwable) { super(throwable); }
    }
    public static class InvalidTransactionSerialization extends WalletConnectorError {
        public InvalidTransactionSerialization() { super(); }
        public InvalidTransactionSerialization(String message) { super(message); }
        public InvalidTransactionSerialization(Throwable throwable) { super(throwable); }
    }
    public static class InvalidKeyForSigning extends WalletConnectorError {
        public InvalidKeyForSigning() { super(); }
        public InvalidKeyForSigning(String message) { super(message); }
        public InvalidKeyForSigning(Throwable throwable) { super(throwable); }
    }
    public static class UnrecoverableKey extends WalletConnectorError {
        public UnrecoverableKey() { super(); }
        public UnrecoverableKey(String message) { super(message); }
        public UnrecoverableKey(Throwable throwable) { super(throwable); }
    }
    public static class UnsignedTransaction extends WalletConnectorError {
        public UnsignedTransaction() { super(); }
        public UnsignedTransaction(String message) { super(message); }
        public UnsignedTransaction(Throwable throwable) { super(throwable); }
    }
    public static class SubmitFailed extends WalletConnectorError {
        public SubmitFailed() { super(); }
        public SubmitFailed(String message) { super(message); }
        public SubmitFailed(Throwable throwable) { super(throwable); }
    }
}
