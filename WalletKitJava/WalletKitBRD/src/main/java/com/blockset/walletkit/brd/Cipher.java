/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKCipher;
import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

/* package */
final class Cipher implements com.blockset.walletkit.Cipher {

    /* package */
    static Optional<byte[]> migrateBRCoreKeyCiphertext(com.blockset.walletkit.Key key, byte[] nonce12, byte[] authenticatedData,
                                                       byte[] ciphertext) {
        return Cipher.createForChaCha20Poly1305(key, nonce12, authenticatedData).migrateBRCoreKeyCiphertext(ciphertext);
    }

    /* package */
    static Cipher createForAesEcb(byte[] key) {
        WKCipher cipher = WKCipher.createAesEcb(key).orNull();
        checkNotNull(cipher);
        return Cipher.create(cipher);
    }

    /* package */
    static Cipher createForChaCha20Poly1305(com.blockset.walletkit.Key key, byte[] nonce12, byte[] authenticatedData) {
        WKCipher cipher = WKCipher.createChaCha20Poly1305(
                Key.from(key).getBRCryptoKey(),
                nonce12,
                authenticatedData)
                .orNull();
        checkNotNull(cipher);
        return Cipher.create(cipher);
    }

    /* package */
    static Cipher createForPigeon(com.blockset.walletkit.Key privKey,
                                  com.blockset.walletkit.Key pubKey,
                                  byte[] nonce12) {
        WKCipher cipher = WKCipher.createPigeon(
                Key.from(privKey).getBRCryptoKey(),
                Key.from(pubKey).getBRCryptoKey(),
                nonce12)
                .orNull();
        checkNotNull(cipher);
        return Cipher.create(cipher);
    }

    private static Cipher create(WKCipher core) {
        Cipher cipher = new Cipher(core);
        ReferenceCleaner.register(cipher, core::give);
        return cipher;
    }

    private final WKCipher core;

    private Cipher(WKCipher core) {
        this.core = core;
    }

    @Override
    public Optional<byte[]> encrypt(byte[] data) {
        return core.encrypt(data);
    }

    @Override
    public Optional<byte[]> decrypt(byte[] data) {
        return core.decrypt(data);
    }

    private Optional<byte[]> migrateBRCoreKeyCiphertext(byte[] data) {
        return core.migrateBRCoreKeyCiphertext(data);
    }
}
