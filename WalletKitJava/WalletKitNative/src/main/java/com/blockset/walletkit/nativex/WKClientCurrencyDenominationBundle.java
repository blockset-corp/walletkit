/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.primitives.UnsignedInteger;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKClientCurrencyDenominationBundle extends PointerType {
    public static WKClientCurrencyDenominationBundle create(
            String name,
            String code,
            String symbol,
            UnsignedInteger decimals) {
        return new WKClientCurrencyDenominationBundle(
                WKNativeLibraryDirect.wkClientCurrencyDenominationBundleCreate(
                name,
                code,
                symbol,
                decimals.byteValue()));
    }

    public WKClientCurrencyDenominationBundle() {
        super();
    }

    public WKClientCurrencyDenominationBundle(Pointer address) {
        super(address);
    }

}
