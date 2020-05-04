package com.breadwallet.core.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class BdbTransfer(
        @SerialName("transfer_id")
        val transferId: String,
        @SerialName("blockchain_id")
        val blockchainId: String,
        val index: Long, // TODO: ULong
        val amount: BdbAmount,
        val meta: Map<String, String>,
        @SerialName("from_address")
        val fromAddress: String? = null,
        @SerialName("to_address")
        val toAddress: String? = null,
        @SerialName("transaction_id")
        val transactionId: String? = null,
        val acknowledgements: Long? = null // TODO: ULong
)
