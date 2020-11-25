/*
 * Created by Ehsan Rezaie <ehsan@brd.com> on 11/23/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.google.common.base.Optional;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class BRCryptoExportablePaperWallet extends PointerType {
    public BRCryptoExportablePaperWallet() { super(); }

    public BRCryptoExportablePaperWallet(Pointer address) {
        super(address);
    }

    public static BRCryptoExportablePaperWalletStatus validateSupported(BRCryptoNetwork network,
                                                                        BRCryptoCurrency currency) {
        return BRCryptoExportablePaperWalletStatus.fromCore(
                CryptoLibraryDirect.cryptoExportablePaperWalletValidateSupported(
                        network.getPointer(),
                        currency.getPointer()
                )
        );
    }

    public static BRCryptoExportablePaperWallet createAsBTC(BRCryptoNetwork network,
                                                            BRCryptoCurrency currency) {
        return new BRCryptoExportablePaperWallet(
                CryptoLibraryDirect.cryptoExportablePaperWalletCreateAsBTC(
                        network.getPointer(),
                        currency.getPointer()
                )
        );
    }

    public Optional<BRCryptoKey> getKey() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoExportablePaperWalletGetKey(thisPtr)
        ).transform(BRCryptoKey::new);
    }

    public Optional<BRCryptoAddress> getAddress() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoExportablePaperWalletGetAddress(thisPtr)
        ).transform(BRCryptoAddress::new);
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        CryptoLibraryDirect.cryptoExportablePaperWalletRelease(thisPtr);
    }
}
