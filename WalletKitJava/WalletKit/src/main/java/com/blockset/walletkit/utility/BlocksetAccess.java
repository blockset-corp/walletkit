/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.utility;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

/**
 * A BlocksetAccess provides the `baseURL` and `token` that is needed to connect to Blockset.  This
 * is loaded from a private file that is installed from a User's home directory into the Java
 * test target or the Android demo application
 */
public class BlocksetAccess {

    // creators

    @JsonCreator
    public static BlocksetAccess create(@JsonProperty("baseURL") String baseURL,
                                        @JsonProperty("token") String token) {
        return new BlocksetAccess(
                checkNotNull(baseURL),
                checkNotNull(token)
                );
    }

    // fields

    private final String baseURL;
    private final String token;

    public BlocksetAccess(String baseURL,
                          String token) {
        this.baseURL = baseURL;
        this.token = token;
    }

    // getters

    @JsonProperty("baseURL")
    public String getBaseURL() {
        return baseURL;
    }

    @JsonProperty("token")
    public String getToken() {
        return token;
    }
}
