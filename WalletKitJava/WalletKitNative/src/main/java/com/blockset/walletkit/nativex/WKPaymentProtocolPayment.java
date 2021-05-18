/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/29/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.utility.SizeTByReference;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInts;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKPaymentProtocolPayment extends PointerType {

    public static Optional<WKPaymentProtocolPayment> create(WKPaymentProtocolRequest request,
                                                            WKTransfer transfer,
                                                            WKAddress refundAddress) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkPaymentProtocolPaymentCreate(
                        request.getPointer(),
                        transfer.getPointer(),
                        refundAddress.getPointer())
        ).transform(
                WKPaymentProtocolPayment::new
        );
    }

    public WKPaymentProtocolPayment() {
        super();
    }

    public WKPaymentProtocolPayment(Pointer address) {
        super(address);
    }

    public Optional<byte[]> encode() {
        Pointer thisPtr = this.getPointer();

        SizeTByReference length = new SizeTByReference(UnsignedLong.ZERO);
        Pointer returnValue = WKNativeLibraryDirect.wkPaymentProtocolPaymentEncode(thisPtr, length);
        try {
            return Optional.fromNullable(returnValue)
                    .transform(v -> v.getByteArray(0, UnsignedInts.checkedCast(length.getValue().longValue())));
        } finally {
            if (returnValue != null) Native.free(Pointer.nativeValue(returnValue));
        }
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkPaymentProtocolPaymentGive(thisPtr);
    }
}
