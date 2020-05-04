package com.breadwallet.core.common

import brcrypto.*
import kotlinx.cinterop.addressOf
import kotlinx.cinterop.toCValues
import kotlinx.cinterop.usePinned
import kotlinx.io.core.Closeable

actual class Signer internal constructor(
        internal val core: BRCryptoSigner
) : Closeable {
    actual fun sign(digest: ByteArray, key: Key): ByteArray? {
        val privKey = key.core
        val digestBytes = digest.asUByteArray().toCValues()
        val digestLength = digestBytes.size.toULong()
        require(digestLength == 32uL)

        val targetLength = cryptoSignerSignLength(core, privKey, digestBytes, digestLength)
        if (targetLength == 0uL) return null
        val target = UByteArray(targetLength.toInt())

        val result = target.usePinned {
            cryptoSignerSign(core, privKey, it.addressOf(0), targetLength, digestBytes, digestLength)
        }
        return if (result == CRYPTO_TRUE) {
            target.asByteArray()
        } else null
    }

    actual fun recover(digest: ByteArray, signature: ByteArray): Key? {
        val digestBytes = digest.asUByteArray().toCValues()
        val digestLength = digest.size.toULong()
        require(digestBytes.size == 32)

        val signatureBytes = signature.asUByteArray().toCValues()
        val signatureLength = signatureBytes.size.toULong()
        val coreKey = cryptoSignerRecover(core, digestBytes, digestLength, signatureBytes, signatureLength)
        return Key(coreKey ?: return null, false)
    }

    actual override fun close() {
        cryptoSignerGive(core)
    }

    actual companion object {
        actual fun createForAlgorithm(algorithm: SignerAlgorithm): Signer =
                when (algorithm) {
                    SignerAlgorithm.BASIC_DER -> BRCryptoSignerType.CRYPTO_SIGNER_BASIC_DER
                    SignerAlgorithm.BASIC_JOSE -> BRCryptoSignerType.CRYPTO_SIGNER_BASIC_JOSE
                    SignerAlgorithm.COMPACT -> BRCryptoSignerType.CRYPTO_SIGNER_COMPACT
                }.run(::cryptoSignerCreate)
                        .let { coreSigner -> Signer(checkNotNull(coreSigner)) }
    }
}
