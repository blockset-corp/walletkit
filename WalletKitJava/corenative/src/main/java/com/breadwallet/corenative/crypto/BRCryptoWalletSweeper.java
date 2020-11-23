/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.breadwallet.corenative.utility.SizeT;
import com.google.common.base.Optional;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class BRCryptoWalletSweeper extends PointerType {

    public BRCryptoWalletSweeper() {
        super();
    }

    public BRCryptoWalletSweeper(Pointer address) {
        super(address);
    }

    public static BRCryptoWalletSweeperStatus validateSupported(BRCryptoWalletManager cwm,
                                                                BRCryptoWallet wallet,
                                                                BRCryptoKey key) {
        return BRCryptoWalletSweeperStatus.fromCore(
                CryptoLibraryDirect.cryptoWalletManagerWalletSweeperValidateSupported(
                        cwm.getPointer(),
                        wallet.getPointer(),
                        key.getPointer()
                )
        );
    }

    public static BRCryptoWalletSweeper createAsBtc(BRCryptoWalletManager cwm,
                                                    BRCryptoWallet wallet,
                                                    BRCryptoKey key) {
        return new BRCryptoWalletSweeper(
                CryptoLibraryDirect.cryptoWalletManagerCreateWalletSweeper(
                        cwm.getPointer(),
                        wallet.getPointer(),
                        key.getPointer()
                )
        );
    }

    public BRCryptoKey getKey() {
        Pointer thisPtr = this.getPointer();

        return new BRCryptoKey(CryptoLibraryDirect.cryptoWalletSweeperGetKey(thisPtr));
    }

    public Optional<BRCryptoAmount> getBalance() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoWalletSweeperGetBalance(
                        thisPtr
                )
        ).transform(BRCryptoAmount::new);
    }

    public Optional<BRCryptoAddress> getAddress() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                CryptoLibraryDirect.cryptoWalletSweeperGetAddress(thisPtr)
        ).transform(BRCryptoAddress::new);
    }

    public BRCryptoWalletSweeperStatus handleTransactionAsBtc(byte[] transaction) {
        Pointer thisPtr = this.getPointer();

        //TODO:SWEEP use transaction bundle
        return BRCryptoWalletSweeperStatus.fromCore(
                CryptoLibraryDirect.cryptoWalletSweeperAddTransactionFromBundle(
                        thisPtr,
                        transaction,
                        new SizeT(transaction.length))
        );
    }

    public BRCryptoWalletSweeperStatus validate() {
        Pointer thisPtr = this.getPointer();

        return BRCryptoWalletSweeperStatus.fromCore(
                CryptoLibraryDirect.cryptoWalletSweeperValidate(thisPtr)
        );
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        CryptoLibraryDirect.cryptoWalletSweeperRelease(thisPtr);
    }
}
