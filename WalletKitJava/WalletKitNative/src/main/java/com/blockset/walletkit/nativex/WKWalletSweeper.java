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
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKWalletSweeper extends PointerType {

    public WKWalletSweeper() {
        super();
    }

    public WKWalletSweeper(Pointer address) {
        super(address);
    }

    public static WKWalletSweeperStatus validateSupported(WKWalletManager cwm,
                                                          WKWallet wallet,
                                                          WKKey key) {
        return WKWalletSweeperStatus.fromCore(
                WKNativeLibraryDirect.wkWalletManagerWalletSweeperValidateSupported(
                        cwm.getPointer(),
                        wallet.getPointer(),
                        key.getPointer()
                )
        );
    }

    public static WKWalletSweeper createAsBtc(WKWalletManager cwm,
                                              WKWallet wallet,
                                              WKKey key) {
        return new WKWalletSweeper(
                WKNativeLibraryDirect.wkWalletManagerCreateWalletSweeper(
                        cwm.getPointer(),
                        wallet.getPointer(),
                        key.getPointer()
                )
        );
    }

    public WKKey getKey() {
        Pointer thisPtr = this.getPointer();

        return new WKKey(WKNativeLibraryDirect.wkWalletSweeperGetKey(thisPtr));
    }

    public Optional<WKAmount> getBalance() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletSweeperGetBalance(
                        thisPtr
                )
        ).transform(WKAmount::new);
    }

    public Optional<WKAddress> getAddress() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletSweeperGetAddress(thisPtr)
        ).transform(WKAddress::new);
    }

    public WKWalletSweeperStatus handleTransactionAsBtc(WKClientTransactionBundle bundle) {
        return WKWalletSweeperStatus.fromCore(
                WKNativeLibraryDirect.wkWalletSweeperAddTransactionFromBundle(
		      this.getPointer(),
		      bundle.getPointer())
        );
    }

    public WKWalletSweeperStatus validate() {
        Pointer thisPtr = this.getPointer();

        return WKWalletSweeperStatus.fromCore(
                WKNativeLibraryDirect.wkWalletSweeperValidate(thisPtr)
        );
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkWalletSweeperRelease(thisPtr);
    }
}
