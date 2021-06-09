/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
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
import com.google.common.primitives.UnsignedLong;

import java.util.Map;

import static com.google.common.base.Preconditions.checkNotNull;

public class Transfer implements SystemClient.Transfer {
    // creators

    @JsonCreator
    public static Transfer create(@JsonProperty("transfer_id") String id,
                                  @JsonProperty("blockchain_id") String blockchainId,
                                  @JsonProperty("index") UnsignedLong index,
                                  @JsonProperty("amount") Amount amount,
                                  @JsonProperty("meta") Map<String, String> metaData,
                                  @JsonProperty("from_address") @Nullable String source,
                                  @JsonProperty("to_address") @Nullable String target,
                                  @JsonProperty("transaction_id") @Nullable String transactionId,
                                  @JsonProperty("acknowledgements") @Nullable UnsignedLong acknowledgements) {
        return new Transfer(
                checkNotNull(id),
                checkNotNull(blockchainId),
                checkNotNull(index),
                checkNotNull(amount),
                checkNotNull(metaData),
                source,
                target,
                transactionId,
                acknowledgements
        );
    }

    // fields

    private final String id;
    private final String blockchainId;
    private final UnsignedLong index;
    private final Amount amount;
    private final Map<String, String> metaData;
    private final @Nullable String source;
    private final @Nullable String target;
    private final @Nullable String transactionId;
    private final @Nullable UnsignedLong acknowledgements;

    private Transfer(String id,
                     String blockchainId,
                     UnsignedLong index,
                     Amount amount,
                     Map<String, String> metaData,
                     @Nullable String source,
                     @Nullable String target,
                     @Nullable String transactionId,
                     @Nullable UnsignedLong acknowledgements) {
        this.id = id;
        this.blockchainId = blockchainId;
        this.index = index;
        this.amount = amount;
        this.metaData = metaData;
        this.source = source;
        this.target = target;
        this.transactionId = transactionId;
        this.acknowledgements = acknowledgements;
    }

    // getters

    @JsonProperty("transfer_id")
    public String getId() {
        return id;
    }

    @JsonProperty("blockchain_id")
    public String getBlockchainId() {
        return blockchainId;
    }

    @JsonProperty("index")
    public UnsignedLong getIndex() {
        return index;
    }

    @JsonProperty("amount")
    public Amount getAmount() {
        return amount;
    }

    @JsonProperty("meta")
    public Map<String, String> getMetaData() {
        return metaData;
    }

    @JsonProperty("from_address")
    public Optional<String> getSource() {
        return Optional.fromNullable(source);
    }

    @JsonProperty("to_address")
    public Optional<String> getTarget() {
        return Optional.fromNullable(target);
    }

    @JsonProperty("transaction_id")
    public Optional<String> getTransactionId() {
        return Optional.fromNullable(transactionId);
    }

    @JsonProperty("acknowledgements")
    public Optional<UnsignedLong> getAcknowledgements() {
        return Optional.fromNullable(acknowledgements);
    }
}
