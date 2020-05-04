package com.breadwallet.core

import brcrypto.*
import kotlinx.cinterop.toKStringFromUtf8
import kotlinx.io.core.Closeable

actual class Currency internal constructor(
        core: BRCryptoCurrency,
        take: Boolean
) : Closeable {

    internal val core: BRCryptoCurrency = if (take) {
        checkNotNull(cryptoCurrencyTake(core))
    } else core

    actual val uids: String
        get() = checkNotNull(cryptoCurrencyGetUids(core)).toKStringFromUtf8()
    actual val code: String
        get() = checkNotNull(cryptoCurrencyGetCode(core)).toKStringFromUtf8()
    actual val name: String
        get() = checkNotNull(cryptoCurrencyGetName(core)).toKStringFromUtf8()
    actual val type: String
        get() = checkNotNull(cryptoCurrencyGetType(core)).toKStringFromUtf8()
    actual val issuer: String?
        get() = cryptoCurrencyGetIssuer(core)?.toKStringFromUtf8()

    actual override fun equals(other: Any?): Boolean {
        return other is Currency && CRYPTO_TRUE == cryptoCurrencyIsIdentical(core, other.core)
    }

    actual override fun hashCode(): Int = core.hashCode()

    override fun close() {
        cryptoCurrencyGive(core)
    }

    actual companion object {
        actual fun create(
                uids: String,
                name: String,
                code: String,
                type: String,
                issuer: String?
        ) = Currency(
                core = checkNotNull(cryptoCurrencyCreate(
                        uids, name, code, type, issuer
                )),
                take = false
        )
    }

}
