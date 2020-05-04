package com.breadwallet.core.common

import com.breadwallet.core.Secret
import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoKey
import kotlinx.io.core.Closeable

actual class Key internal constructor(
        internal val core: BRCryptoKey
) : Closeable {

    init {
        ReferenceCleaner.register(core, ::close)
    }

    internal actual constructor(
            secret: Secret
    ) : this(
            checkNotNull(
                    BRCryptoKey.cryptoKeyCreateFromSecret(secret.u8).orNull()
            )
    )

    actual val hasSecret: Boolean
        get() = core.hasSecret()

    actual val encodeAsPrivate: ByteArray
        get() = checkNotNull(core.encodeAsPrivate())

    actual val encodeAsPublic: ByteArray
        get() = checkNotNull(core.encodeAsPublic())

    actual val secret: Secret
        get() = Secret(core.secret)

    actual fun publicKeyMatch(that: Key): Boolean =
            core.publicKeyMatch(that.core)

    internal actual fun privateKeyMatch(that: Key): Boolean =
            core.privateKeyMatch(that.core)

    override fun close() {
        core.give()
    }

    actual companion object {
        actual var wordList: List<String>? = null
            @Synchronized get
            @Synchronized set

        actual fun isProtectedPrivateKey(privateKey: String): Boolean =
                BRCryptoKey.isProtectedPrivateKeyString(privateKey.toByteArray())

        actual fun createFromPhrase(phrase: String, words: List<String>?): Key? =
                if (words == null && wordList == null) null
                else BRCryptoKey.createFromPhrase(phrase.toByteArray(), words)
                        .orNull()
                        ?.run(::Key)

        actual fun createFromProtectedPrivateKey(privateKey: String, passphrase: String): Key? =
                BRCryptoKey.createFromPrivateKeyString(privateKey.toByteArray(), passphrase.toByteArray())
                        .orNull()
                        ?.run(::Key)

        actual fun createFromPrivateKey(privateKey: String): Key? =
                BRCryptoKey.createFromPrivateKeyString(privateKey.toByteArray())
                        .orNull()
                        ?.run(::Key)

        actual fun createFromPublicKey(string: String): Key? =
                BRCryptoKey.createFromPublicKeyString(string.toByteArray())
                        .orNull()
                        ?.run(::Key)

        actual fun createForPigeonFromKey(key: Key, nonce: ByteArray): Key? =
                BRCryptoKey.createForPigeon(key.core, nonce).orNull()?.run(::Key)

        actual fun createForBIP32ApiAuth(phrase: String, words: List<String>?): Key? =
                if (words == null && wordList == null) null
                else BRCryptoKey.createForBIP32ApiAuth(phrase.toByteArray(), words)
                        .orNull()
                        ?.run(::Key)

        actual fun createForBIP32BitID(phrase: String, index: Int, uri: String, words: List<String>?): Key? =
                if (words == null && wordList == null) null
                else BRCryptoKey.createForBIP32BitID(phrase.toByteArray(), index, uri, words)
                        .orNull()
                        ?.run(::Key)
    }
}
