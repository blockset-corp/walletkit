package com.blockset.walletkit.demo.walletconnect.msg;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import static com.google.common.base.Preconditions.checkNotNull;

public class WCSessionRequestResp {

    @JsonCreator
    public static WCSessionRequestResp create(@JsonProperty("peerId")       String      peerId,
                                              @JsonProperty("peerMeta")     PeerMeta    peerMeta,
                                              @JsonProperty("approved")     boolean     approved,
                                              @JsonProperty("chainId")      Number      chainId,
                                              @JsonProperty("accounts")     String[]    accounts    ) {
        return new WCSessionRequestResp(checkNotNull(peerId),
                checkNotNull(peerMeta),
                approved,
                checkNotNull(chainId),
                checkNotNull(accounts));
    }

    private final String    peerId;
    private final PeerMeta  peerMeta;
    private final boolean   approved;
    private final Number    chainId;
    private final String[]  accounts;

    private WCSessionRequestResp(String              peerId,
                                 PeerMeta            peerMeta,
                                 boolean             approved,
                                 Number              chainId,
                                 String[]            accounts   ) {
        this.peerId = peerId;
        this.peerMeta = peerMeta;
        this.approved = approved;
        this.chainId = chainId;
        this.accounts = accounts;
    }

    // getters

    @JsonProperty("peerId")
    public String getPeerId() { return peerId; }

    @JsonProperty("peerMeta")
    public PeerMeta getPeerMeta() { return peerMeta; }

    @JsonProperty("approved")
    public boolean getApproved() { return approved; }

    @JsonProperty("chainId")
    public Number getChainId() { return chainId; }

    @JsonProperty("accounts")
    public String[] getAccounts() { return accounts; }

}
