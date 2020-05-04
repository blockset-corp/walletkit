package com.breadwallet.core

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoCurrency
import kotlinx.io.core.Closeable

actual class Currency internal constructor(
        internal val core: BRCryptoCurrency
) : Closeable {

    init {
        ReferenceCleaner.register(core, ::close)
    }

    /** 'A Unique Identifier */
    actual val uids: String
        get() = core.uids

    /** The code; e.g. BTC */
    actual val code: String
        get() = core.code

    /** The name; e.g. Bitcoin */
    actual val name: String
        get() = core.name

    /** The type: */
    actual val type: String
        get() = core.type

    /** The issuer, if present.  This is generally an ERC20 address. */
    actual val issuer: String?
        get() = core.issuer

    actual override fun equals(other: Any?): Boolean =
            other is Currency && core.isIdentical(other.core)

    actual override fun hashCode(): Int = core.hashCode()

    override fun close() {
        core.give()
    }

    actual companion object {
        actual fun create(
                uids: String,
                name: String,
                code: String,
                type: String,
                issuer: String?
        ): Currency = Currency(
                BRCryptoCurrency.create(uids, name, code, type, issuer)
        )
    }
}
