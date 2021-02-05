package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryIndirect;
import com.breadwallet.corenative.utility.SizeT;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.util.Map;

public class BRCryptoClientTransferBundle extends PointerType {
    public static BRCryptoClientTransferBundle create(
            BRCryptoTransferStateType status,
            String uids,
            String hash,
            String identifier,
            String from,
            String to,
            String amount,
            String currency,
            String fee,
            UnsignedLong blockTimestamp,
            UnsignedLong blockHeight,
            UnsignedLong blockConfirmations,
            UnsignedLong blockTransactionIndex,
            String blockHash,
            Map<String, String> meta) {

        int metaCount = meta.size();
        String[] metaKeys = meta.keySet().toArray(new String[metaCount]);
        String[] metaVals = meta.values().toArray(new String[metaCount]);


        Pointer pointer = CryptoLibraryIndirect.cryptoClientTransferBundleCreate(
                status.toCore(),
                uids,
                hash,
                identifier,
                from,
                to,
                amount,
                currency,
                fee,
                blockTimestamp.longValue(),
                blockHeight.longValue(),
                blockConfirmations.longValue(),
                blockTransactionIndex.longValue(),
                blockHash,
                new SizeT(metaCount),
                metaKeys,
                metaVals);

        return new BRCryptoClientTransferBundle(pointer);
    }

    public BRCryptoClientTransferBundle() {
        super();
    }

    public BRCryptoClientTransferBundle(Pointer address) {
        super(address);
    }

}
