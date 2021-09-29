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

import static com.google.common.base.Preconditions.checkState;

public class WKSigner extends PointerType {

    // these must mirror BRCryptoCoderType's enum values
    private static final int CRYPTO_SIGNER_BASIC_DER  = 0;
    private static final int CRYPTO_SIGNER_BASIC_JOSE = 1;
    private static final int CRYPTO_SIGNER_COMPACT    = 2;

    public static Optional<WKSigner> createBasicDer() {
        return create(CRYPTO_SIGNER_BASIC_DER);
    }

    public static Optional<WKSigner> createBasicJose() {
        return create(CRYPTO_SIGNER_BASIC_JOSE);
    }

    public static Optional<WKSigner> createCompact() {
        return create(CRYPTO_SIGNER_COMPACT);
    }

    private static Optional<WKSigner> create(int alg) {
        return Optional.fromNullable(WKNativeLibraryDirect.wkSignerCreate(alg)).transform(WKSigner::new);
    }

    public WKSigner() {
        super();
    }

    public WKSigner(Pointer address) {
        super(address);
    }

    public Optional<byte[]> sign(byte[] digest, WKKey key) {
        checkState(32 == digest.length);
        Pointer thisPtr = this.getPointer();
        Pointer keyPtr = key.getPointer();

        SizeT length = WKNativeLibraryDirect.wkSignerSignLength(thisPtr, keyPtr, digest, new SizeT(digest.length));
        int lengthAsInt = Ints.checkedCast(length.longValue());
        if (0 == lengthAsInt) return Optional.absent();

        byte[] signature = new byte[lengthAsInt];
        int result = WKNativeLibraryDirect.wkSignerSign(thisPtr, keyPtr, signature, new SizeT(signature.length), digest, new SizeT(digest.length));
        return result == WKBoolean.WK_TRUE ? Optional.of(signature) : Optional.absent();
    }

    public Optional<WKKey> recover(byte[] digest, byte[] signature) {
        checkState(32 == digest.length);
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(
                WKNativeLibraryDirect.wkSignerRecover(
                        thisPtr,
                        digest,
                        new SizeT(digest.length),
                        signature,
                        new SizeT(signature.length)
                )
        ).transform(WKKey::new);
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkSignerGive(thisPtr);
    }
}
