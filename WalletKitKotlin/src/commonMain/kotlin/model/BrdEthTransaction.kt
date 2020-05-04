package com.breadwallet.core.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class BrdEthTransactions(
        val status: String,
        val message: String,
        val result: List<BrdEthTransaction>
)

@Serializable
data class BrdEthTransaction(
        val hash: String,
        @SerialName("from")
        val sourceAddr: String,
        @SerialName("to")
        val targetAddr: String,
        @SerialName("contractAddress")
        val contractAddr: String,
        @SerialName("value")
        val amount: String,
        @SerialName("gas")
        val gasLimit: String,
        val gasPrice: String,
        @SerialName("input")
        val data: String,
        val nonce: String,
        val gasUsed: String,
        val blockNumber: String,
        val blockHash: String,
        val confirmations: String,
        val transactionIndex: String,
        val timeStamp: String,
        val isError: String
)
