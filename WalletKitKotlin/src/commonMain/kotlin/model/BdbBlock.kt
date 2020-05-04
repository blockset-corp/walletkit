package com.breadwallet.core.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class BdbBlock(
        @SerialName("block_id")
        val blockId: String,
        val hash: String,
        @SerialName("blockchain_id")
        val blockchainId: String,
        val height: Long, // TODO: Ulong
        val mined: Long, // TODO: ULong
        @SerialName("transaction_ids")
        val transactionIds: List<String>,
        val size: Long, // TODO: ULong
        @SerialName("total_fees")
        val totalFees: BdbAmount,
        val acknowledgements: Long, // TODO: ULong
        @SerialName("is_active_chain")
        val isActiveChain: Boolean,
        @SerialName("_embedded")
        val embedded: Embedded? = null,
        @SerialName("prevHash")
        val prevHash: String? = null,
        @SerialName("nextHash")
        val nextHash: String? = null,
        val header: String? = null,
        val raw: String? = null
) {

    @Serializable
    data class Embedded(
            val transactions: List<BdbTransaction>
    )
}
