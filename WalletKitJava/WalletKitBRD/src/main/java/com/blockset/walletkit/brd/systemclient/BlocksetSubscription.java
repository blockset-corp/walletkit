/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import com.blockset.walletkit.SystemClient.SubscriptionEndpoint;
import com.blockset.walletkit.SystemClient.Subscription;
import com.blockset.walletkit.SystemClient.SubscriptionCurrency;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.ArrayList;
import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetSubscription implements Subscription {

    // creators

    @JsonCreator
    public static BlocksetSubscription create(@JsonProperty("subscription_id") String id,
                                              @JsonProperty("device_id") String deviceId,
                                              @JsonProperty("endpoint") BlocksetSubscriptionEndpoint endpoint,
                                              @JsonProperty("currencies") List<BlocksetSubscriptionCurrency> currencies) {
        return new BlocksetSubscription(
                checkNotNull(id),
                checkNotNull(deviceId),
                checkNotNull(endpoint),
                checkNotNull(currencies)
        );
    }

    // fields

    private final String id;
    private final String deviceId;
    private final BlocksetSubscriptionEndpoint endpoint;
    private final List<BlocksetSubscriptionCurrency> currencies;

    private BlocksetSubscription(String subscriptionId,
                                 String deviceId,
                                 BlocksetSubscriptionEndpoint endpoint,
                                 List<BlocksetSubscriptionCurrency> currencies) {
        this.id = subscriptionId;
        this.deviceId = deviceId;
        this.endpoint = endpoint;
        this.currencies = currencies;
    }

    // getters

    @Override
    @JsonProperty("subscription_id")
    public String getId() {
        return id;
    }

    @Override
    @JsonProperty("device_id")
    public String getDevice() {
        return deviceId;
    }

    @Override
    @JsonProperty("endpoint")
    public SubscriptionEndpoint getEndpoint() {
        return endpoint;
    }

    @Override
    @JsonIgnore
    public List<SubscriptionCurrency> getCurrencies() {
        return new ArrayList<SubscriptionCurrency>(getBlocksetCurrencies());
    }

    @JsonProperty("currencies")
    private List<BlocksetSubscriptionCurrency> getBlocksetCurrencies() {
        return currencies;
    }

}
