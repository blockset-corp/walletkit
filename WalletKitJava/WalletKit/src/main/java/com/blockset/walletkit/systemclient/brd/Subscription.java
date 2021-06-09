/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.systemclient.brd;

import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.SystemClient.SubscriptionEndpoint;
import com.blockset.walletkit.SystemClient.SubscriptionCurrency;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class Subscription implements SystemClient.Subscription {

    // creators

    @JsonCreator
    public static Subscription create(@JsonProperty("subscription_id") String id,
                                      @JsonProperty("device_id") String deviceId,
                                      @JsonProperty("endpoint") SubscriptionEndpoint endpoint,
                                      @JsonProperty("currencies") List<SubscriptionCurrency> currencies) {
        return new Subscription(
                checkNotNull(id),
                checkNotNull(deviceId),
                checkNotNull(endpoint),
                checkNotNull(currencies)
        );
    }

    // fields

    private final String id;
    private final String deviceId;
    private final SubscriptionEndpoint endpoint;
    private final List<SubscriptionCurrency> currencies;

    private Subscription(String subscriptionId,
                         String deviceId,
                         SubscriptionEndpoint endpoint,
                         List<SubscriptionCurrency> currencies) {
        this.id = subscriptionId;
        this.deviceId = deviceId;
        this.endpoint = endpoint;
        this.currencies = currencies;
    }

    // getters

    @JsonProperty("subscription_id")
    public String getId() {
        return id;
    }

    @JsonProperty("device_id")
    public String getDevice() {
        return deviceId;
    }

    @JsonProperty("endpoint")
    public SubscriptionEndpoint getEndpoint() {
        return endpoint;
    }

    @JsonProperty("currencies")
    public List<SubscriptionCurrency> getCurrencies() {
        return currencies;
    }
}
