/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.library.WKNativeLibraryIndirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.blockset.walletkit.nativex.utility.SizeTByReference;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInts;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.ptr.IntByReference;

import java.util.ArrayList;
import java.util.List;

import javax.annotation.Nullable;

public class WKWallet extends PointerType {

    public WKWallet() {
        super();
    }

    public WKWallet(Pointer address) {
        super(address);
    }

    public WKAmount getBalance() {
        Pointer thisPtr = this.getPointer();

        return new WKAmount(WKNativeLibraryDirect.wkWalletGetBalance(thisPtr));
    }

    public Optional<WKAmount> getBalanceMaximum () {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletGetBalanceMaximum(thisPtr)
        ).transform(WKAmount::new);
    }

    public Optional<WKAmount> getBalanceMinimum () {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletGetBalanceMinimum(thisPtr)
        ).transform(WKAmount::new);
    }

    public List<WKTransfer> getTransfers() {
        Pointer thisPtr = this.getPointer();

        List<WKTransfer> transfers = new ArrayList<>();
        SizeTByReference count = new SizeTByReference();
        Pointer transfersPtr = WKNativeLibraryDirect.wkWalletGetTransfers(thisPtr, count);
        if (null != transfersPtr) {
            try {
                int transfersSize = UnsignedInts.checkedCast(count.getValue().longValue());
                for (Pointer transferPtr: transfersPtr.getPointerArray(0, transfersSize)) {
                    transfers.add(new WKTransfer(transferPtr));
                }

            } finally {
                Native.free(Pointer.nativeValue(transfersPtr));
            }
        }
        return transfers;
    }


    public boolean containsTransfer(WKTransfer transfer) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkWalletHasTransfer(thisPtr, transfer.getPointer());
    }

    public UnsignedLong getTransferAttributeCount(@Nullable WKAddress target) {
        Pointer thisPtr = this.getPointer();
        Pointer targetPtr = (null == target ? null : target.getPointer());

        return UnsignedLong.fromLongBits(
                WKNativeLibraryDirect.wkWalletGetTransferAttributeCount(
                        thisPtr,
                        targetPtr
                ).longValue()
        );
    }

    public Optional<WKTransferAttribute> getTransferAttributeAt(@Nullable WKAddress target, UnsignedLong index) {
        Pointer thisPtr = this.getPointer();
        Pointer targetPtr = (null == target ? null : target.getPointer());

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletGetTransferAttributeAt(
                        thisPtr,
                        targetPtr,
                        new SizeT(index.longValue())
                )
        ).transform(WKTransferAttribute::new);
    }

    public Optional<WKTransferAttributeValidationError> validateTransferAttribute(WKTransferAttribute attribute) {
        Pointer thisPtr = this.getPointer();

        IntByReference validates = new IntByReference(WKBoolean.WK_FALSE);

        WKTransferAttributeValidationError error = WKTransferAttributeValidationError.fromCore(
                WKNativeLibraryDirect.wkWalletValidateTransferAttribute(
                        thisPtr,
                        attribute.getPointer(),
                        validates
                ));

        return (WKBoolean.WK_TRUE == validates.getValue()
                ? Optional.absent()
                : Optional.of(error));
    }

    public Optional<WKTransferAttributeValidationError> validateTransferAttributes(List<WKTransferAttribute> attributes) {
        Pointer thisPtr = this.getPointer();

        IntByReference validates = new IntByReference(WKBoolean.WK_FALSE);

        int attributesCount  = attributes.size();
        WKTransferAttribute[] attributeRefs = new WKTransferAttribute[attributesCount];
        for (int i = 0; i < attributesCount; i++) attributeRefs[i] = attributes.get(i);

        WKTransferAttributeValidationError error = WKTransferAttributeValidationError.fromCore(
                WKNativeLibraryIndirect.wkWalletValidateTransferAttributes(
                        thisPtr,
                        new SizeT(attributes.size()),
                        attributeRefs,
                        validates
                ));

        return (WKBoolean.WK_TRUE == validates.getValue()
                ? Optional.absent()
                : Optional.of(error));
    }

    public WKCurrency getCurrency() {
        Pointer thisPtr = this.getPointer();

        return new WKCurrency(WKNativeLibraryDirect.wkWalletGetCurrency(thisPtr));
    }

    public WKUnit getUnitForFee() {
        Pointer thisPtr = this.getPointer();

        return new WKUnit(WKNativeLibraryDirect.wkWalletGetUnitForFee(thisPtr));
    }

    public WKUnit getUnit() {
        Pointer thisPtr = this.getPointer();

        return new WKUnit(WKNativeLibraryDirect.wkWalletGetUnit(thisPtr));
    }

    public WKWalletState getState() {
        Pointer thisPtr = this.getPointer();

        return WKWalletState.fromCore(WKNativeLibraryDirect.wkWalletGetState(thisPtr));
    }

    public WKAddress getTargetAddress(WKAddressScheme addressScheme) {
        Pointer thisPtr = this.getPointer();

        return new WKAddress(WKNativeLibraryDirect.wkWalletGetAddress(thisPtr, addressScheme.toCore()));
    }

    public boolean containsAddress(WKAddress address) {
        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkWalletHasAddress(
                this.getPointer(),
                address.getPointer()
        );
    }

    public Optional<WKTransfer> createTransfer(WKAddress target, WKAmount amount,
                                               WKFeeBasis estimatedFeeBasis,
                                               List<WKTransferAttribute> attributes) {
        Pointer thisPtr = this.getPointer();

        int attributesCount  = attributes.size();
        WKTransferAttribute[] attributeRefs = new WKTransferAttribute[attributesCount];
        for (int i = 0; i < attributesCount; i++) attributeRefs[i] = attributes.get(i);

        return Optional.fromNullable(
                WKNativeLibraryIndirect.wkWalletCreateTransfer(
                        thisPtr,
                        target.getPointer(),
                        amount.getPointer(),
                        estimatedFeeBasis.getPointer(),
                        new SizeT(attributesCount),
                        attributeRefs
                )
        ).transform(WKTransfer::new);
    }

    public Optional<WKTransfer> createTransferForWalletSweep(WKWalletSweeper sweeper, WKWalletManager manager, WKFeeBasis estimatedFeeBasis) {
        Pointer thisPtr = this.getPointer();

        assert (false); // TODO: Need WalletManager?
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletSweeperCreateTransferForWalletSweep(
                        sweeper.getPointer(),
                        manager.getPointer(),
                        thisPtr,
                        estimatedFeeBasis.getPointer()
                )
        ).transform(WKTransfer::new);
    }

    public Optional<WKTransfer> createTransferForPaymentProtocolRequest(WKPaymentProtocolRequest request, WKFeeBasis estimatedFeeBasis) {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletCreateTransferForPaymentProtocolRequest(
                        thisPtr,
                        request.getPointer(),
                        estimatedFeeBasis.getPointer()
                )
        ).transform(WKTransfer::new);
    }

     public WKWallet take() {
        Pointer thisPtr = this.getPointer();

        return new WKWallet(WKNativeLibraryDirect.wkWalletTake(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletGive(thisPtr);
    }
}
