/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 9/19/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import androidx.annotation.Nullable;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKPeer;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;

import java.util.Objects;

/* package */
final class NetworkPeer implements com.blockset.walletkit.NetworkPeer {

    /* package */
    static Optional<NetworkPeer> create(com.blockset.walletkit.Network network, String address, UnsignedInteger port, @Nullable String publicKey) {
        Network cryptoNetwork = Network.from(network);
        Optional<WKPeer> core = WKPeer.create(cryptoNetwork.getCoreBRCryptoNetwork(), address, port, publicKey);
        return core.transform(NetworkPeer::create);
    }

    /* package */
    static NetworkPeer create(WKPeer core) {
        NetworkPeer peer = new NetworkPeer(core);
        ReferenceCleaner.register(peer, core::give);
        return peer;
    }

    /* package */
    static NetworkPeer from(com.blockset.walletkit.NetworkPeer peer) {
        if (peer == null) {
            return null;
        }

        if (peer instanceof NetworkPeer) {
            return (NetworkPeer) peer;
        }

        throw new IllegalArgumentException("Unsupported peer instance");
    }

    private final WKPeer core;

    private final Network network;
    private final String address;
    private final int port;

    @Nullable
    private final String publicKey;

    private NetworkPeer(WKPeer core) {
        this.core = core;
        this.network = Network.create(core.getNetwork(), false);
        this.address = core.getAddress();
        this.port = core.getPort().shortValue();
        this.publicKey = core.getPublicKey().orNull();
    }

    @Override
    public Network getNetwork() {
        return network;
    }

    @Override
    public String getAddress() {
        return address;
    }

    @Override
    public int getPort() {
        return port;
    }

    @Override
    public Optional<String> getPublicKey() {
        return Optional.fromNullable(publicKey);
    }

    @Override
    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }

        if (!(object instanceof NetworkPeer)) {
            return false;
        }

        NetworkPeer that = (NetworkPeer) object;
        return core.isIdentical(that.core);
    }

    @Override
    public int hashCode() {
        return Objects.hash(network, address, port, publicKey);
    }

    /* package */
    WKPeer getBRCryptoPeer() {
        return core;
    }
}
