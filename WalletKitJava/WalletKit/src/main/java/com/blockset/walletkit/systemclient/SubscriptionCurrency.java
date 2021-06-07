/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 11/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.systemclient;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class SubscriptionCurrency {

    // creators

    @JsonCreator
    public static SubscriptionCurrency create(@JsonProperty("currency_id") String currencyId,
                                              @JsonProperty("addresses") List<String> addresses,
                                              @JsonProperty("events") List<com.blockset.walletkit.systemclient.SubscriptionEvent> events) {
        return new SubscriptionCurrency(
                checkNotNull(currencyId),
                checkNotNull(addresses),
                checkNotNull(events)
        );
    }

    // fields

    private final String currencyId;
    private final List<String> addresses;
    private final List<com.blockset.walletkit.systemclient.SubscriptionEvent> events;

    private SubscriptionCurrency(String currencyId,
                                 List<String> addresses,
                                 List<com.blockset.walletkit.systemclient.SubscriptionEvent> events) {
        this.currencyId = currencyId;
        this.addresses = addresses;
        this.events = events;
    }

    // getters

    @JsonProperty("currency_id")
    public String getCurrencyId() {
        return currencyId;
    }

    @JsonProperty("addresses")
    public List<String> getAddresses() {
        return addresses;
    }

    @JsonProperty("events")
    public List<com.blockset.walletkit.systemclient.SubscriptionEvent> getEvents() {
        return events;
    }
}
