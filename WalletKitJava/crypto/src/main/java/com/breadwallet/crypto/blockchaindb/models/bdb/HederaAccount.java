package com.breadwallet.crypto.blockchaindb.models.bdb;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.Comparator;
import java.util.Date;

import static com.google.common.base.Preconditions.checkNotNull;

public class HederaAccount {
    @JsonCreator
    public static HederaAccount create(@JsonProperty("account_id") String accountId,
                                       @JsonProperty("hbar_balance") Long balance,
                                       @JsonProperty("account_status") String status,
                                       @JsonProperty("updated") Date timestamp) {
        return new HederaAccount(
                checkNotNull(accountId),
                checkNotNull(balance),
                checkNotNull(status),
                checkNotNull(timestamp)
        );
    }

    private final String accountId;
    private final Long balance;
    private final String status;
    private final Date timestamp;

    private HederaAccount (String accountId, Long balance, String status, Date timestamp) {
        this.accountId = accountId;
        this.balance = balance;
        this.status = status;
        this.timestamp = timestamp;
    }

    @JsonProperty("account_id")
    public String getAccountId () {
        return accountId;
    }

    @JsonProperty("hbar_balance")
    public Long getBalance () {
        return balance;
    }

    @JsonProperty("account_status")
    public String getStatus() {
        return status;
    }

    @JsonProperty("updated")
    public Date getTimestamp() {
        return timestamp;
    }

    public boolean isDeleted() {
        return !"active".equalsIgnoreCase(status);
    }

    public static final Comparator<HederaAccount> BALANCE_COMPARATOR =
            (HederaAccount a1, HederaAccount a2) -> Long.compare(a1.getBalance(), a2.getBalance());
}
