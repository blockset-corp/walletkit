package com.breadwallet.core.common

import brcrypto.*
import kotlinx.cinterop.addressOf
import kotlinx.cinterop.toCValues
import kotlinx.cinterop.usePinned
import kotlinx.io.core.Closeable

actual class Cipher internal constructor(
        internal val core: BRCryptoCipher
) : Closeable {

    actual fun encrypt(data: ByteArray): ByteArray? {
        val inputBytes = data.asUByteArray().toCValues()
        val inputLength = inputBytes.size.toULong()

        val outputLength = cryptoCipherEncryptLength(core, inputBytes, inputLength)
        if (outputLength == 0uL) return null
        val output = UByteArray(outputLength.toInt())

        val result = output.usePinned {
            cryptoCipherEncrypt(core, it.addressOf(0), outputLength, inputBytes, inputLength)
        }
        return if (result == CRYPTO_TRUE) {
            output.toByteArray()
        } else null
    }

    actual fun decrypt(data: ByteArray): ByteArray? {
        val inputBytes = data.asUByteArray().toCValues()
        val inputLength = inputBytes.size.toULong()

        val outputLength = cryptoCipherDecryptLength(core, inputBytes, inputLength)
        if (outputLength == 0uL) return null
        val output = UByteArray(outputLength.toInt())

        val result = output.usePinned {
            cryptoCipherDecrypt(core, it.addressOf(0), outputLength, inputBytes, inputLength)
        }
        return if (result == CRYPTO_TRUE) {
            output.toByteArray()
        } else null
    }

    actual override fun close() {
        cryptoCipherGive(core)
    }

    actual companion object {
        actual fun createForAesEcb(key: ByteArray): Cipher {
            val keyBytes = key.asUByteArray().toCValues()
            val keyLength = keyBytes.size.toULong()
            val coreCipher = cryptoCipherCreateForAESECB(keyBytes, keyLength)
            return Cipher(checkNotNull(coreCipher))
        }

        actual fun createForChaCha20Poly1305(key: Key, nonce12: ByteArray, ad: ByteArray): Cipher {
            val nonceBytes = nonce12.asUByteArray().toCValues()
            val nonceLength = nonceBytes.size.toULong()
            val dataBytes = ad.asUByteArray().toCValues()
            val dataLength = ad.size.toULong()
            val coreCipher = cryptoCipherCreateForChacha20Poly1305(key.core, nonceBytes, nonceLength, dataBytes, dataLength)
            return Cipher(checkNotNull(coreCipher))
        }

        actual fun createForPigeon(privKey: Key, pubKey: Key, nonce12: ByteArray): Cipher {
            val nonceBytes = nonce12.asUByteArray().toCValues()
            val nonceLength = nonceBytes.size.toULong()
            val coreCipher = cryptoCipherCreateForPigeon(privKey.core, pubKey.core, nonceBytes, nonceLength)
            return Cipher(checkNotNull(coreCipher))
        }
    }
}
