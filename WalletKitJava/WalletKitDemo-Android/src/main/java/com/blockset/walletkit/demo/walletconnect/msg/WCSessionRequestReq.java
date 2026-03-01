package com.blockset.walletkit.demo.walletconnect.msg;

import androidx.annotation.Nullable;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

public class WCSessionRequestReq {

    @JsonCreator
    public static WCSessionRequestReq create(@JsonProperty("peerId")    String      peerId,
                                             @JsonProperty("peerMeta")  PeerMeta    peerMeta,
                                             @JsonProperty("chainId")   Number      chainId) {
        return new WCSessionRequestReq(checkNotNull(peerId),
                                       checkNotNull(peerMeta),
                                       chainId );
    }

    private final           String    peerId;
    private final           PeerMeta  peerMeta;
    private final @Nullable Number    chainId;

    private WCSessionRequestReq(String              peerId,
                                PeerMeta            peerMeta,
                                @Nullable Number    chainId ) {
        this.peerId = peerId;
        this.peerMeta = peerMeta;
        this.chainId = chainId;
    }

    // getters

    @JsonProperty("peerId")
    public String getPeerId() { return peerId; }

    @JsonProperty("peerMeta")
    public PeerMeta getPeerMeta() { return peerMeta; }

    @JsonProperty("chainId")
    public Optional<Number> getChainId() { return Optional.fromNullable(chainId); }

}
