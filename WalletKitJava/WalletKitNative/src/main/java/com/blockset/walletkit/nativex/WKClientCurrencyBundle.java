/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.library.WKNativeLibraryIndirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.util.List;

import javax.annotation.Nullable;

public class WKClientCurrencyBundle extends PointerType {

    public static WKClientCurrencyBundle create(
            String uids,
            String name,
            String code,
            String type,
            String blockchainId,
            @Nullable String address,
            boolean verified,
            List<WKClientCurrencyDenominationBundle> denomniations) {

        return new WKClientCurrencyBundle(
                WKNativeLibraryIndirect.wkClientCurrencyBundleCreate(
                        uids,
                        name,
                        code,
                        type,
                        blockchainId,
                        address,
                        verified,
                        new SizeT(denomniations.size()),
                        denomniations.toArray(new WKClientCurrencyDenominationBundle[denomniations.size()])));
    }

    public void release () {
        WKNativeLibraryDirect.wkClientCurrencyBundleRelease(
                this.getPointer());
    }

    public WKClientCurrencyBundle() {
        super();
    }

    public WKClientCurrencyBundle(Pointer address) {
        super(address);
    }
}
