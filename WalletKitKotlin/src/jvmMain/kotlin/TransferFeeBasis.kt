package com.breadwallet.core

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoFeeBasis

actual class TransferFeeBasis internal constructor(
        internal val core: BRCryptoFeeBasis
) {

    init {
        ReferenceCleaner.register(core, core::give)
    }

    actual val unit: CUnit
        get() = CUnit(core.pricePerCostFactorUnit)

    actual val currency: Currency
        get() = unit.currency

    actual val pricePerCostFactor: Amount
        get() = Amount(core.pricePerCostFactor)

    actual val costFactor: Double
        get() = core.costFactor

    actual val fee: Amount
        get() = Amount(checkNotNull(core.fee.orNull()))

    actual override fun equals(other: Any?): Boolean =
            other is TransferFeeBasis && core.isIdentical(other.core)

    actual override fun hashCode(): Int {
        return core.hashCode()
    }
}
