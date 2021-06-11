/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 11/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import com.blockset.walletkit.SystemClient.SubscriptionEvent;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.primitives.UnsignedInteger;
import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetSubscriptionEvent implements SubscriptionEvent {

    // creators

    @JsonCreator
    public static BlocksetSubscriptionEvent create(@JsonProperty("name") String name,
                                                   @JsonProperty("confirmations") List<UnsignedInteger> confirmations) {
        return new BlocksetSubscriptionEvent(
                checkNotNull(name),
                checkNotNull(confirmations)
        );
    }

    // fields

    private final String name;
    private final List<UnsignedInteger> confirmations;

    private BlocksetSubscriptionEvent(String name,
                                      List<UnsignedInteger> confirmations) {
        this.name = name;
        this.confirmations = confirmations;
    }

    // getters

    @Override
    @JsonProperty("name")
    public String getName() {
        return name;
    }

    @Override
    @JsonProperty("confirmations")
    public List<UnsignedInteger> getConfirmations() {
        return confirmations;
    }
}
