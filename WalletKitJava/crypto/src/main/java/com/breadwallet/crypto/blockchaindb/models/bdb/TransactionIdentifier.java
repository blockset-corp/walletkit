package com.breadwallet.crypto.blockchaindb.models.bdb;

import android.support.annotation.Nullable;

import com.breadwallet.crypto.blockchaindb.models.Utilities;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;

import static com.google.common.base.Preconditions.checkNotNull;

public class TransactionIdentifier {
    // creator

    @JsonCreator
    public static com.breadwallet.crypto.blockchaindb.models.bdb.TransactionIdentifier create(@JsonProperty("transaction_id") String transactionId,
                                                                                              @JsonProperty("identifier") String identifier,
                                                                                              @JsonProperty("hash") String hash,
                                                                                              @JsonProperty("blockchain_id") String blockchainId) {
        return new com.breadwallet.crypto.blockchaindb.models.bdb.TransactionIdentifier(
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
