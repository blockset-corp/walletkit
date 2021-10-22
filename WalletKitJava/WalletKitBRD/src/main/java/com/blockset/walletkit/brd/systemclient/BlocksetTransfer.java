/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;
import androidx.annotation.Nullable;

import com.blockset.walletkit.SystemClient.Transfer;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;

import java.util.Map;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetTransfer implements Transfer {
    // creators

    @JsonCreator
    public static BlocksetTransfer create(@JsonProperty("transfer_id") String id,
                                          @JsonProperty("blockchain_id") String blockchainId,
                                          @JsonProperty("index") UnsignedLong index,
                                          @JsonProperty("amount") BlocksetAmount amount,
                                          @JsonProperty("meta") Map<String, String> metaData,
                                          @JsonProperty("from_address") @Nullable String source,
                                          @JsonProperty("to_address") @Nullable String target,
                                          @JsonProperty("transaction_id") @Nullable String transactionId,
                                          @JsonProperty("acknowledgements") @Nullable UnsignedLong acknowledgements) {
        return new BlocksetTransfer(
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
    private final BlocksetAmount amount;
    private final Map<String, String> metaData;
    private final @Nullable String source;
    private final @Nullable String target;
    private final @Nullable String transactionId;
    private final @Nullable UnsignedLong acknowledgements;

    private BlocksetTransfer(String id,
                             String blockchainId,
                             UnsignedLong index,
                             BlocksetAmount amount,
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

    @Override
    @JsonProperty("transfer_id")
    public String getId() {
        return id;
    }

    @Override
    @JsonProperty("from_address")
    public Optional<String> getSource() {
        return Optional.fromNullable(source);
    }

    @Override
    @JsonProperty("to_address")
    public Optional<String> getTarget() {
        return Optional.fromNullable(target);
    }

    @Override
    @JsonProperty("amount")
    public BlocksetAmount getAmount() {
        return amount;
    }

    @Override
    @JsonProperty("acknowledgements")
    public Optional<UnsignedLong> getAcknowledgements() {
        return Optional.fromNullable(acknowledgements);
    }

    @Override
    @JsonProperty("index")
    public UnsignedLong getIndex() {
        return index;
    }

    @Override
    @JsonProperty("transaction_id")
    public Optional<String> getTransactionId() {
        return Optional.fromNullable(transactionId);
    }

    @Override
    @JsonProperty("blockchain_id")
    public String getBlockchainId() {
        return blockchainId;
    }

    @Override
    @JsonProperty("meta")
    public Map<String, String> getMetaData() {
        return metaData;
    }

}
