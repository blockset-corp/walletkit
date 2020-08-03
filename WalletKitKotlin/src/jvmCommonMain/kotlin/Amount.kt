package com.breadwallet.core

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoAmount
import com.breadwallet.corenative.crypto.BRCryptoComparison
import kotlinx.io.core.Closeable
import java.math.RoundingMode
import java.text.DecimalFormat
import java.text.DecimalFormatSymbols

actual class Amount internal constructor(
        internal val core: BRCryptoAmount
) : Comparable<Amount>, Closeable {
    actual companion object {
        actual fun create(double: Double, unit: CUnit): Amount =
                Amount(BRCryptoAmount.create(double, unit.core))

        actual fun create(long: Long, unit: CUnit): Amount =
                Amount(BRCryptoAmount.create(long, unit.core))

        actual fun create(string: String, unit: CUnit, isNegative: Boolean): Amount? =
                BRCryptoAmount.create(string, isNegative, unit.core).orNull()?.run(::Amount)
    }

    init {
        ReferenceCleaner.register(core, ::close)
    }

    actual val unit: CUnit
        get() = CUnit(core.unit)
    actual val currency: Currency
        get() = Currency(core.currency)
    actual val isNegative: Boolean
        get() = core.isNegative
    actual val negate: Amount
        get() = Amount(core.negate())
    actual val isZero: Boolean
        get() = core.isZero

    actual fun asDouble(unit: CUnit): Double? =
            core.getDouble(unit.core).orNull()

    actual fun asString(unit: CUnit): String? {
        val amountDouble = asDouble(unit) ?: return null
        return formatterWithUnit(unit).format(amountDouble)
    }

    actual fun asString(pair: CurrencyPair): String? =
            pair.exchangeAsBase(this)?.asString(pair.quoteUnit)

    actual fun asString(base: Int, preface: String): String? =
            core.toStringWithBase(base, preface)

    actual operator fun plus(that: Amount): Amount =
            Amount(checkNotNull(core.add(that.core).orNull()))

    actual operator fun minus(that: Amount): Amount =
            Amount(checkNotNull(core.sub(that.core).orNull()))

    actual fun convert(unit: CUnit): Amount? =
            core.convert(unit.core).orNull()?.run(::Amount)

    actual fun isCompatible(amount: Amount): Boolean =
            core.isCompatible(amount.core)

    actual fun hasCurrency(currency: Currency): Boolean =
            core.hasCurrency(currency.core)

    actual override fun equals(other: Any?): Boolean =
            other is Amount && core.compare(other.core) == BRCryptoComparison.CRYPTO_COMPARE_EQ

    actual override fun toString(): String =
            asString(unit) ?: "<nan>"

    override fun close() {
        core.give()
    }

    actual override fun hashCode(): Int = core.hashCode()

    actual override operator fun compareTo(other: Amount): Int =
            when (checkNotNull(core.compare(other.core))) {
                BRCryptoComparison.CRYPTO_COMPARE_EQ -> 0
                BRCryptoComparison.CRYPTO_COMPARE_GT -> 1
                BRCryptoComparison.CRYPTO_COMPARE_LT -> -1
                else -> error("Failed crypto compare")
            }

    private fun formatterWithUnit(unit: CUnit): DecimalFormat =
            (DecimalFormat.getCurrencyInstance().clone() as DecimalFormat).apply {
                val decimals: Int = unit.decimals.toInt()
                maximumFractionDigits = decimals
                isParseBigDecimal = 0 != decimals
                maximumIntegerDigits = Int.MAX_VALUE
                roundingMode = RoundingMode.HALF_EVEN

                decimalFormatSymbols = (decimalFormatSymbols.clone() as DecimalFormatSymbols).apply {
                    unit.symbol
                            .also(::setInternationalCurrencySymbol)
                            .also(::setCurrencySymbol)
                }
            }
}
