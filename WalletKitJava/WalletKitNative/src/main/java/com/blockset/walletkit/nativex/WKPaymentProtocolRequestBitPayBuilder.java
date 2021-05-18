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
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.util.Date;
import java.util.concurrent.TimeUnit;

import javax.annotation.Nullable;

public class WKPaymentProtocolRequestBitPayBuilder extends PointerType {

    public static Optional<WKPaymentProtocolRequestBitPayBuilder> create(WKNetwork network,
                                                                         WKCurrency currency,
                                                                         WKPayProtReqBitPayAndBip70Callbacks callbacks,
                                                                         String networkName,
                                                                         Date time,
                                                                         Date expires,
                                                                         double feePerByte,
                                                                         String memo,
                                                                         String paymentUrl,
                                                                         @Nullable byte[] merchantData) {
        return Optional.fromNullable(
            WKNativeLibraryDirect.wkPaymentProtocolRequestBitPayBuilderCreate(
                    network.getPointer(),
                    currency.getPointer(),
                    callbacks.toByValue(),
                    networkName,
                    TimeUnit.MILLISECONDS.toSeconds(time.getTime()),
                    TimeUnit.MILLISECONDS.toSeconds(expires.getTime()),
                    feePerByte,
                    memo,
                    paymentUrl,
                    merchantData,
                    new SizeT(null == merchantData ? 0 : merchantData.length)
            )
        ).transform(
                WKPaymentProtocolRequestBitPayBuilder::new
        );
    }

    public WKPaymentProtocolRequestBitPayBuilder() {
        super();
    }

    public WKPaymentProtocolRequestBitPayBuilder(Pointer address) {
        super(address);
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkPaymentProtocolRequestBitPayBuilderGive(thisPtr);
    }

    public void addOutput(String address, UnsignedLong amount) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkPaymentProtocolRequestBitPayBuilderAddOutput(thisPtr, address, amount.longValue());
    }

    public Optional<WKPaymentProtocolRequest> build() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkPaymentProtocolRequestBitPayBuilderBuild(thisPtr)
        ).transform(
                WKPaymentProtocolRequest::new
        );
    }
}
