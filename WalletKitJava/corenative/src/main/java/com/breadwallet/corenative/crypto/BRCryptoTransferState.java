/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.LongByReference;
import com.sun.jna.ptr.PointerByReference;

public class BRCryptoTransferState extends PointerType {

    public BRCryptoTransferState() { super(); }

    public BRCryptoTransferState(Pointer address) { super (address); }

    public BRCryptoTransferStateType type() {
        return BRCryptoTransferStateType.fromCore(
                CryptoLibraryDirect.cryptoTransferStateGetType(
                        this.getPointer()));
    }

    private static int CRYPTO_TRANSFER_INCLUDED_ERROR_SIZE = 16;

    public class Included {

        public final UnsignedLong blockNumber;
        public final UnsignedLong blockTimestamp;
        public final UnsignedLong transactionIndex;
        public final BRCryptoFeeBasis feeBasis;
        public final boolean success;
        public Optional<String> error;

        public Included(UnsignedLong blockNumber,
                        UnsignedLong blockTimestamp,
                        UnsignedLong transactionIndex,
                        BRCryptoFeeBasis feeBasis,
                        boolean success,
                        Optional<String> error) {
            this.blockNumber = blockNumber;
            this.blockTimestamp = blockTimestamp;
            this.transactionIndex = transactionIndex;
            this.feeBasis = feeBasis;
            this.success = success;
            this.error = error;
        }
    }

    public Included included() {
        LongByReference blockNumber      = new LongByReference();
        LongByReference blockTimestamp   = new LongByReference();
        LongByReference transactionIndex = new LongByReference();

        PointerByReference feeBasis = new PointerByReference();
        IntByReference     success  = new IntByReference();
        PointerByReference error    = new PointerByReference();

        if (BRCryptoBoolean.CRYPTO_FALSE ==
                CryptoLibraryDirect.cryptoTransferStateExtractIncluded(
                        this.getPointer(),
                        blockNumber,
                        blockTimestamp,
                        transactionIndex,
                        feeBasis,
                        success,
                        error))
            throw new IllegalStateException();

        try {
            return new Included(
                    UnsignedLong.fromLongBits(blockNumber.getValue()),
                    UnsignedLong.fromLongBits(blockTimestamp.getValue()),
                    UnsignedLong.fromLongBits(transactionIndex.getValue()),
                    new BRCryptoFeeBasis(feeBasis.getValue()),
                    BRCryptoBoolean.CRYPTO_TRUE == success.getValue(),
                    Optional.fromNullable(error.getValue()).transform(p -> p.getString(0)));
        }
        finally {
            CryptoLibraryDirect.cryptoMemoryFreeExtern(error.getValue());
        }
    }

    public BRCryptoTransferSubmitError errored() {
        BRCryptoTransferSubmitError.ByValue error = new BRCryptoTransferSubmitError.ByValue();

        if (BRCryptoBoolean.CRYPTO_FALSE ==
                CryptoLibraryDirect.cryptoTransferStateExtractError(
                        this.getPointer(),
                        error))
            throw new IllegalStateException();

        return error;
    }

    public BRCryptoTransferState take() {
        return new BRCryptoTransferState(
                CryptoLibraryDirect.cryptoTransferStateTake (
                        this.getPointer()));
    }

    public void give() {
        CryptoLibraryDirect.cryptoTransferStateGive(
                this.getPointer());
    }
}
