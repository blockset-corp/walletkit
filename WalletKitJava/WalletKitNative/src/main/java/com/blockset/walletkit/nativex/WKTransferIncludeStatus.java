package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

public class WKTransferIncludeStatus extends Structure {
    public enum Type {
        SUCCESS {
            @Override
            public int toCore() {
                return SUCCESS_VALUE;
            }
        },

        FAILURE_INSUFFICIENT_NETWORK_COST_UNIT {
            @Override
            public int toCore() {
                return FAILURE_INSUFFICIENT_NETWORK_COST_UNIT_VALUE;
            }
        },

        FAILURE_REVERTED {
            @Override
            public int toCore() {
                return FAILURE_REVERTED_VALUE;
            }
        },

        FAILURE_UNKNOWN {
            @Override
            public int toCore() {
                return FAILURE_UNKNOWN_VALUE;
            }
        };

        public static final int SUCCESS_VALUE          = 0;
        public static final int FAILURE_INSUFFICIENT_NETWORK_COST_UNIT_VALUE = 1;
        public static final int FAILURE_REVERTED_VALUE = 2;
        public static final int FAILURE_UNKNOWN_VALUE  = 3;

        public static WKTransferIncludeStatus.Type fromCore(int core) {
            switch (core) {
                case SUCCESS_VALUE:          return SUCCESS;
                case FAILURE_INSUFFICIENT_NETWORK_COST_UNIT_VALUE: return FAILURE_INSUFFICIENT_NETWORK_COST_UNIT;
                case FAILURE_REVERTED_VALUE: return FAILURE_REVERTED;
                case FAILURE_UNKNOWN_VALUE:  return FAILURE_UNKNOWN;
                default: throw new IllegalArgumentException("Invalid core value");
            }
        }

        public abstract int toCore();
    }

    private static final int WK_TRANSFER_STATUS_DETAILS_LENGTH = (127+1);

    public int type;
    public byte[] details = new byte[WK_TRANSFER_STATUS_DETAILS_LENGTH];

    public WKTransferIncludeStatus() {
        super();
    }

    public WKTransferIncludeStatus(Pointer pointer) {
        super(pointer);
    }

    public WKTransferIncludeStatus(int type, byte[] details) {
        super();
        this.type = type;
        this.details = details;
    }

    public WKTransferIncludeStatus(Type type, String details) {
        super(type == Type.SUCCESS
                ? WKNativeLibraryDirect.wkTransferIncludeStatusCreateSuccess()
                : WKNativeLibraryDirect.wkTransferIncludeStatusCreateFailure(
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

    public static class ByReference extends WKTransferIncludeStatus implements Structure.ByReference {
    }

    public static class ByValue extends WKTransferIncludeStatus implements Structure.ByValue {
    }
}
