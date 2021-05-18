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
import com.google.common.primitives.Ints;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKCipher extends PointerType {

    public static Optional<WKCipher> createAesEcb(byte[] key) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkCipherCreateForAESECB(key, new SizeT(key.length))
        ).transform(WKCipher::new);
    }

    public static Optional<WKCipher> createChaCha20Poly1305(WKKey key, byte[] nonce12, byte[] ad) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkCipherCreateForChacha20Poly1305(
                        key.getPointer(),
                        nonce12,
                        new SizeT(nonce12.length),
                        ad,
                        new SizeT(ad.length))
        ).transform(WKCipher::new);
    }

    public static Optional<WKCipher> createPigeon(WKKey privKey, WKKey pubKey, byte[] nonce12) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkCipherCreateForPigeon(
                        privKey.getPointer(),
                        pubKey.getPointer(),
                        nonce12,
                        new SizeT(nonce12.length))
        ).transform(WKCipher::new);
    }

    public WKCipher() {
        super();
    }

    public WKCipher(Pointer address) {
        super(address);
    }

    public Optional<byte[]> encrypt(byte[] input) {
        Pointer thisPtr = this.getPointer();

        SizeT length = WKNativeLibraryDirect.wkCipherEncryptLength(thisPtr, input, new SizeT(input.length));
        int lengthAsInt = Ints.checkedCast(length.longValue());
        if (0 == lengthAsInt) return Optional.absent();

        byte[] output = new byte[lengthAsInt];
        int result = WKNativeLibraryDirect.wkCipherEncrypt(thisPtr, output, new SizeT(output.length), input, new SizeT(input.length));
        return result == WKBoolean.WK_TRUE ? Optional.of(output) : Optional.absent();
    }

    public Optional<byte[]> decrypt(byte[] input) {
        Pointer thisPtr = this.getPointer();

        SizeT length = WKNativeLibraryDirect.wkCipherDecryptLength(thisPtr, input, new SizeT(input.length));
        int lengthAsInt = Ints.checkedCast(length.longValue());
        if (0 == lengthAsInt) return Optional.absent();

        byte[] output = new byte[lengthAsInt];
        int result = WKNativeLibraryDirect.wkCipherDecrypt(thisPtr, output, new SizeT(output.length), input, new SizeT(input.length));
        return result == WKBoolean.WK_TRUE ? Optional.of(output) : Optional.absent();
    }

    public Optional<byte[]> migrateBRCoreKeyCiphertext(byte[] input) {
        Pointer thisPtr = this.getPointer();

        int lengthAsInt = input.length;
        if (0 == lengthAsInt) return Optional.absent();

        byte[] output = new byte[lengthAsInt];
        int result = WKNativeLibraryDirect.wkCipherMigrateBRCoreKeyCiphertext(thisPtr, output, new SizeT(output.length), input, new SizeT(input.length));
        return result == WKBoolean.WK_TRUE ? Optional.of(output) : Optional.absent();
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkCipherGive(thisPtr);
    }
}
