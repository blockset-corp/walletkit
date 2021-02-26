package com.breadwallet.crypto.blockchaindb.models.bdb;

import android.support.annotation.Nullable;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

public class TransactionIdentifier {
    // creator

    @JsonCreator
    public static TransactionIdentifier create(@JsonProperty("transaction_id") String transactionId,
                                               @JsonProperty("identifier") String identifier,
                                               @JsonProperty("hash")  @Nullable String hash,
                                               @JsonProperty("blockchain_id") String blockchainId) {
        return new TransactionIdentifier(
                checkNotNull(transactionId),
                checkNotNull(identifier),
                hash,
                checkNotNull(blockchainId)
        );
    }

    // fields

    private final String transactionId;
    private final String identifier;
    private final @Nullable String hash;
    private final String blockchainId;

    private TransactionIdentifier(String transactionId,
                                  String identifier,
                                  @Nullable String hash,
                                  String blockchainId) {
        this.transactionId = transactionId;
        this.identifier = identifier;
        this.hash = hash;
        this.blockchainId = blockchainId;
    }
    // getters

    @JsonProperty("transaction_id")
    public String getId() {
        return transactionId;
    }

    @JsonProperty("identifier")
    public String getIdentifier() {
        return identifier;
    }

    @JsonProperty("hash")
    public Optional<String> getHash() {
        return Optional.fromNullable(hash);
    }

    @JsonProperty("blockchain_id")
    public String getBlockchainId() {
        return blockchainId;
    }
}
