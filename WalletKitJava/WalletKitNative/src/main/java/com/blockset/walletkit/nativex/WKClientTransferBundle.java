/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryIndirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.util.Map;

public class WKClientTransferBundle extends PointerType {
    public static WKClientTransferBundle create(
            WKTransferStateType status,
            String hash,
            String identifier,
            String uids,
            String from,
            String to,
            String amount,
            String currency,
            String fee,
            UnsignedLong transferIndex,
            UnsignedLong blockTimestamp,
            UnsignedLong blockHeight,
            UnsignedLong blockConfirmations,
            UnsignedLong blockTransactionIndex,
            String blockHash,
            Map<String, String> meta) {

        int metaCount = meta.size();
        String[] metaKeys = meta.keySet().toArray(new String[metaCount]);
        String[] metaVals = meta.values().toArray(new String[metaCount]);


        Pointer pointer = WKNativeLibraryIndirect.wkClientTransferBundleCreate(
                status.toCore(),
                hash,
                identifier,
                uids,
                from,
                to,
                amount,
                currency,
                fee,
                transferIndex.longValue(),
                blockTimestamp.longValue(),
                blockHeight.longValue(),
                blockConfirmations.longValue(),
                blockTransactionIndex.longValue(),
                blockHash,
                new SizeT(metaCount),
                metaKeys,
                metaVals);

        return new WKClientTransferBundle(pointer);
    }

    public WKClientTransferBundle() {
        super();
    }

    public WKClientTransferBundle(Pointer address) {
        super(address);
    }

}
