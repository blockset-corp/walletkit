package com.breadwallet.core.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable


@Serializable
data class BdbTransactions(
        @SerialName("_embedded")
        val embedded: Embedded = Embedded(emptyList()),
        @SerialName("_links")
        val links: Links? = null
) {

    @Serializable
    data class Embedded(val transactions: List<BdbTransaction>)
}

@Serializable
data class BdbTransaction(
        @SerialName("transaction_id")
        val transactionId: String,
        val identifier: String,
        val hash: String,
        @SerialName("blockchain_id")
        val blockchainId: String,
        val size: Long, // TODO: ULong
        val fee: BdbAmount,
        val status: String,
        @SerialName("_embedded")
        val embedded: Embedded? = null,
        @SerialName("first_seen")
        val firstSeen: String? = null, // TODO: Date
        val timestamp: String? = null, // TODO: Date
        val index: Long? = null, // TODO: ULong
        @SerialName("block_hash")
        val blockHash: String?,
        @SerialName("block_height")
        val blockHeight: Long? = null, // TODO: ULong
        val acknowledgements: Long? = null, // TODO: ULong
        val confirmations: Long? = null, // TODO: ULong
        val raw: String?,
        val proof: String? = null
) {

    @Serializable
    data class Embedded(val transfers: List<BdbTransfer>)

    val transfers: List<BdbTransfer>
        get() = embedded?.transfers ?: emptyList()
}
