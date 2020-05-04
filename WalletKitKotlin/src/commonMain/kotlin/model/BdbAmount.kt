package com.breadwallet.core.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class BdbAmount(
        @SerialName("currency_id")
        val currencyId: String,
        @SerialName("amount")
        val value: String
)
