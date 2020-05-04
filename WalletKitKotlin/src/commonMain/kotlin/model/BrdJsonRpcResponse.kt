package com.breadwallet.core.model

import kotlinx.serialization.Serializable

@Serializable
data class BrdJsonRpcResponse(
        val jsonrpc: String,
        val id: Int,
        val message: String? = null,
        val status: String? = null,
        val result: String
)
