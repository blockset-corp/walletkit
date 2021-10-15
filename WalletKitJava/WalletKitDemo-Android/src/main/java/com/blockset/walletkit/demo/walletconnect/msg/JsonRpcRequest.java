package com.blockset.walletkit.demo.walletconnect.msg;

import androidx.annotation.Nullable;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JavaType;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.SerializerProvider;
import com.fasterxml.jackson.databind.annotation.JsonDeserialize;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.annotation.JsonSerialize;
import com.fasterxml.jackson.databind.node.JsonNodeType;
import com.fasterxml.jackson.databind.ser.std.StdSerializer;
import com.fasterxml.jackson.databind.type.TypeFactory;
import com.fasterxml.jackson.databind.util.Converter;
import com.fasterxml.jackson.core.type.TypeReference;
import com.google.common.base.Optional;

import java.io.IOException;
import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class JsonRpcRequest {

    public final static String WALLET_CONNECT_1_0_RPC_VERSION = "2.0";

    // WalletConnect 1.0 spec does not mention this 'silent' flag but the sample
    // dApp has it
    @JsonCreator
    public static JsonRpcRequest create(@JsonProperty("id")         Number      id,
                                        @JsonProperty("jsonrpc")    String      jsonRpc,
                                        @JsonProperty("method") @Nullable     String      method,
                                        @JsonProperty("params")     List<String>      params) {
        return new JsonRpcRequest(id,
                                  checkNotNull(jsonRpc),
                                  method,
                                  checkNotNull(params));
    }

    private final           Number    id;
    private final           String    jsonRpc;
    private final @Nullable String    method;

    /// Deserialization of array of Json where the Json represents the
    /// params: any[]; of the JsonRpcRequest struct, is problematic based
    /// strictly on annotations. Have to use a custom content convertor in
    /// this case to convert the any[] into array of strings.
    @JsonDeserialize(contentConverter = JsonRpcRequest.RawParamsToString.class)

    /// Serialization of array of Json where the individual elements themselves
    /// may be JSON. Same as what is done in JsonRpcResponse
    @JsonSerialize(using = JsonRpcRequest.RawParamsToJSON.class)
    private final           List<String>  params;

    public static class RawParamsToJSON extends StdSerializer<List<String>> {
        public RawParamsToJSON() {
            this(null);
        }
        public RawParamsToJSON(Class<List<String>> n) {
            super(n);
        }

        private void serializeSingleParam(String param,
                                          JsonGenerator gen) throws JsonProcessingException, IOException {
            ObjectMapper mapper = new ObjectMapper();
            JsonNode nodeFromString = mapper.readTree(param);
            String paramJson = mapper.writeValueAsString(nodeFromString);
            gen.writeRaw(paramJson);
        }

        @Override
        public void serialize(List<String> params,
                              JsonGenerator gen,
                              SerializerProvider provider) throws IOException {
            ObjectMapper mapper = new ObjectMapper();
            gen.writeRaw(":");
            if (params == null) {
                gen.writeRaw("null");
                return;
            }
            gen.writeRaw("[");
            try {
                for (int i=0;i < params.size() - 1; i++) {
                    serializeSingleParam(params.get(i), gen);
                    gen.writeRaw(",");
                }
                serializeSingleParam(params.get(params.size() - 1), gen);

            } catch (JsonProcessingException jpe) {
                /// ...
            }
            gen.writeRaw("]");
        }
    }
    public static class RawParamsToString implements Converter<JsonNode, String> {

        public RawParamsToString() {
        }

        @Override
        public String convert(JsonNode value) {
            // Another JSON
            if (value.getNodeType() == JsonNodeType.OBJECT)
                return value.toString();
            if (value.getNodeType() == JsonNodeType.STRING)
                return value.asText();
            return value.toString();
        }

        @Override
        public JavaType getInputType(TypeFactory typeFactory) {
            return typeFactory.constructType(new TypeReference<JsonNode>(){});
        }

        @Override
        public JavaType getOutputType(TypeFactory typeFactory) {
            return typeFactory.constructType(new TypeReference<String>() {});
        }
    }

    private JsonRpcRequest( Number              id,
                            String              jsonRpc,
                            @Nullable String    method,
                            List<String>              params) {
        this.id = id;
        this.jsonRpc = jsonRpc;
        this.method = method;
        this.params = params; // The params field is itself a nested Json but the interpretation
                              // based on 'method'
    }

    // getters

    @JsonProperty("id")
    public Number getId() { return id; }

    @JsonProperty("jsonrpc")
    public String getJsonRpc() { return jsonRpc; }

    @JsonProperty("method")
    public Optional<String> getMethod() { return Optional.fromNullable(method); }

    @JsonProperty("params")
    public List<String> getParams() { return params; }

    public String toString() {
        return  "    id: " + id + "\n" +
                "    version: " + jsonRpc + "\n" +
                "    method: " + method;
    }
}
