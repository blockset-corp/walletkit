/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKCurrency extends PointerType {

    public static WKCurrency create(String uids, String name, String code, String type, String issuer) {
        return new WKCurrency(WKNativeLibraryDirect.wkCurrencyCreate(uids, name, code, type, issuer));
    }

    public WKCurrency(Pointer address) {
        super(address);
    }

    public WKCurrency() {
        super();
    }

    public String getUids() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkCurrencyGetUids(thisPtr).getString(0, "UTF-8");
    }

    public String getName() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkCurrencyGetName(thisPtr).getString(0, "UTF-8");
    }

    public String getCode() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkCurrencyGetCode(thisPtr).getString(0, "UTF-8");
    }

    public String getType() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkCurrencyGetType(thisPtr).getString(0, "UTF-8");
    }

    public String getIssuer() {
        Pointer thisPtr = this.getPointer();

        Pointer issuer = WKNativeLibraryDirect.wkCurrencyGetIssuer(thisPtr);
        return issuer == null ? null : issuer.getString(0, "UTF-8");
    }

    public boolean isIdentical(WKCurrency o) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkCurrencyIsIdentical(thisPtr, o.getPointer());
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkCurrencyGive(thisPtr);
    }
}
