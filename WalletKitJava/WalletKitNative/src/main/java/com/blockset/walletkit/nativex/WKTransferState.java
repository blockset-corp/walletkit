/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.LongByReference;
import com.sun.jna.ptr.PointerByReference;

public class WKTransferState extends PointerType {

    public WKTransferState() { super(); }

    public WKTransferState(Pointer address) { super (address); }

    public WKTransferStateType type() {
        return WKTransferStateType.fromCore(
                WKNativeLibraryDirect.wkTransferStateGetType(
                        this.getPointer()));
    }

    private static int CRYPTO_TRANSFER_INCLUDED_ERROR_SIZE = 16;

    public class Included {

        public final UnsignedLong blockNumber;
        public final UnsignedLong blockTimestamp;
        public final UnsignedLong transactionIndex;
        public final WKFeeBasis feeBasis;
        public final WKTransferIncludeStatus status;

        public Included(UnsignedLong blockNumber,
                        UnsignedLong blockTimestamp,
                        UnsignedLong transactionIndex,
                        WKFeeBasis feeBasis,
                        WKTransferIncludeStatus status) {
            this.blockNumber = blockNumber;
            this.blockTimestamp = blockTimestamp;
            this.transactionIndex = transactionIndex;
            this.feeBasis = feeBasis;
            this.status = status;
        }
    }

    public Included included() {
        LongByReference blockNumber      = new LongByReference();
        LongByReference blockTimestamp   = new LongByReference();
        LongByReference transactionIndex = new LongByReference();

        PointerByReference feeBasis = new PointerByReference();
        WKTransferIncludeStatus.ByValue status = new WKTransferIncludeStatus.ByValue();

        if (WKBoolean.WK_FALSE ==
                WKNativeLibraryDirect.wkTransferStateExtractIncluded(
                        this.getPointer(),
                        blockNumber,
                        blockTimestamp,
                        transactionIndex,
                        feeBasis,
                        status))
            throw new IllegalStateException();

        try {
            return new Included(
                    UnsignedLong.fromLongBits(blockNumber.getValue()),
                    UnsignedLong.fromLongBits(blockTimestamp.getValue()),
                    UnsignedLong.fromLongBits(transactionIndex.getValue()),
                    new WKFeeBasis(feeBasis.getValue()),
                    status);
        }
        finally {
 //           WKNativeLibraryDirect.wkMemoryFreeExtern(error.getValue());
        }
    }

    public WKTransferSubmitError errored() {
        WKTransferSubmitError.ByValue error = new WKTransferSubmitError.ByValue();

        if (WKBoolean.WK_FALSE ==
                WKNativeLibraryDirect.wkTransferStateExtractError(
                        this.getPointer(),
                        error))
            throw new IllegalStateException();

        return error;
    }

    public WKTransferState take() {
        return new WKTransferState(
                WKNativeLibraryDirect.wkTransferStateTake (
                        this.getPointer()));
    }

    public void give() {
        WKNativeLibraryDirect.wkTransferStateGive(
                this.getPointer());
    }
}
