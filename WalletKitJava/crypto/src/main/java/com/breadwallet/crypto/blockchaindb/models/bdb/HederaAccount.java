package com.breadwallet.crypto.blockchaindb.models.bdb;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import java.util.Comparator;
import java.util.Date;

import static com.google.common.base.Preconditions.checkNotNull;

public class HederaAccount {
    @JsonCreator
    public static HederaAccount create(@JsonProperty("account_id") String accountId,
                                       @JsonProperty("hbar_balance") Long balance,
                                       @JsonProperty("account_status") String status) {
        return new HederaAccount(
                checkNotNull(accountId),
                balance,
                checkNotNull(status)
        );
    }

    private final String accountId;
    private final Optional<Long> balance;
    private final String status;

    private HederaAccount (String accountId, Long balance, String status) {
        this.accountId = accountId;
        this.balance = Optional.fromNullable(balance);
        this.status = status;
        }

    @JsonProperty("account_id")
    public String getAccountId () {
        return accountId;
    }

    @JsonProperty("hbar_balance")
    public Optional<Long> getBalance () {
        return balance;
    }

    @JsonProperty("account_status")
    public String getStatus() {
        return status;
    }

    public boolean isDeleted() {
        return !"active".equalsIgnoreCase(status);
    }

    public static final Comparator<HederaAccount> BALANCE_COMPARATOR =
            (HederaAccount a1, HederaAccount a2) -> Long.compare(a1.getBalance().or(0L), a2.getBalance().or(0L));
}
