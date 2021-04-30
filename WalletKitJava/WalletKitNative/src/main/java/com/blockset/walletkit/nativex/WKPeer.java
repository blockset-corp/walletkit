/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 9/9/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKPeer extends PointerType {

    public static Optional<WKPeer> create(WKNetwork network, String address, UnsignedInteger port, String publicKey) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkPeerCreate(
                        network.getPointer(),
                        address,
                        port.shortValue(),
                        publicKey
                )
        ).transform(WKPeer::new);
    }

    public WKPeer() {
        super();
    }

    public WKPeer(Pointer address) {
        super(address);
    }

    public WKNetwork getNetwork() {
        Pointer thisPtr = this.getPointer();

        return new WKNetwork(WKNativeLibraryDirect.wkPeerGetNetwork(thisPtr));
    }

    public String getAddress() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkPeerGetAddress(thisPtr).getString(0, "UTF-8");
    }

    public Optional<String> getPublicKey() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkPeerGetPublicKey(
                        thisPtr
                )
        ).transform(p -> p.getString(0, "UTF-8"));
    }

    public UnsignedInteger getPort() {
        Pointer thisPtr = this.getPointer();

        return UnsignedInteger.fromIntBits(WKNativeLibraryDirect.wkPeerGetPort(thisPtr));
    }

    public boolean isIdentical(WKPeer other) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkPeerIsIdentical(thisPtr, other.getPointer());
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkPeerGive(thisPtr);
    }
}
