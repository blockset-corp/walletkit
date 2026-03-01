package com.blockset.walletkit.demo.walletconnect.msg;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

public class JsonRpcError {

    // Can only find this about JSON RPC error codes:
    // https://eth.wiki/json-rpc/json-rpc-error-codes-improvement-proposal
    // The WalletConnect samples use the below SERVER_ERROR for indicating
    // failures plus a reason messagE.
    public static final int SERVER_ERROR = -32000;

    @JsonCreator
    public static JsonRpcError create(@JsonProperty("code")    Number      code,
                               @JsonProperty("message") String      message) {
        return new JsonRpcError(checkNotNull(code),
                            checkNotNull(message));
    }

    private final Number    code;
    private final String    message;

    private JsonRpcError(Number     code,
                     String     message) {
        this.code = code;
        this.message = message;
    }

    // getters

    @JsonProperty("code")
    public Number getCode() { return code; }

    @JsonProperty("message")
    public String getMessage() { return message; }
}
