package com.blockset.walletkit.demo.walletconnect.msg;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

// See .../walletconnect-test-wallet/node_modules/@walletconnect/socket-transport/dist/cjs
public class SocketMessage {

    public static final String SUBSCRIBE_TYPE   = "sub";
    public static final String PUBLISH_TYPE     = "pub";
    public static final String NO_PAYLOAD       = "";

    // WalletConnect 1.0 spec does not mention this 'silent' flag but the sample
    // dApp has it
    @JsonCreator
    public static SocketMessage create(@JsonProperty("topic")   String      topic,
                                       @JsonProperty("type")    String      type,
                                       @JsonProperty("payload") String      payload,
                                       @JsonProperty("silent")  boolean     silent) {

        return new SocketMessage(checkNotNull(topic),
                             checkNotNull(type),
                             checkNotNull(payload),
                             silent);
    }

    private final String    topic;
    private final String    type;
    private final String    payload;
    private final boolean   silent;

    private SocketMessage(String topic,
                      String type,
                      String payload,
                      boolean silent) {
        this.topic = topic;
        this.type = type;
        this.payload = payload;
        this.silent = silent;
    }

    // getters

    @JsonProperty("topic")
    public String getTopic() { return topic; }

    @JsonProperty("type")
    public String getType() { return type; }

    @JsonProperty("payload")
    public String getPayload() { return payload; }

    @JsonProperty("silent")
    public boolean getSilent() { return silent; }

    public String toString() {
        return  "    Type: " + type + "\n" +
                "    Topic: " + topic + "\n" +
                (silent ? "    Silent" : "") + "\n" +
                "    Payload: " + payload;
    }
}
