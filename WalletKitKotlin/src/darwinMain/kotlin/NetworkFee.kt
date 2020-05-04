package com.breadwallet.core

import brcrypto.*
import kotlinx.io.core.Closeable

actual class NetworkFee(
        core: BRCryptoNetworkFee,
        take: Boolean
) : Closeable {

    internal actual constructor(
            timeIntervalInMilliseconds: ULong,
            pricePerCostFactor: Amount
    ) : this(
            checkNotNull(cryptoNetworkFeeCreate(
                    timeIntervalInMilliseconds,
                    pricePerCostFactor.core,
                    pricePerCostFactor.unit.core
            )),
            false
    )

    internal val core: BRCryptoNetworkFee =
            if (take) checkNotNull(cryptoNetworkFeeTake(core))
            else core

    actual val timeIntervalInMilliseconds: ULong =
            cryptoNetworkFeeGetConfirmationTimeInMilliseconds(core)
    internal actual val pricePerCostFactor: Amount =
            Amount(checkNotNull(cryptoNetworkFeeGetPricePerCostFactor(core)), false)

    actual override fun hashCode(): Int = core.hashCode()
    actual override fun equals(other: Any?): Boolean =
            other is NetworkFee && CRYPTO_TRUE == cryptoNetworkFeeEqual(core, other.core)

    actual override fun close() {
        cryptoNetworkFeeGive(core)
    }
}
