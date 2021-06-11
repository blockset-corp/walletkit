/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;


import android.support.annotation.Nullable;

import com.blockset.walletkit.SystemClient.Transfer;
import com.blockset.walletkit.SystemClient.Transaction;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;
import com.google.common.io.BaseEncoding;
import com.google.common.primitives.UnsignedLong;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Map;
import java.util.Comparator;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetTransaction implements Transaction {
    // creator

    @JsonCreator
    public static BlocksetTransaction create(@JsonProperty("transaction_id") String id,
                                             @JsonProperty("identifier") String identifier,
                                             @JsonProperty("hash") String hash,
                                             @JsonProperty("blockchain_id") String blockchainId,
                                             @JsonProperty("size") UnsignedLong size,
                                             @JsonProperty("fee") BlocksetAmount fee,
                                             @JsonProperty("status") String status,
                                             @JsonProperty("_embedded") @Nullable Embedded embedded,
                                             @JsonProperty("first_seen") @Nullable Date firstSeen,
                                             @JsonProperty("timestamp") @Nullable Date timestamp,
                                             @JsonProperty("index") @Nullable UnsignedLong index,
                                             @JsonProperty("block_hash") @Nullable String blockHash,
                                             @JsonProperty("block_height") @Nullable UnsignedLong blockHeight,
                                             @JsonProperty("acknowledgements") @Nullable UnsignedLong acknowledgements,
                                             @JsonProperty("confirmations") @Nullable UnsignedLong confirmations,
                                             @JsonProperty("raw") @Nullable String raw,
                                             @JsonProperty("proof") @Nullable String proof,
                                             @JsonProperty("meta") Map<String, String> metaData) {
        return new BlocksetTransaction(
                checkNotNull(id),
                checkNotNull(identifier),
                checkNotNull(hash),
                checkNotNull(blockchainId),
                checkNotNull(size),
                checkNotNull(fee),
                checkNotNull(status),
                embedded,
                firstSeen,
                timestamp,
                index,
                blockHash,
                blockHeight,
                acknowledgements,
                confirmations,
                raw,
                proof,
                metaData
        );
    }

    // fields

    private final String id;
    private final String identifier;
    private final String hash;
    private final String blockchainId;
    private final UnsignedLong size;
    private final BlocksetAmount fee;
    private final String status;
    private final @Nullable Embedded embedded;
    private final @Nullable Date firstSeen;
    private final @Nullable Date timestamp;
    private final @Nullable UnsignedLong index;
    private final @Nullable String blockHash;
    private final @Nullable UnsignedLong blockHeight;
    private final @Nullable UnsignedLong acknowledgements;
    private final @Nullable UnsignedLong confirmations;
    private final @Nullable String raw;
    private final @Nullable String proof;
    private final Map<String, String> metaData;

    private BlocksetTransaction(String id,
                                String identifier,
                                String hash,
                                String blockchainId,
                                UnsignedLong size,
                                BlocksetAmount fee,
                                String status,
                                @Nullable Embedded embedded,
                                @Nullable Date firstSeen,
                                @Nullable Date timestamp,
                                @Nullable UnsignedLong index,
                                @Nullable String blockHash,
                                @Nullable UnsignedLong blockHeight,
                                @Nullable UnsignedLong acknowledgements,
                                @Nullable UnsignedLong confirmations,
                                @Nullable String raw,
                                @Nullable String proof,
                                Map<String, String> metaData) {
        this.id = id;
        this.identifier = identifier;
        this.hash = hash;
        this.blockchainId = blockchainId;
        this.size = size;
        this.fee = fee;
        this.status = status;
        this.embedded = embedded;
        this.firstSeen = firstSeen;
        this.timestamp = timestamp;
        this.index = index;
        this.blockHash = blockHash;
        this.blockHeight = blockHeight;
        this.acknowledgements = acknowledgements;
        this.confirmations = confirmations;
        this.raw = raw;
        this.proof = proof;
        this.metaData = metaData;
    }
    // getters

    @Override
    @JsonProperty("transaction_id")
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
    @JsonProperty("identifier")
    public String getIdentifier() {
        return identifier;
    }

    @Override
    @JsonProperty("block_height")
    public Optional<UnsignedLong> getBlockHeight() {
        return Optional.fromNullable(blockHeight);
    }

    @Override
    @JsonProperty("index")
    public Optional<UnsignedLong> getIndex() {
        return Optional.fromNullable(index);
    }

    @Override
    @JsonProperty("confirmations")
    public Optional<UnsignedLong> getConfirmations() {
        return Optional.fromNullable(confirmations);
    }

    @Override
    @JsonProperty("status")
    public String getStatus() {
        return status;
    }

    @Override
    @JsonProperty("size")
    public UnsignedLong getSize() {
        return size;
    }

    @Override
    @JsonProperty("timestamp")
    public Optional<Date> getTimestamp() {
        return Optional.fromNullable(timestamp);
    }

    @Override
    @JsonProperty("first_seen")
    public Optional<Date> getFirstSeen() {
        return Optional.fromNullable(firstSeen);
    }

    @Override
    @JsonIgnore
    public Optional<byte[]> getRaw() {
        return getOptionalBase64Bytes(raw);
    }

    @Override
    @JsonProperty("fee")
    public BlocksetAmount getFee() {
        return fee;
    }

    @Override
    @JsonIgnore
    public List<Transfer> getTransfers() {
        return embedded == null ? Collections.emptyList() : new ArrayList<Transfer>(embedded.transfers);
    }

    @Override
    @JsonProperty("acknowledgements")
    public Optional<UnsignedLong> getAcknowledgements() {
        return Optional.fromNullable(acknowledgements);
    }

    @Override
    @JsonProperty("meta")
    public Map<String, String> getMetaData() { return metaData; }


    
    @JsonProperty("block_hash")
    public Optional<String> getBlockHash() {
        return Optional.fromNullable(blockHash);
    }

    @JsonProperty("proof")
    public Optional<String> getProof() {
        return Optional.fromNullable(proof);
    }

    @JsonProperty("_embedded")
    public Optional<Embedded> getEmbedded() {
        return Optional.fromNullable(embedded);
    }

    @JsonProperty("raw")
    public Optional<String> getRawValue() {
        return Optional.fromNullable(raw);
    }

    // internal details

    public static class Embedded {

        @JsonProperty
        public List<BlocksetTransfer> transfers;
    }

    private Optional<byte[]> getOptionalBase64Bytes(String value) {
        if (null == value) {
            return Optional.absent();
        }
        try {
            return Optional.fromNullable(BaseEncoding.base64().decode(value));
        } catch (IllegalArgumentException e) {
            return Optional.absent();
        }
    }

    // Comparators

    public static final Comparator<Transaction> blockHeightAndIndexComparator =
            (Transaction t1, Transaction t2) -> {
            UnsignedLong th1 = t1.getBlockHeight().or(UnsignedLong.MAX_VALUE);
            UnsignedLong th2 = t2.getBlockHeight().or(UnsignedLong.MAX_VALUE);

            UnsignedLong ti1 = t1.getIndex().or(UnsignedLong.MAX_VALUE);
            UnsignedLong ti2 = t2.getIndex().or(UnsignedLong.MAX_VALUE);

            int heightCompare = th1.compareTo(th2);
            int indexCompare  = ti1.compareTo(ti2);

            return heightCompare != 0 ? heightCompare : indexCompare;
        };
}
