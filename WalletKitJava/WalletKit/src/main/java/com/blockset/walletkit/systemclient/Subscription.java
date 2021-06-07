/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
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

public class Subscription {

    // creators

    @JsonCreator
    public static Subscription create(@JsonProperty("subscription_id") String subscriptionId,
                                      @JsonProperty("device_id") String deviceId,
                                      @JsonProperty("endpoint") com.blockset.walletkit.systemclient.SubscriptionEndpoint endpoint,
                                      @JsonProperty("currencies") List<com.blockset.walletkit.systemclient.SubscriptionCurrency> currencies) {
        return new Subscription(
                checkNotNull(subscriptionId),
                checkNotNull(deviceId),
                checkNotNull(endpoint),
                checkNotNull(currencies)
        );
    }

    // fields

    private final String subscriptionId;
    private final String deviceId;
    private final com.blockset.walletkit.systemclient.SubscriptionEndpoint endpoint;
    private final List<com.blockset.walletkit.systemclient.SubscriptionCurrency> currencies;

    private Subscription(String subscriptionId,
                         String deviceId,
                         com.blockset.walletkit.systemclient.SubscriptionEndpoint endpoint,
                         List<com.blockset.walletkit.systemclient.SubscriptionCurrency> currencies) {
        this.subscriptionId = subscriptionId;
        this.deviceId = deviceId;
        this.endpoint = endpoint;
        this.currencies = currencies;
    }

    // getters

    @JsonProperty("subscription_id")
    public String getId() {
        return subscriptionId;
    }

    @JsonProperty("device_id")
    public String getDevice() {
        return deviceId;
    }

    @JsonProperty("endpoint")
    public com.blockset.walletkit.systemclient.SubscriptionEndpoint getEndpoint() {
        return endpoint;
    }

    @JsonProperty("currencies")
    public List<com.blockset.walletkit.systemclient.SubscriptionCurrency> getCurrencies() {
        return currencies;
    }
}
