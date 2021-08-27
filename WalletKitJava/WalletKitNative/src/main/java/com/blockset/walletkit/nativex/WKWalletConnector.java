/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.google.common.base.Optional;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKWalletConnector extends PointerType {

    public WKWalletConnector() {
        super ();
    }

    public WKWalletConnector(Pointer address) {
        super (address);
    }

    public static Optional<WKWalletConnector> create(WKWalletManager coreManager) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletConnectorCreate(coreManager.getPointer())
        ).transform(WKWalletConnector::new);
    }

    public void give() {
        Pointer thisPtr = this.getPointer();
        WKNativeLibraryDirect.wkWalletConnectorRelease(thisPtr);
    }

    /* TODO: Hook WKWalletConnector methods to the actual wkMethods exposed by WKWalletConnector.h
       as needed by WalletConnector.
     */
}
