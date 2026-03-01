/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.errors;

import androidx.annotation.Nullable;

public abstract class SystemClientError extends Exception {
    public interface Visitor<T> {
        T visit(BadRequest  error);
        T visit(Permission  error);
        T visit(Resource    error);
        T visit(Submission  error);
        T visit(BadResponse error);
        T visit(Unavailable error);
        T visit(LostConnectivity error);
    }

    public abstract static class DefaultVisitor<T> implements Visitor<T> {
        @Nullable public T visit(BadRequest error)  { return null; }
        @Nullable public T visit(Permission error)  { return null; }
        @Nullable public T visit(Resource error)    { return null; }
        @Nullable public T visit(Submission error)  { return null; }
        @Nullable public T visit(BadResponse error) { return null; }
        @Nullable public T visit(Unavailable error) { return null; }
        @Nullable public T visit(LostConnectivity error) { return null; }
     }

    public abstract <T> T accept(Visitor<T> visitor);

    protected SystemClientError() {
        super();
    }

    public static class BadRequest extends SystemClientError {
        public final String details;

        public BadRequest(String details) {
            super();
            this.details = details;
        }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }

        @Override
        public String toString() {
            return "BadRequest{" +
                    "details='" + details + '\'' +
                    '}';
        }
    }

    public static class Permission extends SystemClientError {
        public Permission() { super (); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }

        @Override
        public String toString() {
            return "Permission{}";
        }
    }

    public static class Resource extends SystemClientError {
        public Resource() { super (); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }

        @Override
        public String toString() {
            return "Resource{}";
        }
    }

    public static class BadResponse extends SystemClientError {
        public final String details;

        public BadResponse(String details) {
            super ();
            this.details = details;
        }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }

        @Override
        public String toString() {
            return "BadResponse{" +
                    "details='" + details + '\'' +
                    '}';
        }
    }

    public static class Submission extends SystemClientError {
        public final SystemClientSubmitError error;

        public Submission(SystemClientSubmitError error) {
            super ();
            this.error   = error;
        }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }

        @Override
        public String toString() {
            return "Submission{" + error + '}';
        }
    }

    public static class Unavailable extends SystemClientError {
        public Unavailable() { super (); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }

        @Override
        public String toString() {
            return "Unavailable{}";
        }
    }

    public static class LostConnectivity extends SystemClientError {
        public LostConnectivity() { super (); }
        public <T> T accept(Visitor<T> visitor) {
            return visitor.visit(this);
        }

        @Override
        public String toString() {
            return "LostConnectivity{}";
        }
    }
}
