/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKClientTransactionBundle extends PointerType {
    public static WKClientTransactionBundle create(
            WKTransferStateType status,
            byte[] transaction,
            UnsignedLong blockTimestamp,
            UnsignedLong blockHeight) {
        Pointer pointer = WKNativeLibraryDirect.wkClientTransactionBundleCreate(
                status.toCore(),
                transaction,
                new SizeT(transaction.length),
                blockTimestamp.longValue(),
                blockHeight.longValue());

        return new WKClientTransactionBundle(pointer);
    }

    public void release () {
        WKNativeLibraryDirect.wkClientTransactionBundleRelease(
                this.getPointer()
        );
    }

    public WKClientTransactionBundle() {
        super();
    }

    public WKClientTransactionBundle(Pointer address) {
        super(address);
    }
}
