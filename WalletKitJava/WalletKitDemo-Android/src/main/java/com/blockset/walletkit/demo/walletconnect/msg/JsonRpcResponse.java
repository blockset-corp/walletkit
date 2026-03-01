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

public class JsonRpcResponse {

    public final static String WALLET_CONNECT_1_0_RPC_VERSION = "2.0";

    @JsonCreator
    public static JsonRpcResponse create(@JsonProperty("id")        Number      id,
                                        @JsonProperty("jsonrpc")    String      jsonRpc,
                                        @JsonProperty("result")     String      result) {
        return new JsonRpcResponse(id,
                                  checkNotNull(jsonRpc),
                                  checkNotNull(result));
    }

    private final Number    id;
    private final String    jsonRpc;

    // N.B. The 'result' field is 'any' JSON...
    //
    // Facilitate adding an object of any type OR a simple string.
    // Accordingly treat it the field value as a string so it can
    // be assigned any JSON serialization on response, and use a custom serializer
    // to write out the contents properly, as JSON string, OR string to the generator
    @JsonSerialize(using = JsonRpcResponse.RawResultToJSON.class)
    private final           String  result;


    public static class RawResultToJSON extends StdSerializer<String> {

        public RawResultToJSON() {
            this(null);
        }
        public RawResultToJSON(Class<String> n) {
            super(n);
        }

        @Override
        public void serialize(String value, JsonGenerator gen, SerializerProvider provider) throws IOException {

            // Start with the assumption that the 'result' might be a simple value like a plain string, number etc
            // rather than a JSON object.
            ObjectMapper mapper = new ObjectMapper();
            String json = mapper.writeValueAsString(value);
            try {
                JsonNode nodeFromString = mapper.readTree(value);
                json = mapper.writeValueAsString(nodeFromString);
            } catch (JsonProcessingException jpe) {
                /// ... if the 'value' is not indeed JSON
            }

            // When writing to generator as string, the generator will escape
            // all JSON delimiters, leading to bad JSON. Writing raw prevents
            // escaping, but have to include a ":" to denote 'result' is a JSON
            // field
            gen.writeRaw(":" + json);
        }
    }

    private JsonRpcResponse( Number              id,
                            String              jsonRpc,
                            String              result) {
        this.id = id;
        this.jsonRpc = jsonRpc;
        this.result = result; // This field is an arbitrary 'any' JSON
                              // result structure and is treated as a string
    }

    // getters

    @JsonProperty("id")
    public Number getId() { return id; }

    @JsonProperty("jsonrpc")
    public String getJsonRpc() { return jsonRpc; }

    @JsonProperty("result")
    public String getResult() { return result; }
}
