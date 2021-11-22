package com.blockset.walletkit;

import javax.annotation.Nullable;

public class TransferSubmitError {
    public enum Type {
        ACCOUNT,
        SIGNATURE,
        INSUFFICIENT_BALANCE,
        INSUFFICIENT_NETWORK_FEE,
        INSUFFICIENT_NETWORK_COST_UNIT,
        INSUFFICIENT_FEE,
        NONCE_TOO_LOW,
        NONCE_INVALID,
        TRANSACTION_EXPIRED,
        TRANSACTION_DUPLICATE,
        TRANSACTION,
        UNKNOWN,
        CLIENT_BAD_REQUEST,
        CLIENT_PERMISSION,
        CLIENT_RESOURCE,
        CLIENT_BAD_RESPONSE,
        CLIENT_UNAVAILABLE,
        LOST_NETWORK;
    }

    Type type;
    @Nullable String details;

    public TransferSubmitError(Type type, @Nullable String details) {
        this.type = type;
        this.details = details;
    }
}
