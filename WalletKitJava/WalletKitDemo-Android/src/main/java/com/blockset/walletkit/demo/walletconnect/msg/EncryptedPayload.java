package com.blockset.walletkit.demo.walletconnect.msg;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

public class EncryptedPayload {

    @JsonCreator
    public static EncryptedPayload create(@JsonProperty("data")     String      data,
                                          @JsonProperty("hmac")     String      dmac,
                                          @JsonProperty("iv")       String      iv) {

        return new EncryptedPayload(checkNotNull(data),
                             checkNotNull(dmac),
                             checkNotNull(iv));
    }

    private final String    data;
    private final String    hmac;
    private final String    iv;

    private EncryptedPayload(   String data,
                                String hmac,
                                String iv ) {
        this.data = data;
        this.hmac = hmac;
        this.iv = iv;
    }

    // getters

    @JsonProperty("data")
    public String getData() { return data; }

    @JsonProperty("hmac")
    public String getHmac() { return hmac; }

    @JsonProperty("iv")
    public String getIv() { return iv; }

    public String toString() {
        return  "\n    data: " + data + "\n" +
                "    hmac: " + hmac + "\n" +
                "    iv: " + iv;
    }
}
