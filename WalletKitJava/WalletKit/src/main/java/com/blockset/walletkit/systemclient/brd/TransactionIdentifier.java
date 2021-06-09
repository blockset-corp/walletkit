/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.systemclient.brd;
import android.support.annotation.Nullable;

import com.blockset.walletkit.SystemClient;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

public class TransactionIdentifier implements SystemClient.TransactionIdentifier {
    // creator

    @JsonCreator
    public static TransactionIdentifier create(@JsonProperty("transaction_id") String id,
                                               @JsonProperty("identifier") String identifier,
                                               @JsonProperty("hash") @Nullable String hash,
                                               @JsonProperty("blockchain_id") String blockchainId) {
        return new TransactionIdentifier(
                checkNotNull(id),
                checkNotNull(identifier),
                hash,
                checkNotNull(blockchainId)
        );
    }

    // fields

    private final String id;
    private final String identifier;
    private final @Nullable String hash;
    private final String blockchainId;

    private TransactionIdentifier(String id,
                                  String identifier,
                                  @Nullable String hash,
                                  String blockchainId) {
        this.id = id;
        this.identifier = identifier;
        this.hash = hash;
        this.blockchainId = blockchainId;
    }
    // getters

    @JsonProperty("transaction_id")
    public String getId() {
        return id;
    }

    @JsonProperty("identifier")
    public String getIdentifier() {
        return identifier;
    }

    @JsonProperty("hash")
    public Optional<String> getHash() {
        return Optional.fromNullable (hash);
    }

    @JsonProperty("blockchain_id")
    public String getBlockchainId() {
        return blockchainId;
    }
}
