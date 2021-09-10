/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import androidx.annotation.Nullable;

import com.blockset.walletkit.SystemClient.Block;
import com.blockset.walletkit.SystemClient.Transaction;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetBlock implements Block {

    // creator

    @JsonCreator
    public static BlocksetBlock create(@JsonProperty("block_id") String id,
                                       @JsonProperty("hash") String hash,
                                       @JsonProperty("blockchain_id") String blockchainId,
                                       @JsonProperty("height") UnsignedLong height,
                                       @JsonProperty("mined") Date mined,
                                       @JsonProperty("transaction_ids") List<String> transactionIds,
                                       @JsonProperty("size") UnsignedLong size,
                                       @JsonProperty("total_fees") BlocksetAmount totalFees,
                                       @JsonProperty("acknowledgements") UnsignedLong acknowledgements,
                                       @JsonProperty("is_active_chain") Boolean isActiveChain,
                                       @JsonProperty("_embedded") @Nullable Embedded embedded,
                                       @JsonProperty("prev_hash") @Nullable String prevHash,
                                       @JsonProperty("next_hash") @Nullable String nextHash,
                                       @JsonProperty("header") @Nullable String header,
                                       @JsonProperty("raw") @Nullable String raw) {
        return new BlocksetBlock(
                checkNotNull(id),
                checkNotNull(hash),
                checkNotNull(blockchainId),
                checkNotNull(height),
                checkNotNull(mined),
                checkNotNull(transactionIds),
                checkNotNull(size),
                checkNotNull(totalFees),
                checkNotNull(acknowledgements),
                checkNotNull(isActiveChain),
                embedded,
                prevHash,
                nextHash,
                header,
                raw
        );
    }


    // fields

    private final String id;
    private final String hash;
    private final String blockchainId;
    private final UnsignedLong height;
    private final Date mined;
    private final List<String> transactionIds;
    private final UnsignedLong size;
    private final BlocksetAmount totalFees;
    private final UnsignedLong acknowledgements;
    private final boolean isActiveChain;
    private final @Nullable Embedded embedded;
    private final @Nullable String prevHash;
    private final @Nullable String nextHash;
    private final @Nullable String header;
    private final @Nullable String raw;

    private BlocksetBlock(String id,
                          String hash,
                          String blockchainId,
                          UnsignedLong height,
                          Date mined,
                          List<String> transactionIds,
                          UnsignedLong size,
                          BlocksetAmount totalFees,
                          UnsignedLong acknowledgements,
                          boolean isActiveChain,
                          @Nullable Embedded embedded,
                          @Nullable String prevHash,
                          @Nullable String nextHash,
                          @Nullable String header,
                          @Nullable String raw) {
        this.id = id;
        this.hash = hash;
        this.blockchainId = blockchainId;
        this.embedded = embedded;
        this.prevHash = prevHash;
        this.nextHash = nextHash;
        this.height = height;
        this.mined = mined;
        this.transactionIds = transactionIds;
        this.size = size;
        this.totalFees = totalFees;
        this.header = header;
        this.raw = raw;
        this.acknowledgements = acknowledgements;
        this.isActiveChain = isActiveChain;
    }

    // getters

    @Override
    @JsonProperty("block_id")
    public String getId() {
        return id;
    }

    @Override
    @JsonProperty("blockchain_id")
    public String getBlockchainId() {
        return blockchainId;
    }

    @Override
    @JsonProperty("hash")
    public String getHash() {
        return hash;
    }

    @Override
    @JsonProperty("height")
    public UnsignedLong getHeight() {
        return height;
    }

    @Override
    @JsonProperty("header")
    public Optional<String> getHeader() {
        return Optional.fromNullable(header);
    }

    @Override
    @JsonProperty("raw")
    public Optional<String> getRaw() {
        return Optional.fromNullable(raw);
    }

    @Override
    @JsonProperty("mined")
    public Date getMined() {
        return mined;
    }

    @Override
    @JsonProperty("size")
    public UnsignedLong getSize() {
        return size;
    }

    @Override
    @JsonProperty("prev_hash")
    public Optional<String> getPrevHash() {
        return Optional.fromNullable(prevHash);
    }

    @Override
    @JsonProperty("next_hash")
    public Optional<String> getNextHash() {
        return Optional.fromNullable(nextHash);
    }

    @Override
    @JsonIgnore
    public List<Transaction> getTransactions() {
        return embedded == null ? Collections.emptyList() : new ArrayList<Transaction>(embedded.transactions);
    }

    @Override
    @JsonProperty("acknowledgements")
    public UnsignedLong getAcknowledgements() {
        return acknowledgements;
    }



    @JsonProperty("transaction_ids")
    public List<String> getTransactionIds() {
        return transactionIds;
    }

    @JsonProperty("total_fees")
    public BlocksetAmount getTotalFees() {
        return totalFees;
    }

    @JsonProperty("is_active_chain")
    public boolean isActiveChain() {
        return isActiveChain;
    }

    @JsonProperty("_embedded")
    private Optional<Embedded> getEmbedded() {
        return Optional.fromNullable(embedded);
    }

    // internal details

    public static class Embedded {

        @JsonProperty
        public List<BlocksetTransaction> transactions;
    }
}
