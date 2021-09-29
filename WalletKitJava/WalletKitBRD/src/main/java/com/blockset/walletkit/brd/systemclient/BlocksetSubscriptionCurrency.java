/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 11/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import com.blockset.walletkit.SystemClient.SubscriptionCurrency;
import com.blockset.walletkit.SystemClient.SubscriptionEvent;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.ArrayList;
import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetSubscriptionCurrency implements SubscriptionCurrency {

    // creators

    @JsonCreator
    public static BlocksetSubscriptionCurrency create(@JsonProperty("currency_id") String currencyId,
                                                      @JsonProperty("addresses") List<String> addresses,
                                                      @JsonProperty("events") List<BlocksetSubscriptionEvent> events) {
        return new BlocksetSubscriptionCurrency(
                checkNotNull(currencyId),
                checkNotNull(addresses),
                checkNotNull(events)
        );
    }

    // fields

    private final String currencyId;
    private final List<String> addresses;
    private final List<BlocksetSubscriptionEvent> events;

    private BlocksetSubscriptionCurrency(String currencyId,
                                         List<String> addresses,
                                         List<BlocksetSubscriptionEvent> events) {
        this.currencyId = currencyId;
        this.addresses = addresses;
        this.events = events;
    }

    // getters

    @Override
    @JsonProperty("currency_id")
    public String getCurrencyId() {
        return currencyId;
    }

    @Override
    @JsonProperty("addresses")
    public List<String> getAddresses() {
        return addresses;
    }

    @Override
    @JsonIgnore
    public List<SubscriptionEvent> getEvents() {
        return new ArrayList<SubscriptionEvent>(getBlocksetEvents());
    }

    @JsonProperty("events")
    private List<BlocksetSubscriptionEvent> getBlocksetEvents() {
        return events;
    }

}
