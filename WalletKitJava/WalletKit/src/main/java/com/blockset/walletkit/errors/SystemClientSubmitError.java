/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 9/18/19.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.errors;

import androidx.annotation.Nullable;

public abstract class SystemClientSubmitError extends Exception {
    public interface Visitor<T> {
        T visit(Access error);
        T visit(Account error);
        T visit(Signature error);
        T visit(InsufficientBalance error);
        T visit(InsufficientNetworkFee error);
        T visit(InsufficientNetworkCostUnit error);
        T visit(InsufficientFee error);
        T visit(NonceTooLow error);
        T visit(NonceInvalid error);
        T visit(TransactionExpired error);
        T visit(TransactionDuplicate error);
        T visit(Transaction error);
        T visit(Unknown error);
    }

    public abstract static class DefaultVisitor<T> implements Visitor<T> {
        @Nullable public T visit(Access error) { return null; }
        @Nullable public T visit(Account error) { return null; }
        @Nullable public T visit(Signature error) { return null; }
        @Nullable public T visit(InsufficientBalance error) { return null; }
        @Nullable public T visit(InsufficientNetworkFee error) { return null; }
        @Nullable public T visit(InsufficientNetworkCostUnit error) { return null; }
        @Nullable public T visit(InsufficientFee error) { return null; }
        @Nullable public T visit(NonceTooLow error) { return null; }
        @Nullable public T visit(NonceInvalid error) { return null; }
        @Nullable public T visit(TransactionExpired error) { return null; }
        @Nullable public T visit(TransactionDuplicate error) { return null; }
        @Nullable public T visit(Transaction error) { return null; }
        @Nullable public T visit(Unknown error) { return null; }
    }

    public abstract <T> T accept(Visitor<T> visitor);

    /* package */
    public String details;

    SystemClientSubmitError(String details) {
        super();
        this.details = details;
    }

    @Override
    public String toString() {
        return this.getClass().getSimpleName() + "{" + "details='" + details + '\'' + '}';
    }

    public static class Access extends SystemClientSubmitError {
        public Access(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class Account extends SystemClientSubmitError {
        public Account(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class Signature extends SystemClientSubmitError {
        public Signature(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class InsufficientBalance extends SystemClientSubmitError {
        public InsufficientBalance(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class InsufficientNetworkFee extends SystemClientSubmitError {
        public InsufficientNetworkFee(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class InsufficientNetworkCostUnit extends SystemClientSubmitError {
        public InsufficientNetworkCostUnit(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class InsufficientFee extends SystemClientSubmitError {
        public InsufficientFee(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class NonceTooLow extends SystemClientSubmitError {
        public NonceTooLow(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class NonceInvalid extends SystemClientSubmitError {
        public NonceInvalid(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class TransactionExpired extends SystemClientSubmitError {
        public TransactionExpired(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class TransactionDuplicate extends SystemClientSubmitError {
        public TransactionDuplicate(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class Transaction extends SystemClientSubmitError {
        public Transaction(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }

    public static class Unknown extends SystemClientSubmitError {
        public Unknown(String details) { super (details); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }
    }
}
