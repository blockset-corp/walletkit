/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKHash extends PointerType {

    public WKHash() {
        super();
    }

    public WKHash(Pointer address) {
        super(address);
    }

    public int getValue() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkHashGetHashValue(thisPtr);
    }

    public boolean isIdentical(WKHash o) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkHashEqual(thisPtr, o.getPointer());
    }

    @Override
    public String toString() {
        Pointer thisPtr = this.getPointer();

        Pointer ptr = WKNativeLibraryDirect.wkHashEncodeString(thisPtr);
        try {
            return ptr.getString(0, "UTF-8");
        } finally {
            Native.free(Pointer.nativeValue(ptr));
        }
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkHashGive(thisPtr);
    }
}
