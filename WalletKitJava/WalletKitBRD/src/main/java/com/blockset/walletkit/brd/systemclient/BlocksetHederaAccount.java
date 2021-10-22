/*
 * Created by Michael Carrara.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import com.blockset.walletkit.SystemClient.HederaAccount;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import java.util.Comparator;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetHederaAccount implements HederaAccount {

    @JsonCreator
    public static BlocksetHederaAccount create(@JsonProperty("account_id") String id,
                                               @JsonProperty("hbar_balance") Long balance,
                                               @JsonProperty("account_status") String status) {
        return new BlocksetHederaAccount(
                checkNotNull(id),
                balance,
                checkNotNull(status)
        );
    }

    private final String id;
    private final Optional<Long> balance;
    private final String status;

    private BlocksetHederaAccount (String id, Long balance, String status) {
        this.id = id;
        this.balance = Optional.fromNullable(balance);
        this.status = status;
    }

    @Override
    @JsonProperty("account_id")
    public String getId () {
        return id;
    }

    @Override
    @JsonProperty("hbar_balance")
    public Optional<Long> getBalance () {
        return balance;
    }

    @Override
    public Boolean isDeleted() {
        return !"active".equalsIgnoreCase(status);
    }

    @JsonProperty("account_status")
    public String getStatus() {
        return status;
    }
}
