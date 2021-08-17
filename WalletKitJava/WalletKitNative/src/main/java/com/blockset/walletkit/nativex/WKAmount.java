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
import com.sun.jna.ptr.IntByReference;

public class WKAmount extends PointerType {

    public static WKAmount create(double value, WKUnit unit) {
        return new WKAmount(WKNativeLibraryDirect.wkAmountCreateDouble(value, unit.getPointer()));
    }

    public static WKAmount create(long value, WKUnit unit) {
        return new WKAmount(WKNativeLibraryDirect.wkAmountCreateInteger(value, unit.getPointer()));
    }

    public static Optional<WKAmount> create(String value, boolean isNegative, WKUnit unit) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkAmountCreateString(
                        value,
                        isNegative ? WKBoolean.WK_TRUE : WKBoolean.WK_FALSE,
                        unit.getPointer())
        ).transform(WKAmount::new);
    }

    public WKAmount() {
        super();
    }

    public WKAmount(Pointer address) {
        super(address);
    }

    public WKCurrency getCurrency() {
        Pointer thisPtr = this.getPointer();

        return new WKCurrency(WKNativeLibraryDirect.wkAmountGetCurrency(thisPtr));
    }

    public WKUnit getUnit() {
        Pointer thisPtr = this.getPointer();

        return new WKUnit(WKNativeLibraryDirect.wkAmountGetUnit(thisPtr));
    }

    public Optional<Double> getDouble(WKUnit unit) {
        Pointer thisPtr = this.getPointer();

        IntByReference overflowRef = new IntByReference(WKBoolean.WK_FALSE);
        double value = WKNativeLibraryDirect.wkAmountGetDouble(thisPtr, unit.getPointer(), overflowRef);
        return overflowRef.getValue() == WKBoolean.WK_TRUE ? Optional.absent() : Optional.of(value);
    }

    public Optional<WKAmount> add(WKAmount other) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkAmountAdd(
                        thisPtr,
                        other.getPointer()
                )
        ).transform(WKAmount::new);
    }

    public Optional<WKAmount> sub(WKAmount other) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkAmountSub(
                        thisPtr,
                        other.getPointer()
                )
        ).transform(WKAmount::new);
    }

    public WKAmount negate() {
        Pointer thisPtr = this.getPointer();

        return new WKAmount(WKNativeLibraryDirect.wkAmountNegate(thisPtr));
    }

    public Optional<WKAmount> convert(WKUnit toUnit) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkAmountConvertToUnit(
                        thisPtr,
                        toUnit.getPointer()
                )
        ).transform(WKAmount::new);
    }

    public boolean isNegative() {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAmountIsNegative(thisPtr);
    }

    public boolean isZero() {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAmountIsZero(thisPtr);
    }

    public WKComparison compare(WKAmount other) {
        Pointer thisPtr = this.getPointer();

        return WKComparison.fromCore(WKNativeLibraryDirect.wkAmountCompare(thisPtr, other.getPointer()));
    }

    public boolean isCompatible(WKAmount amount) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAmountIsCompatible(thisPtr, amount.getPointer());
    }

    public boolean hasCurrency(WKCurrency currency) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAmountHasCurrency(thisPtr, currency.getPointer());
    }

    public String toStringWithBase(int base, String preface) {
        Pointer thisPtr = this.getPointer();

        Pointer ptr = WKNativeLibraryDirect.wkAmountGetStringPrefaced(thisPtr, base, preface);
        try {
            return ptr.getString(0, "UTF-8");
        } finally {
            Native.free(Pointer.nativeValue(ptr));
        }
    }

    public WKAmount take() {
        return new WKAmount(
                WKNativeLibraryDirect.wkAmountTake(
                        this.getPointer()
                ));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkAmountGive(thisPtr);
    }
}
