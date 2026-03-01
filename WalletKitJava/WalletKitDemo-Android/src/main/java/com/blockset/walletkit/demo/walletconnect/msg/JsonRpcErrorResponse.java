package com.blockset.walletkit.demo.walletconnect.msg;

import androidx.annotation.Nullable;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.SerializerProvider;
import com.fasterxml.jackson.databind.annotation.JsonSerialize;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ser.std.StdSerializer;
import com.fasterxml.jackson.databind.type.TypeFactory;
import com.fasterxml.jackson.databind.util.Converter;
import com.fasterxml.jackson.core.type.TypeReference;

import static com.google.common.base.Preconditions.checkNotNull;

import java.io.IOException;

public class JsonRpcErrorResponse {

    public final static String WALLET_CONNECT_1_0_RPC_VERSION = "2.0";

    @JsonCreator
    public static JsonRpcErrorResponse create(@JsonProperty("id")       Number          id,
                                              @JsonProperty("jsonrpc")  String          jsonRpc,
                                              @JsonProperty("error")    JsonRpcError    error) {
        return new JsonRpcErrorResponse(id,
                                        checkNotNull(jsonRpc),
                                        error);
    }

    private final Number    id;
    private final String    jsonRpc;
    private final           JsonRpcError  error;

    private JsonRpcErrorResponse( Number              id,
                                  String              jsonRpc,
                                  JsonRpcError        error) {
        this.id = id;
        this.jsonRpc = jsonRpc;
        this.error = error;
    }

    // getters

    @JsonProperty("id")
    public Number getId() { return id; }

    @JsonProperty("jsonrpc")
    public String getJsonRpc() { return jsonRpc; }

    @JsonProperty("error")
    public JsonRpcError getError() { return error; }
}
