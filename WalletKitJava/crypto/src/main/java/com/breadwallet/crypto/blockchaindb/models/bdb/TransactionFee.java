package com.breadwallet.crypto.blockchaindb.models.bdb;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.primitives.UnsignedLong;

import static com.google.common.base.Preconditions.checkNotNull;

public class TransactionFee {
    // creator

    @JsonCreator
    public static TransactionFee create(@JsonProperty("cost_units") UnsignedLong costUnits) {
        return new TransactionFee(
                checkNotNull(costUnits)
        );
    }

    // fields

    private final UnsignedLong costUnits;

    private TransactionFee(UnsignedLong costUnits) {
        this.costUnits = costUnits;
    }

    // getters

    @JsonProperty("cost_units")
    public UnsignedLong getCostUnits() {
        return costUnits;
    }
}
