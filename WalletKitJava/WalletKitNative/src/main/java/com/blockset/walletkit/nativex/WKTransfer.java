/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKTransfer extends PointerType {

    public WKTransfer() {
        super();
    }

    public WKTransfer(Pointer address) {
        super(address);
    }

    public Optional<WKAddress> getSourceAddress() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkTransferGetSourceAddress(
                        thisPtr
                )
        ).transform(WKAddress::new);
    }

    public Optional<WKAddress> getTargetAddress() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkTransferGetTargetAddress(
                        thisPtr
                )
        ).transform(WKAddress::new);
    }

    public WKAmount getAmount() {
        Pointer thisPtr = this.getPointer();

        return new WKAmount(WKNativeLibraryDirect.wkTransferGetAmount(thisPtr));
    }

    public WKAmount getAmountDirected() {
        Pointer thisPtr = this.getPointer();

        return new WKAmount(WKNativeLibraryDirect.wkTransferGetAmountDirected(thisPtr));
    }

    public Optional<String> getIdentifier() {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkTransferGetIdentifier (
                        this.getPointer()
                )
        ).transform((s) -> s.getString(0, "UTF-8"));
    }

    public Optional<WKHash> getHash() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkTransferGetHash(
                        thisPtr
                )
        ).transform(WKHash::new);
    }

    public WKTransferDirection getDirection() {
        Pointer thisPtr = this.getPointer();

        return WKTransferDirection.fromCore(WKNativeLibraryDirect.wkTransferGetDirection(thisPtr));
    }

    public WKTransferState getState() {
        return new WKTransferState(
            WKNativeLibraryDirect.wkTransferGetState(
                    this.getPointer()));
    }

    public Optional<WKFeeBasis> getEstimatedFeeBasis() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkTransferGetEstimatedFeeBasis(
                        thisPtr
                )
        ).transform(WKFeeBasis::new);
    }

    public Optional<WKFeeBasis> getConfirmedFeeBasis() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkTransferGetConfirmedFeeBasis(
                        thisPtr
                )
        ).transform(WKFeeBasis::new);
    }

    public UnsignedLong getAttributeCount() {
        Pointer thisPtr = this.getPointer();

        return UnsignedLong.fromLongBits(
                WKNativeLibraryDirect.wkTransferGetAttributeCount(
                        thisPtr
                ).longValue()
        );
    }

    public Optional<WKTransferAttribute> getAttributeAt(UnsignedLong index) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkTransferGetAttributeAt(
                        thisPtr,
                        new SizeT(index.longValue())
                )
        ).transform(WKTransferAttribute::new);
    }

    public WKUnit getUnitForFee() {
        Pointer thisPtr = this.getPointer();

        return new WKUnit(WKNativeLibraryDirect.wkTransferGetUnitForFee(thisPtr));
    }

    public WKUnit getUnitForAmount() {
        Pointer thisPtr = this.getPointer();

        return new WKUnit(WKNativeLibraryDirect.wkTransferGetUnitForAmount(thisPtr));
    }

    public boolean isIdentical(WKTransfer other) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkTransferEqual(thisPtr, other.getPointer());
    }

    public WKTransfer take() {
        Pointer thisPtr = this.getPointer();

        return new WKTransfer(WKNativeLibraryDirect.wkTransferTake(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkTransferGive(thisPtr);
    }
}
