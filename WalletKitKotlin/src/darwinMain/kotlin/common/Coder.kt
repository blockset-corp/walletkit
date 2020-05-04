package com.breadwallet.core.common

import brcrypto.*
import kotlinx.cinterop.addressOf
import kotlinx.cinterop.toCValues
import kotlinx.cinterop.toKString
import kotlinx.cinterop.usePinned
import kotlinx.io.core.Closeable

actual class Coder internal constructor(
        internal val core: BRCryptoCoder
) : Closeable {
    actual fun encode(source: ByteArray): String? {
        val sourceBytes = source.asUByteArray().toCValues()
        val sourceLength = sourceBytes.size.toULong()
        val targetLength = cryptoCoderEncodeLength(core, sourceBytes, sourceLength)
        if (targetLength == 0uL) return null

        val target = ByteArray(targetLength.toInt())

        val result = target.usePinned {
            cryptoCoderEncode(core, it.addressOf(0), targetLength, sourceBytes, sourceLength)
        }
        return if (result == CRYPTO_TRUE) {
            target.toKString()
        } else null
    }

    actual fun decode(source: String): ByteArray? {
        val targetLength = cryptoCoderDecodeLength(core, source)
        if (targetLength == 0uL) return null

        val target = UByteArray(targetLength.toInt())
        val result = target.usePinned {
            cryptoCoderDecode(core, it.addressOf(0), targetLength, source)
        }
        return if (result == CRYPTO_TRUE) {
            target.toByteArray()
        } else null
    }

    actual override fun close() {
        cryptoCoderGive(core)
    }

    actual companion object {
        actual fun createForAlgorithm(algorithm: CoderAlgorithm): Coder =
                when (algorithm) {
                    CoderAlgorithm.HEX -> BRCryptoCoderType.CRYPTO_CODER_HEX
                    CoderAlgorithm.BASE58 -> BRCryptoCoderType.CRYPTO_CODER_BASE58
                    CoderAlgorithm.BASE58CHECK -> BRCryptoCoderType.CRYPTO_CODER_BASE58CHECK
                }.let { Coder(checkNotNull(cryptoCoderCreate(it))) }
    }
}
