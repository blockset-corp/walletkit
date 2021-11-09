package com.blockset.walletkit.demo.walletconnect.msg;

import androidx.annotation.Nullable;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

public class WCSessionUpdateReq {

    @JsonCreator
    public static WCSessionUpdateReq create(@JsonProperty("approved")   boolean      approved,
                                            @JsonProperty("accounts")   String[]    accounts,
                                            @JsonProperty("chainId")    Number      chainId,
                                            @JsonProperty("networkId")  Number       networkId) {
        return new WCSessionUpdateReq(approved,
                                      accounts,
                                      chainId,
                                      networkId);
    }

    private final           boolean   approved;
    private final @Nullable String[]  accounts; // Neither does the spec say the accounts are '?' nullable
    private final @Nullable Number    chainId;
    private final @Nullable Number    networkId;


    private WCSessionUpdateReq(boolean              approved,
                               @Nullable String[]   accounts,
                               @Nullable Number     chainId,
                               @Nullable Number     networkId ) {
        this.approved = approved;
        this.accounts = accounts;
        this.chainId = chainId;
        this.networkId = networkId;
    }

    // getters

    @JsonProperty("approved")
    public boolean getApproved() { return approved; }

    @JsonProperty("accounts")
    public Optional<String[]> getAccounts() { return Optional.fromNullable(accounts); }

    @JsonProperty("chainId")
    public Optional<Number> getChainId() { return Optional.fromNullable(chainId); }

    @JsonProperty("networkId")
    public Optional<Number> getNetworkId() { return Optional.fromNullable(networkId); }

}
