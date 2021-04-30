/*
 * Created by Ehsan Rezaie <ehsan@brd.com> on 11/23/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.base.Optional;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKExportablePaperWallet extends PointerType {
    public WKExportablePaperWallet() { super(); }

    public WKExportablePaperWallet(Pointer address) {
        super(address);
    }

    public static WKExportablePaperWalletStatus validateSupported(WKNetwork network,
                                                                  WKCurrency currency) {
        return WKExportablePaperWalletStatus.fromCore(
                WKNativeLibraryDirect.wkExportablePaperWalletValidateSupported(
                        network.getPointer(),
                        currency.getPointer()
                )
        );
    }

    public static Optional<WKExportablePaperWallet> create(WKNetwork network,
                                                           WKCurrency currency) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkExportablePaperWalletCreate(
                        network.getPointer(),
                        currency.getPointer())
        ).transform(WKExportablePaperWallet::new);
    }

    public Optional<WKKey> getKey() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkExportablePaperWalletGetKey(thisPtr)
        ).transform(WKKey::new);
    }

    public Optional<WKAddress> getAddress() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkExportablePaperWalletGetAddress(thisPtr)
        ).transform(WKAddress::new);
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkExportablePaperWalletRelease(thisPtr);
    }
}
