/*
 * Created by Michael Carrara.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.systemclient.brd;

import com.blockset.walletkit.SystemClient;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import java.util.Comparator;

import static com.google.common.base.Preconditions.checkNotNull;

public class HederaAccount implements SystemClient.HederaAccount {
    @JsonCreator
    public static HederaAccount create(@JsonProperty("account_id") String id,
                                       @JsonProperty("hbar_balance") Long balance,
                                       @JsonProperty("account_status") String status) {
        return new HederaAccount(
                checkNotNull(id),
                balance,
                checkNotNull(status)
        );
    }

    private final String id;
    private final Optional<Long> balance;
    private final String status;

    private HederaAccount (String id, Long balance, String status) {
        this.id = id;
        this.balance = Optional.fromNullable(balance);
        this.status = status;
        }

    @JsonProperty("account_id")
    public String getId () {
        return id;
    }

    @JsonProperty("hbar_balance")
    public Optional<Long> getBalance () {
        return balance;
    }

    @JsonProperty("account_status")
    public String getStatus() {
        return status;
    }

    public Boolean isDeleted() {
        return !"active".equalsIgnoreCase(status);
    }
}
