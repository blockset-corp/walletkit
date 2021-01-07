/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

public class BRCryptoWalletEvent extends PointerType {

    public BRCryptoWalletEvent() {
        super();
    }

    public BRCryptoWalletEvent(Pointer address) {
        super(address);
    }

    public BRCryptoWalletEventType type() {
        return BRCryptoWalletEventType.fromCore(
                CryptoLibraryDirect.cryptoWalletEventGetType(
                        this.getPointer()));
    }

    public class States {
        public final BRCryptoWalletState oldState;
        public final BRCryptoWalletState newState;

        private States(BRCryptoWalletState oldState, BRCryptoWalletState newState) {
            this.oldState = oldState;
            this.newState = newState;
        }

    }

    public States states() {
        IntByReference oldState = new IntByReference();
        IntByReference newState = new IntByReference();

        if (BRCryptoBoolean.CRYPTO_FALSE ==
            CryptoLibraryDirect.cryptoWalletEventExtractState(this.getPointer(), oldState, newState))
            throw new IllegalStateException();

        return new States (
                BRCryptoWalletState.fromCore (oldState.getValue()),
                BRCryptoWalletState.fromCore (newState.getValue()));
    }

    public BRCryptoTransfer transfer() {
        PointerByReference transferPtr = new PointerByReference();

        if (BRCryptoBoolean.CRYPTO_FALSE ==
                CryptoLibraryDirect.cryptoWalletEventExtractTransfer(this.getPointer(), transferPtr))
            throw new IllegalStateException();

        return new BRCryptoTransfer (transferPtr.getValue());
    }

    public BRCryptoTransfer transferSubmit() {
        PointerByReference transferPtr = new PointerByReference();

        if (BRCryptoBoolean.CRYPTO_FALSE ==
                CryptoLibraryDirect.cryptoWalletEventExtractTransferSubmit(this.getPointer(), transferPtr))
            throw new IllegalStateException();

        return new BRCryptoTransfer (transferPtr.getValue());
    }

    public BRCryptoAmount balance() {
        PointerByReference balancePtr = new PointerByReference();

        if (BRCryptoBoolean.CRYPTO_FALSE ==
                CryptoLibraryDirect.cryptoWalletEventExtractBalanceUpdate(this.getPointer(), balancePtr))
            throw new IllegalStateException();

        return new BRCryptoAmount (balancePtr.getValue());
    }

    public BRCryptoFeeBasis feeBasisUpdate() {
        PointerByReference feeBasisPtr = new PointerByReference();

        if (BRCryptoBoolean.CRYPTO_FALSE ==
                CryptoLibraryDirect.cryptoWalletEventExtractFeeBasisUpdate(this.getPointer(), feeBasisPtr))
            throw new IllegalStateException();

        return new BRCryptoFeeBasis (feeBasisPtr.getValue());
    }

    public class FeeBasisEstimate {
        public final BRCryptoStatus status;
        public final Pointer cookie;
        public final BRCryptoFeeBasis basis; // must be given

        public FeeBasisEstimate(BRCryptoStatus status, Pointer cookie, BRCryptoFeeBasis basis) {
            this.status = status;
            this.cookie = cookie;
            this.basis = basis;
        }
    }

    public FeeBasisEstimate feeBasisEstimate() {
        IntByReference statusPtr = new IntByReference();
        PointerByReference cookiePtr = new PointerByReference();
        PointerByReference feeBasisPtr = new PointerByReference();

        if (BRCryptoBoolean.CRYPTO_FALSE ==
                CryptoLibraryDirect.cryptoWalletEventExtractFeeBasisEstimate(this.getPointer(), statusPtr, cookiePtr, feeBasisPtr))
            throw new IllegalStateException();

        return new FeeBasisEstimate(
                BRCryptoStatus.fromCore(statusPtr.getValue()),
                cookiePtr.getValue(),
                new BRCryptoFeeBasis (feeBasisPtr.getValue()));
    }

    public BRCryptoWallet take() {
        return new BRCryptoWallet(
                CryptoLibraryDirect.cryptoWalletEventTake(
                        this.getPointer()));
    }

    public void give() {
        CryptoLibraryDirect.cryptoWalletEventGive(
                this.getPointer());
    }
}
