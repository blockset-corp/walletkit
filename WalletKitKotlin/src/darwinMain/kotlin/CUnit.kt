package com.breadwallet.core

import brcrypto.*
import kotlinx.cinterop.toKStringFromUtf8
import kotlinx.io.core.Closeable

actual class CUnit internal constructor(
        core: BRCryptoUnit,
        take: Boolean
) : Closeable {

    internal val core: BRCryptoUnit = if (take) {
        checkNotNull(cryptoUnitTake(core))
    } else core

    actual val currency: Currency
        get() = Currency(checkNotNull(cryptoUnitGetCurrency(core)), false)
    internal actual val uids: String
        get() = checkNotNull(cryptoUnitGetUids(core)).toKStringFromUtf8()
    actual val name: String
        get() = checkNotNull(cryptoUnitGetName(core)).toKStringFromUtf8()
    actual val symbol: String
        get() = checkNotNull(cryptoUnitGetSymbol(core)).toKStringFromUtf8()
    actual val base: CUnit
        get() = CUnit(checkNotNull(cryptoUnitGetBaseUnit(core)), false)
    actual val decimals: UInt
        get() = cryptoUnitGetBaseDecimalOffset(core).toUInt()

    actual fun isCompatible(unit: CUnit): Boolean {
        return CRYPTO_TRUE == cryptoUnitIsCompatible(core, unit.core)
    }

    actual fun hasCurrency(currency: Currency): Boolean {
        return CRYPTO_TRUE == cryptoUnitHasCurrency(core, currency.core)
    }

    actual override fun equals(other: Any?): Boolean {
        return other is CUnit && CRYPTO_TRUE == cryptoUnitIsIdentical(core, other.core)
    }

    actual override fun hashCode(): Int = uids.hashCode()

    override fun close() {
        cryptoUnitGive(core)
    }

    actual companion object {
        internal actual fun create(
                currency: Currency,
                uids: String,
                name: String,
                symbol: String
        ) = CUnit(
                core = checkNotNull(
                        cryptoUnitCreateAsBase(
                                currency.core,
                                uids,
                                name,
                                symbol
                        )
                ),
                take = false
        )

        internal actual fun create(
                currency: Currency,
                uids: String,
                name: String,
                symbol: String,
                base: CUnit,
                decimals: UInt
        ) = CUnit(
                core = checkNotNull(
                        cryptoUnitCreate(
                                currency.core,
                                uids,
                                name,
                                symbol,
                                base.core,
                                decimals.toUByte()
                        )
                ),
                take = false
        )
    }
}
