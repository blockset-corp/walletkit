/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.base.Optional;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import javax.annotation.Nullable;

public class WKTransferAttribute extends PointerType {
    public WKTransferAttribute() {
        super();
    }

    public WKTransferAttribute(Pointer address) {
        super(address);
    }

    public String getKey() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkTransferAttributeGetKey(thisPtr).getString(0, "UTF-8");
    }

    public Optional<String> getValue() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable (WKNativeLibraryDirect.wkTransferAttributeGetValue(thisPtr))
                .transform(v -> v.getString(0, "UTF-8"));
    }

    public void setValue(@Nullable String value) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkTransferAttributeSetValue(thisPtr, value);
    }

    public boolean isRequired () {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkTransferAttributeIsRequired(thisPtr);
    }

    public WKTransferAttribute copy () {
        Pointer thisPtr = this.getPointer();

        return new WKTransferAttribute(WKNativeLibraryDirect.wkTransferAttributeCopy (thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkTransferAttributeGive(thisPtr);
    }

}
