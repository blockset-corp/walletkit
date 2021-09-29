/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 11/14/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import com.blockset.walletkit.SystemClient.Amount;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetAmount implements Amount {

    // creators

    // TODO(fix): require the currency id
    public static BlocksetAmount create(String amountValue) {
        return new BlocksetAmount(
                null,
                amountValue
        );
    }

    @JsonCreator
    public static BlocksetAmount create(@JsonProperty("currency_id") String currency,
                                        @JsonProperty("amount") String amount) {
        return new BlocksetAmount(
                checkNotNull(currency),
                checkNotNull(amount)
        );
    }

    // fields

    private final String currency;
    private final String amount;

    private BlocksetAmount(String currencyId,
                           String amount) {
        this.currency = currencyId;
        this.amount= amount;
    }

    // getters

    @Override
    @JsonProperty("currency_id")
    public String getCurrency() {
        return currency;
    }

    @Override
    @JsonProperty("amount")
    public String getAmount() {
        return amount;
    }
}
