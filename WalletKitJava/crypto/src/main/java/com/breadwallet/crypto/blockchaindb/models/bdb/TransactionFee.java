/*
 * Created by Ehsan Rezaie on 10/30/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.crypto.blockchaindb.models.bdb;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.primitives.UnsignedLong;

import java.util.Map;

import static com.google.common.base.Preconditions.checkNotNull;

public class TransactionFee {

    // creator

    @JsonCreator
    public static TransactionFee create(@JsonProperty("cost_units") UnsignedLong costUnits,
                                        @JsonProperty("properties") Map<String, String> properties) {
        return new TransactionFee(
                checkNotNull(costUnits),
                checkNotNull(properties)
        );
    }

    // fields

    private final UnsignedLong costUnits;
    private final Map<String, String> properties;

    private TransactionFee(UnsignedLong costUnits,
                           Map<String, String> properties) {
        this.costUnits = costUnits;
        this.properties = properties;
    }

    // getters

    @JsonProperty("cost_units")
    public UnsignedLong getCostUnits() {
        return costUnits;
    }

    @JsonProperty("properties")
    public Map<String, String> getProperties() {
        return properties;
    }
}
