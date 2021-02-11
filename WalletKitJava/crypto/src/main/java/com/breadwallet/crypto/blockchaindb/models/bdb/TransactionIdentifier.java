package com.breadwallet.crypto.blockchaindb.models.bdb;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

public class TransactionIdentifier {
    // creator

    @JsonCreator
    public static TransactionIdentifier create(@JsonProperty("transaction_id") String transactionId,
                                               @JsonProperty("identifier") String identifier,
                                               @JsonProperty("hash") String hash,
                                               @JsonProperty("blockchain_id") String blockchainId) {
        return new TransactionIdentifier(
                checkNotNull(transactionId),
                checkNotNull(identifier),
                checkNotNull(hash),
                checkNotNull(blockchainId)
        );
    }

    // fields

    private final String transactionId;
    private final String identifier;
    private final String hash;
    private final String blockchainId;

    private TransactionIdentifier(String transactionId,
                                  String identifier,
                                  String hash,
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
    public String getHash() {
        return hash;
    }

    @JsonProperty("blockchain_id")
    public String getBlockchainId() {
        return blockchainId;
    }
}
