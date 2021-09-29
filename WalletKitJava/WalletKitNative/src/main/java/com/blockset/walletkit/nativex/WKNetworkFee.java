/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKNetworkFee extends PointerType {

    public static WKNetworkFee create(UnsignedLong timeIntervalInMilliseconds,
                                      WKAmount pricePerCostFactor,
                                      WKUnit pricePerCostFactorUnit) {
        return new WKNetworkFee(
                WKNativeLibraryDirect.wkNetworkFeeCreate(
                        timeIntervalInMilliseconds.longValue(),
                        pricePerCostFactor.getPointer(),
                        pricePerCostFactorUnit.getPointer()
                )
        );
    }

    public WKNetworkFee() {
        super();
    }

    public WKNetworkFee(Pointer address) {
        super(address);
    }

    public UnsignedLong getConfirmationTimeInMilliseconds() {
        Pointer thisPtr = this.getPointer();

        return UnsignedLong.valueOf(WKNativeLibraryDirect.wkNetworkFeeGetConfirmationTimeInMilliseconds(thisPtr));
    }

    public WKAmount getPricePerCostFactor() {
        Pointer thisPtr = this.getPointer();

        return new WKAmount(WKNativeLibraryDirect.wkNetworkFeeGetPricePerCostFactor(thisPtr));
    }

    public boolean isIdentical(WKNetworkFee other) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkNetworkFeeEqual(thisPtr, other.getPointer());
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkFeeGive(thisPtr);
    }
}
