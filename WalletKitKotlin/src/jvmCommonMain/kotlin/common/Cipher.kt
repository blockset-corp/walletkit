package com.breadwallet.core.common

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoCipher
import kotlinx.io.core.Closeable

actual class Cipher internal constructor(
        internal val core: BRCryptoCipher
) : Closeable {

    init {
        ReferenceCleaner.register(core, ::close)
    }

    actual fun encrypt(data: ByteArray): ByteArray? =
            core.encrypt(data).orNull()

    actual fun decrypt(data: ByteArray): ByteArray? =
            core.decrypt(data).orNull()

    actual override fun close() {
        core.give()
    }

    actual companion object {
        actual fun createForAesEcb(key: ByteArray): Cipher =
                BRCryptoCipher.createAesEcb(key)
                        .orNull().let { coreCipher -> Cipher(checkNotNull(coreCipher)) }

        actual fun createForChaCha20Poly1305(key: Key, nonce12: ByteArray, ad: ByteArray): Cipher =
                BRCryptoCipher.createChaCha20Poly1305(key.core, nonce12, ad)
                        .orNull().let { coreCipher -> Cipher(checkNotNull(coreCipher)) }

        actual fun createForPigeon(privKey: Key, pubKey: Key, nonce12: ByteArray): Cipher =
                BRCryptoCipher.createPigeon(privKey.core, pubKey.core, nonce12)
                        .orNull().let { coreCipher -> Cipher(checkNotNull(coreCipher)) }
    }
}
