package com.breadwallet.core.common

import kotlinx.io.core.Closeable

expect class Cipher : Closeable {

    public fun encrypt(data: ByteArray): ByteArray?
    public fun decrypt(data: ByteArray): ByteArray?

    override fun close()

    companion object {
        public fun createForAesEcb(key: ByteArray): Cipher

        public fun createForChaCha20Poly1305(key: Key, nonce12: ByteArray, ad: ByteArray): Cipher

        public fun createForPigeon(privKey: Key, pubKey: Key, nonce12: ByteArray): Cipher
    }
}
