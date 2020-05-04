package com.breadwallet.core.common

import brcrypto.*
import com.breadwallet.core.Secret
import kotlinx.cinterop.*
import kotlinx.io.core.Closeable

actual class Key internal constructor(
        core: BRCryptoKey,
        take: Boolean
) : Closeable {

    internal val core: BRCryptoKey =
            if (take) checkNotNull(cryptoKeyTake(core))
            else core

    internal actual constructor(
            secret: Secret
    ) : this(
            checkNotNull(cryptoKeyCreateFromSecret(secret.readValue())),
            false
    )

    actual val hasSecret: Boolean
        get() = CRYPTO_TRUE == cryptoKeyHasSecret(core).toUInt()

    // TODO: Clean this up
    actual val encodeAsPrivate: ByteArray
        get() = memScoped {
            checkNotNull(cryptoKeyEncodePrivate(core)).let { coreBytes ->
                var count = 0
                while (true) {
                    if (coreBytes[count] != 0.toByte()) {
                        count++
                    } else break
                }
                ByteArray(count) { i -> coreBytes[i] }
            }
        }

    actual val encodeAsPublic: ByteArray
        get() = memScoped {
            checkNotNull(cryptoKeyEncodePublic(core)).let { coreBytes ->
                var count = 0
                while (true) {
                    if (coreBytes[count] != 0.toByte()) {
                        count++
                    } else break
                }
                ByteArray(count) { i -> coreBytes[i] }
            }
        }

    actual val secret: Secret
        get() = memScoped {
            cryptoKeyGetSecret(core).getPointer(this).pointed
        }

    actual fun publicKeyMatch(that: Key): Boolean =
            CRYPTO_TRUE == cryptoKeyPublicMatch(core, that.core).toUInt()

    internal actual fun privateKeyMatch(that: Key): Boolean =
            CRYPTO_TRUE == cryptoKeySecretMatch(core, that.core).toUInt()

    override fun close() {
        cryptoKeyGive(core)
    }

    actual companion object {
        actual var wordList: List<String>? = null

        actual fun isProtectedPrivateKey(privateKey: String): Boolean =
                CRYPTO_TRUE == cryptoKeyIsProtectedPrivate(privateKey)

        actual fun createFromPhrase(
                phrase: String,
                words: List<String>?
        ): Key? = memScoped {
            words ?: return null
            val wordsArray = words.toCStringArray(this)
            val coreKey = cryptoKeyCreateFromPhraseWithWords(phrase, wordsArray) ?: return null
            Key(coreKey, false)
        }

        actual fun createFromProtectedPrivateKey(privateKey: String, passphrase: String): Key? =
                cryptoKeyCreateFromStringProtectedPrivate(privateKey, passphrase)
                        ?.let { coreKey -> Key(coreKey, false) }

        actual fun createFromPrivateKey(privateKey: String): Key? =
                cryptoKeyCreateFromStringPrivate(privateKey)
                        ?.let { coreKey -> Key(coreKey, false) }

        actual fun createFromPublicKey(string: String): Key? =
                cryptoKeyCreateFromStringPublic(string)
                        ?.let { coreKey -> Key(coreKey, false) }

        actual fun createForPigeonFromKey(key: Key, nonce: ByteArray): Key? {
            val nonceValue = nonce.asUByteArray().toCValues()
            val coreKey = cryptoKeyCreateForPigeon(key.core, nonceValue, nonce.size.toULong())
            return Key(coreKey ?: return null, false)
        }

        actual fun createForBIP32ApiAuth(phrase: String, words: List<String>?): Key? = memScoped {
            words ?: return null
            val wordsArray = words.toCStringArray(this)
            val coreKey = cryptoKeyCreateForBIP32ApiAuth(phrase, wordsArray) ?: return null
            Key(coreKey, false)
        }

        actual fun createForBIP32BitID(
                phrase: String,
                index: Int,
                uri: String,
                words: List<String>?
        ): Key? = memScoped {
            words ?: return null
            val wordsArray = words.toCStringArray(this)
            val coreKey = cryptoKeyCreateForBIP32BitID(phrase, index, uri, wordsArray) ?: return null
            Key(coreKey, false)
        }
    }
}
