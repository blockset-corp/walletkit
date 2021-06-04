/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.blockchaindb.apis.bdb;

import com.blockset.walletkit.blockchaindb.errors.QueryError;
import com.blockset.walletkit.blockchaindb.models.bdb.NewSubscription;
import com.blockset.walletkit.blockchaindb.models.bdb.Subscription;
import com.blockset.walletkit.blockchaindb.models.bdb.SubscriptionCurrency;
import com.blockset.walletkit.blockchaindb.models.bdb.SubscriptionEndpoint;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.collect.ImmutableMultimap;

import java.util.List;

public class SubscriptionApi {

    private final BdbApiClient jsonClient;

    public SubscriptionApi(BdbApiClient jsonClient) {
        this.jsonClient = jsonClient;
    }

    public void getOrCreateSubscription(Subscription subscription,
                                        CompletionHandler<Subscription, QueryError> handler) {
        getSubscription(subscription.getId(), new CompletionHandler<Subscription, QueryError>() {
            @Override
            public void handleData(Subscription data) {
                handler.handleData(data);
            }

            @Override
            public void handleError(QueryError error) {
                createSubscription(subscription.getDevice(), subscription.getEndpoint(), subscription.getCurrencies(), handler);
            }
        });
    }

    public void createSubscription(String deviceId,
                                   SubscriptionEndpoint endpoint,
                                   List<SubscriptionCurrency> currencies,
                                   CompletionHandler<Subscription, QueryError> handler) {
        jsonClient.sendPost("subscriptions", ImmutableMultimap.of(), NewSubscription.create(deviceId, endpoint, currencies),
                Subscription.class, handler);
    }

    public void getSubscription(String subscriptionId,
                                CompletionHandler<Subscription, QueryError> handler) {
        jsonClient.sendGetWithId("subscriptions", subscriptionId, ImmutableMultimap.of(), Subscription.class, handler);
    }

    public void getSubscriptions(CompletionHandler<List<Subscription>, QueryError> handler) {
        jsonClient.sendGetForArray("subscriptions", ImmutableMultimap.of(), Subscription.class, handler);
    }

    public void updateSubscription(Subscription subscription,
                                   CompletionHandler<Subscription, QueryError> handler) {
        jsonClient.sendPutWithId("subscriptions", subscription.getId(), ImmutableMultimap.of(),
                subscription, Subscription.class, handler);
    }

    public void deleteSubscription(String id,
                                   CompletionHandler<Void, QueryError> handler) {
        jsonClient.sendDeleteWithId("subscriptions", id, ImmutableMultimap.of(), handler);
    }
}
