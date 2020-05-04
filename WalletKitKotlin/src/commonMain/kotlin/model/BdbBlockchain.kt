package com.breadwallet.core.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class BdbBlockchains(
        @SerialName("_embedded")
        val embedded: Embedded
) {

    @Serializable
    data class Embedded(
            val blockchains: List<BdbBlockchain>
    )
}

private val BLOCK_HEIGHT_UNSPECIFIED = ULong.MAX_VALUE

@Serializable
data class BdbBlockchain(
        val name: String,
        val id: String,
        @SerialName("native_currency_id")
        val currencyId: String,
        @SerialName("fee_estimates")
        val feeEstimates: List<BdbBlockchainFee>,
        val network: String,
        @SerialName("is_mainnet")
        val isMainnet: Boolean,
        @SerialName("confirmations_until_final")
        val confirmationsUntilFinal: Int, // TODO: UInt
        @SerialName("block_height")
        val blockHeight: Long // TODO: ULong
) {
    fun hasBlockHeight() =
            blockHeight.toULong() != BLOCK_HEIGHT_UNSPECIFIED
}
