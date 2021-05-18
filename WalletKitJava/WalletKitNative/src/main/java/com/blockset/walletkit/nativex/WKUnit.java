/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.primitives.UnsignedBytes;
import com.google.common.primitives.UnsignedInteger;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKUnit extends PointerType {

    public static WKUnit createAsBase(WKCurrency currency, String uids, String name, String symbol) {
        return new WKUnit(
                WKNativeLibraryDirect.wkUnitCreateAsBase(
                        currency.getPointer(),
                        uids,
                        name,
                        symbol
                )
        );
    }

    public static WKUnit create(WKCurrency currency, String uids, String name, String symbol, WKUnit base, UnsignedInteger decimals) {
        byte decimalsAsByte = UnsignedBytes.checkedCast(decimals.longValue());
        return new WKUnit(
                WKNativeLibraryDirect.wkUnitCreate(
                        currency.getPointer(),
                        uids,
                        name,
                        symbol,
                        base.getPointer(),
                        decimalsAsByte
                )
        );
    }

    public WKUnit() {
        super();
    }

    public WKUnit(Pointer address) {
        super(address);
    }

    public String getUids() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkUnitGetUids(thisPtr).getString(0, "UTF-8");
    }

    public String getName() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkUnitGetName(thisPtr).getString(0, "UTF-8");
    }

    public String getSymbol() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkUnitGetSymbol(thisPtr).getString(0, "UTF-8");
    }

    public UnsignedInteger getDecimals() {
        Pointer thisPtr = this.getPointer();

        return UnsignedInteger.fromIntBits(UnsignedBytes.toInt(WKNativeLibraryDirect.wkUnitGetBaseDecimalOffset(thisPtr)));
    }

    public WKUnit getBaseUnit() {
        Pointer thisPtr = this.getPointer();

        return new WKUnit(WKNativeLibraryDirect.wkUnitGetBaseUnit(thisPtr));
    }

    public WKCurrency getCurrency() {
        Pointer thisPtr = this.getPointer();

        return new WKCurrency(WKNativeLibraryDirect.wkUnitGetCurrency(thisPtr));
    }

    public boolean hasCurrency(WKCurrency currency) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkUnitHasCurrency(thisPtr,  currency.getPointer());
    }

    public boolean isCompatible(WKUnit other) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkUnitIsCompatible(thisPtr, other.getPointer());
    }

    public boolean isIdentical(WKUnit other) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkUnitIsIdentical(thisPtr, other.getPointer());
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkUnitGive(thisPtr);
    }
}
