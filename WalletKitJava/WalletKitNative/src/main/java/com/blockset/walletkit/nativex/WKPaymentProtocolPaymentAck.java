/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/29/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.google.common.base.Optional;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKPaymentProtocolPaymentAck extends PointerType {

    public static Optional<WKPaymentProtocolPaymentAck> createForBip70(byte[] serialization) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkPaymentProtocolPaymentACKCreateForBip70(
                        serialization,
                        new SizeT(serialization.length)
                )
        ).transform(
                WKPaymentProtocolPaymentAck::new
        );
    }

    public WKPaymentProtocolPaymentAck() {
        super();
    }

    public WKPaymentProtocolPaymentAck(Pointer address) {
        super(address);
    }

    public Optional<String> getMemo() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(WKNativeLibraryDirect.wkPaymentProtocolPaymentACKGetMemo(thisPtr))
                .transform(v -> v.getString(0, "UTF-8"));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkPaymentProtocolPaymentACKGive(thisPtr);
    }
}
