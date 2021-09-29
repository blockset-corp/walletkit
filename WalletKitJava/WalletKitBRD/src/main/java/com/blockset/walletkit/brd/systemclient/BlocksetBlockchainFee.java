/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import com.blockset.walletkit.SystemClient.BlockchainFee;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.primitives.UnsignedLong;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetBlockchainFee implements BlockchainFee {

    // creators

    public static BlocksetBlockchainFee create(String amount,
                                               String tier,
                                               UnsignedLong confirmationTimeInMilliseconds) {
        return create(
                BlocksetAmount.create(amount),
                tier,
                confirmationTimeInMilliseconds
        );
    }

    @JsonCreator
    public static BlocksetBlockchainFee create(@JsonProperty("fee") BlocksetAmount fee,
                                               @JsonProperty("tier") String tier,
                                               @JsonProperty("estimated_confirmation_in") UnsignedLong confirmationTimeInMilliseconds) {
        return new BlocksetBlockchainFee(
                checkNotNull(fee),
                checkNotNull(tier),
                checkNotNull(confirmationTimeInMilliseconds)
        );
    }

    // fields

    private final BlocksetAmount amount;
    private final String tier;
    private final UnsignedLong confirmationTimeInMilliseconds;

    private BlocksetBlockchainFee(BlocksetAmount amount,
                                  String tier,
                                  UnsignedLong confirmationTimeInMilliseconds) {
        this.amount = amount;
        this.tier = tier;
        this.confirmationTimeInMilliseconds = confirmationTimeInMilliseconds;
    }

    // getters

    @Override
    @JsonIgnore
    public String getAmount() {
        return amount.getAmount();
    }

    @Override
    @JsonProperty("tier")
    public String getTier() {
        return tier;
    }

    @Override
    @JsonProperty("estimated_confirmation_in")
    public UnsignedLong getConfirmationTimeInMilliseconds() {
        return confirmationTimeInMilliseconds;
    }

    @JsonProperty("fee")
    public BlocksetAmount getFee() {
        return amount;
    }
}
