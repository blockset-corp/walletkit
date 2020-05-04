package com.breadwallet.core

import brcrypto.*
import kotlinx.cinterop.toKStringFromUtf8
import kotlinx.io.core.Closeable

actual class Address(
        core: BRCryptoAddress,
        take: Boolean
) : Closeable {

    internal val core: BRCryptoAddress =
            if (take) checkNotNull(cryptoAddressTake(core))
            else core

    actual override fun equals(other: Any?): Boolean =
            other is Address && CRYPTO_TRUE == cryptoAddressIsIdentical(core, other.core)

    actual override fun hashCode(): Int = toString().hashCode()

    actual override fun toString(): String =
            checkNotNull(cryptoAddressAsString(core)).toKStringFromUtf8()

    actual override fun close() {
        cryptoAddressGive(core)
    }

    actual companion object {
        actual fun create(string: String, network: Network): Address? {
            val core = cryptoAddressCreateFromString(network.core, string)
            return if (core != null) {
                Address(core, false)
            } else null
        }
    }
}
