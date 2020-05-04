package com.breadwallet.core.common

import brcrypto.*
import kotlinx.cinterop.addressOf
import kotlinx.cinterop.toCValues
import kotlinx.cinterop.usePinned
import kotlinx.io.core.Closeable

actual class Hasher internal constructor(
        internal val core: BRCryptoHasher
) : Closeable {
    actual fun hash(data: ByteArray): ByteArray? {
        val dataBytes = data.asUByteArray().toCValues()
        val dataLength = dataBytes.size.toULong()

        val targetLength = cryptoHasherLength(core)
        if (targetLength == 0uL) return null

        val target = UByteArray(targetLength.toInt())
        val result = target.usePinned {
            cryptoHasherHash(core, it.addressOf(0), targetLength, dataBytes, dataLength)
        }
        return if (result == CRYPTO_TRUE) {
            target.toByteArray()
        } else null
    }

    actual override fun close() {
        cryptoHasherGive(core)
    }

    actual companion object {
        actual fun createForAlgorithm(algorithm: HashAlgorithm): Hasher =
                when (algorithm) {
                    HashAlgorithm.SHA1 -> BRCryptoHasherType.CRYPTO_HASHER_SHA1
                    HashAlgorithm.SHA224 -> BRCryptoHasherType.CRYPTO_HASHER_SHA224
                    HashAlgorithm.SHA256 -> BRCryptoHasherType.CRYPTO_HASHER_SHA256
                    HashAlgorithm.SHA256_2 -> BRCryptoHasherType.CRYPTO_HASHER_SHA256_2
                    HashAlgorithm.SHA384 -> BRCryptoHasherType.CRYPTO_HASHER_SHA384
                    HashAlgorithm.SHA512 -> BRCryptoHasherType.CRYPTO_HASHER_SHA512
                    HashAlgorithm.SHA3 -> BRCryptoHasherType.CRYPTO_HASHER_SHA3
                    HashAlgorithm.RMD160 -> BRCryptoHasherType.CRYPTO_HASHER_RMD160
                    HashAlgorithm.HASH160 -> BRCryptoHasherType.CRYPTO_HASHER_HASH160
                    HashAlgorithm.KECCAK256 -> BRCryptoHasherType.CRYPTO_HASHER_KECCAK256
                    HashAlgorithm.MD5 -> BRCryptoHasherType.CRYPTO_HASHER_MD5
                }.run(::cryptoHasherCreate).let { coreHasher ->
                    Hasher(checkNotNull(coreHasher))
                }
    }
}
