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

import java.util.Date;
import javax.annotation.Nullable;

import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

/**
 * An AccountSpecification provides private data, notably a `paperKey`, needed to specify an
 * `Account`.  This data is loaded from a private file that is installed from a User's home
 * directory into the Java test target or the Android demo application
 */
public class AccountSpecification {

    // creators

    @JsonCreator
    public static AccountSpecification create(@JsonProperty("identifier") String identifer,
                                              @JsonProperty("network") String network,
                                              @JsonProperty("paperKey") String paperKey,
                                              @JsonProperty("timestamp") Date timestamp,
                                              @JsonProperty("content") @Nullable String content,
                                              @JsonProperty("hedera") @Nullable String hedera) {
        return new AccountSpecification(
                checkNotNull(identifer),
                checkNotNull(network),
                checkNotNull(paperKey),
                checkNotNull(timestamp),
                content,
                hedera
        );
    }

    // fields

    private final String identifier;
    private final String network;
    private final String paperKey;
    private final Date timestamp;
    private final String content;
    private final @Nullable String hedera;

    public AccountSpecification(String identifier,
                                String network,
                                String paperKey,
                                Date timestamp,
                                String content,
                                @Nullable String hedera) {
        this.identifier = identifier;
        this.network = network;
        this.paperKey = paperKey;
        this.timestamp = timestamp;
        this.content = content;
        this.hedera = hedera;
    }

    // getters

    @JsonProperty("identifier")
    public String getIdentifier() {
        return identifier;
    }

    @JsonProperty("network")
    public String getNetwork() {
        return network;
    }

    @JsonProperty("paperKey")
    public String getPaperKey() {
        return paperKey;
    }

    @JsonProperty("timestamp")
    public Date getTimestamp() {
        return timestamp;
    }

    @JsonProperty("content")
    public Optional<String> getContent() {
        return Optional.fromNullable(content);
    }

    @JsonProperty("hedera")
    public Optional<String> getHedera() {
        return Optional.fromNullable(hedera);
    }
}
