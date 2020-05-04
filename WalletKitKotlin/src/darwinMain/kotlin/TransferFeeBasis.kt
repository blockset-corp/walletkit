package com.breadwallet.core

import brcrypto.*

actual class TransferFeeBasis internal constructor(
        core: BRCryptoFeeBasis,
        take: Boolean
) {

    internal val core: BRCryptoFeeBasis =
            if (take) checkNotNull(cryptoFeeBasisTake(core))
            else core

    actual val unit: CUnit
        get() = CUnit(checkNotNull(cryptoFeeBasisGetPricePerCostFactorUnit(core)), false)

    actual val currency: Currency
        get() = unit.currency

    actual val pricePerCostFactor: Amount
        get() = Amount(checkNotNull(cryptoFeeBasisGetPricePerCostFactor(core)), false)

    actual val costFactor: Double
        get() = cryptoFeeBasisGetCostFactor(core)

    actual val fee: Amount
        get() = Amount(checkNotNull(cryptoFeeBasisGetFee(core)), false)

    actual override fun equals(other: Any?): Boolean =
            other is TransferFeeBasis && CRYPTO_TRUE == cryptoFeeBasisIsIdentical(core, other.core)

    actual override fun hashCode(): Int {
        return unit.hashCode() + currency.hashCode() + fee.hashCode() +
                pricePerCostFactor.hashCode() + costFactor.hashCode()
    }
}
