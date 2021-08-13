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
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKFeeBasis extends PointerType {

    public WKFeeBasis() {
        super();
    }

    public WKFeeBasis(Pointer address) {
        super(address);
    }

    public double getCostFactor() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkFeeBasisGetCostFactor(thisPtr);
    }

    public WKUnit getPricePerCostFactorUnit() {
        Pointer thisPtr = this.getPointer();

        return getPricePerCostFactor().getUnit();
    }

    public WKAmount getPricePerCostFactor() {
        Pointer thisPtr = this.getPointer();

        return new WKAmount(WKNativeLibraryDirect.wkFeeBasisGetPricePerCostFactor(thisPtr));
    }

    public Optional<WKAmount> getFee() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(WKNativeLibraryDirect.wkFeeBasisGetFee(thisPtr)).transform(WKAmount::new);
    }

    public boolean isIdentical(WKFeeBasis other) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkFeeBasisIsEqual(thisPtr, other.getPointer());
    }

    public WKFeeBasis take() {
        return new WKFeeBasis(
            WKNativeLibraryDirect.wkFeeBasisTake(
                    this.getPointer()
        ));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkFeeBasisGive(thisPtr);
    }
}
