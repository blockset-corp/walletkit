package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKClientError extends PointerType {
    public enum Type {
        BAD_REQUEST {
            @Override
            public int toCore() {
                return BAD_REQUEST_VALUE;
            }
        },

        PERMISSION {
            @Override
            public int toCore() {
                return PERMISSION_VALUE;
            }
        },

        RESOURCE {
            @Override
            public int toCore() {
                return RESOURCE_VALUE;
            }
        },

        BAD_RESPONSE {
            @Override
            public int toCore() {
                return BAD_RESPONSE_VALUE;
            }
        },

        SUBMISSION {
            @Override
            public int toCore() {
                return SUBMISSION_VALUE;
            }
        },

        UNAVAILABLE {
            @Override
            public int toCore() {
                return UNAVAILABLE_VALUE;
            }
        },

        LOST_CONNECTIVITY {
            @Override
            public int toCore() {
                return LOST_CONNECTIVITY_VALUE;
            }
        };

        public static final int BAD_REQUEST_VALUE  = 0;
        public static final int PERMISSION_VALUE   = 1;
        public static final int RESOURCE_VALUE     = 2;
        public static final int BAD_RESPONSE_VALUE = 3;
        public static final int SUBMISSION_VALUE   = 4;
        public static final int UNAVAILABLE_VALUE  = 5;
        public static final int LOST_CONNECTIVITY_VALUE = 6;

        public static Type fromCore(int core) {
            switch (core) {
                case BAD_REQUEST_VALUE:  return BAD_REQUEST;
                case PERMISSION_VALUE:   return PERMISSION;
                case RESOURCE_VALUE:     return RESOURCE;
                case BAD_RESPONSE_VALUE: return BAD_RESPONSE;
                case SUBMISSION_VALUE:   return SUBMISSION;
                case UNAVAILABLE_VALUE:  return UNAVAILABLE;
                case LOST_CONNECTIVITY_VALUE: return LOST_CONNECTIVITY;
                default: throw new IllegalArgumentException("Invalid core value");
            }
        }

        public abstract int toCore();
    }

    public WKClientError() { super(); }

    public WKClientError(Pointer address) { super (address); }

    public WKClientError (Type type, String details) {
        super (WKNativeLibraryDirect.wkClientErrorCreate(
                type.toCore(),
                details));
    }

    public WKClientError (WKTransferSubmitError.Type type, String details) {
        super (WKNativeLibraryDirect.wkClientErrorCreateSubmission(
                type.toCore(),
                details));
    }
    
    public Type type() {
        return Type.fromCore(
                WKNativeLibraryDirect.wkClientErrorGetType(
                        this.getPointer()));
    }
}
