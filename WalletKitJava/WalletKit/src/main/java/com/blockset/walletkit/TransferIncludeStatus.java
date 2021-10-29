package com.blockset.walletkit;

import javax.annotation.Nullable;

public class TransferIncludeStatus {
    public enum Type {
        SUCCESS,
        INSUFFICIENT_NETWORK_COST_UNIT,
        REVERTED,
        UNKNOWN;
    }

    public final Type type;
    public final @Nullable String details;

    public TransferIncludeStatus(Type type, @Nullable String details) {
        this.type = type;
        this.details = details;
    }
}
