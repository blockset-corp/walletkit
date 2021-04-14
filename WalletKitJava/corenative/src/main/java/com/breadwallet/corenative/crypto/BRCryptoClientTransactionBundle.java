/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.breadwallet.corenative.utility.SizeT;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class BRCryptoClientTransactionBundle extends PointerType {
    public static BRCryptoClientTransactionBundle create(
            BRCryptoTransferStateType status,
            byte[] transaction,
            UnsignedLong blockTimestamp,
            UnsignedLong blockHeight) {
        Pointer pointer = CryptoLibraryDirect.cryptoClientTransactionBundleCreate(
                status.toCore(),
                transaction,
                new SizeT(transaction.length),
                blockTimestamp.longValue(),
                blockHeight.longValue());

        return new BRCryptoClientTransactionBundle(pointer);
    }

    public BRCryptoClientTransactionBundle() {
        super();
    }

    public BRCryptoClientTransactionBundle(Pointer address) {
        super(address);
    }
}
