package com.breadwallet.core

import brcrypto.*
import kotlinx.cinterop.toKStringFromUtf8
import kotlinx.io.core.Closeable

actual class TransferHash(
        core: BRCryptoHash,
        take: Boolean
) : Closeable {

    internal val core: BRCryptoHash =
            if (take) checkNotNull(cryptoHashTake(core))
            else core

    actual override fun equals(other: Any?): Boolean =
            other is TransferHash && CRYPTO_TRUE == cryptoHashEqual(core, other.core)

    actual override fun hashCode(): Int =
            toString().hashCode()

    actual override fun toString(): String =
            checkNotNull(cryptoHashString(core)).toKStringFromUtf8()

    actual override fun close() {
        cryptoHashGive(core)
    }
}
