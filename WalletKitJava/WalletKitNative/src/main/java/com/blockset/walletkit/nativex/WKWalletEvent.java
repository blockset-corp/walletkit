/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

public class WKWalletEvent extends PointerType {

    public WKWalletEvent() {
        super();
    }

    public WKWalletEvent(Pointer address) {
        super(address);
    }

    public WKWalletEventType type() {
        return WKWalletEventType.fromCore(
                WKNativeLibraryDirect.wkWalletEventGetType(
                        this.getPointer()));
    }

    public class States {
        public final WKWalletState oldState;
        public final WKWalletState newState;

        private States(WKWalletState oldState, WKWalletState newState) {
            this.oldState = oldState;
            this.newState = newState;
        }
    }

    public States states() {
        IntByReference oldState = new IntByReference();
        IntByReference newState = new IntByReference();

        if (WKBoolean.WK_FALSE ==
            WKNativeLibraryDirect.wkWalletEventExtractState(this.getPointer(), oldState, newState))
            throw new IllegalStateException();

        return new States (
                WKWalletState.fromCore (oldState.getValue()),
                WKWalletState.fromCore (newState.getValue()));
    }

    public WKTransfer transfer() {
        PointerByReference transferPtr = new PointerByReference();

        if (WKBoolean.WK_FALSE ==
                WKNativeLibraryDirect.wkWalletEventExtractTransfer(this.getPointer(), transferPtr))
            throw new IllegalStateException();

        return new WKTransfer(transferPtr.getValue());
    }

    public WKTransfer transferSubmit() {
        PointerByReference transferPtr = new PointerByReference();

        if (WKBoolean.WK_FALSE ==
                WKNativeLibraryDirect.wkWalletEventExtractTransferSubmit(this.getPointer(), transferPtr))
            throw new IllegalStateException();

        return new WKTransfer(transferPtr.getValue());
    }

    public WKAmount balance() {
        PointerByReference balancePtr = new PointerByReference();

        if (WKBoolean.WK_FALSE ==
                WKNativeLibraryDirect.wkWalletEventExtractBalanceUpdate(this.getPointer(), balancePtr))
            throw new IllegalStateException();

        return new WKAmount(balancePtr.getValue());
    }

    public WKFeeBasis feeBasisUpdate() {
        PointerByReference feeBasisPtr = new PointerByReference();

        if (WKBoolean.WK_FALSE ==
                WKNativeLibraryDirect.wkWalletEventExtractFeeBasisUpdate(this.getPointer(), feeBasisPtr))
            throw new IllegalStateException();

        return new WKFeeBasis(feeBasisPtr.getValue());
    }

    public class FeeBasisEstimate {
        public final WKStatus status;
        public final Pointer cookie;
        public final WKFeeBasis basis; // must be given

        public FeeBasisEstimate(WKStatus status, Pointer cookie, WKFeeBasis basis) {
            this.status = status;
            this.cookie = cookie;
            this.basis = basis;
        }
    }

    public FeeBasisEstimate feeBasisEstimate() {
        IntByReference statusPtr = new IntByReference();
        PointerByReference cookiePtr = new PointerByReference();
        PointerByReference feeBasisPtr = new PointerByReference();

        if (WKBoolean.WK_FALSE ==
                WKNativeLibraryDirect.wkWalletEventExtractFeeBasisEstimate(this.getPointer(), statusPtr, cookiePtr, feeBasisPtr))
            throw new IllegalStateException();

        return new FeeBasisEstimate(
                WKStatus.fromCore(statusPtr.getValue()),
                cookiePtr.getValue(),
                new WKFeeBasis(feeBasisPtr.getValue()));
    }

    public WKWalletEvent take() {
        return new WKWalletEvent(
                WKNativeLibraryDirect.wkWalletEventTake(
                        this.getPointer()));
    }

    public void give() {
        WKNativeLibraryDirect.wkWalletEventGive(
                this.getPointer());
    }
}
