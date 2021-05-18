/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.base.Optional;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKAddress extends PointerType {

    public static Optional<WKAddress> create(String address, WKNetwork network) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkNetworkCreateAddress(
                        network.getPointer(),
                        address
                )
        ).transform(WKAddress::new);
    }

    public WKAddress() {
        super();
    }

    public WKAddress(Pointer address) {
        super(address);
    }

    public boolean isIdentical(WKAddress o) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAddressIsIdentical(thisPtr, o.getPointer());
    }

    @Override
    public String toString() {
        Pointer thisPtr = this.getPointer();

        Pointer addressPtr = WKNativeLibraryDirect.wkAddressAsString(thisPtr);
        try {
            return addressPtr.getString(0, "UTF-8");
        } finally {
            Native.free(Pointer.nativeValue(addressPtr));
        }
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkAddressGive(thisPtr);
    }
}
