/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import androidx.annotation.Nullable;

import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.SystemClient.Blockchain;
import com.blockset.walletkit.SystemClient.BlockchainFee;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedLong;

import java.util.ArrayList;
import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetBlockchain implements Blockchain {

    // creators

    @JsonCreator
    public static BlocksetBlockchain create(@JsonProperty("id") String id,
                                            @JsonProperty("name") String name,
                                            @JsonProperty("network") String network,
                                            @JsonProperty("is_mainnet") Boolean isMainNet,
                                            @JsonProperty("native_currency_id") String currencyId,
                                            @JsonProperty("verified_height") UnsignedLong blockHeight,
                                            @JsonProperty("fee_estimates") List<BlocksetBlockchainFee> feeEstimates,
                                            @JsonProperty("confirmations_until_final") UnsignedInteger confirmationsUntilFinal,
                                            @JsonProperty("verified_block_hash") @Nullable String verifiedBlockHash) {
        return new BlocksetBlockchain(
                checkNotNull(id),
                checkNotNull(name),
                checkNotNull(network),
                checkNotNull(isMainNet),
                checkNotNull(currencyId),
                checkNotNull(blockHeight),
                checkNotNull(feeEstimates),
                checkNotNull(confirmationsUntilFinal),
                verifiedBlockHash
        );
    }

    // fields

    private final String name;
    private final String id;
    private final String currencyId;
    private final List<BlocksetBlockchainFee> feeEstimates;
    private final Boolean isMainNet;
    private final String network;
    private final UnsignedLong blockHeight;
    private final UnsignedInteger confirmationsUntilFinal;
    private final @Nullable String verifiedBlockHash;

    private BlocksetBlockchain(String id,
                               String name,
                               String network,
                               boolean isMainNet,
                               String currencyId,
                               UnsignedLong blockHeight,
                               List<BlocksetBlockchainFee> feeEstimates,
                               UnsignedInteger confirmationsUntilFinal,
                               @Nullable String verifiedBlockHash) {
        this.id = id;
        this.name = name;
        this.network = network;
        this.isMainNet = isMainNet;
        this.currencyId = currencyId;
        this.blockHeight = blockHeight;
        this.feeEstimates = feeEstimates;
        this.confirmationsUntilFinal = confirmationsUntilFinal;
        this.verifiedBlockHash = verifiedBlockHash;
    }

    // getters

    @Override
    @JsonProperty("id")
    public String getId() {
        return id;
    }

    @Override
    @JsonProperty("name")
    public String getName() {
        return name;
    }

    @Override
    @JsonProperty("network")
    public String getNetwork() {
        return network;
    }

    @Override
    @JsonProperty("is_mainnet")
    public Boolean isMainnet() {
        return isMainNet;
    }

    @Override
    @JsonProperty("native_currency_id")
    public String getCurrency() {
        return currencyId;
    }

    @Override
    @JsonIgnore
    public Optional<UnsignedLong> getBlockHeight() {
        return SystemClient.Blockchain.BLOCK_HEIGHT_UNSPECIFIED.equals(blockHeight) ? Optional.absent() : Optional.of(blockHeight);
    }

    @Override
    @JsonProperty("verified_block_hash")
    public Optional<String> getVerifiedBlockHash() {
        return Optional.fromNullable(verifiedBlockHash);
    }

    @Override
    @JsonIgnore
    public List<BlockchainFee> getFeeEstimates() {
        return new ArrayList<BlockchainFee>(getBlocksetFeeEstimates());
    }

    @Override
    @JsonProperty("confirmations_until_final")
    public UnsignedInteger getConfirmationsUntilFinal() {
        return confirmationsUntilFinal;
    }

    @JsonProperty("fee_estimates")
    private List<BlocksetBlockchainFee> getBlocksetFeeEstimates() {
        return feeEstimates;
    }

    @JsonProperty("verified_height")
    public UnsignedLong getBlockHeightValue() {
        return blockHeight;
    }
}
