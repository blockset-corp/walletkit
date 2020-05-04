package com.breadwallet.core.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class BdbCurrencies(
        @SerialName("_embedded")
        val embedded: Embedded
) {

    @Serializable
    data class Embedded(
            val currencies: List<BdbCurrency>
    )
}

@Serializable
data class BdbCurrency(
        @SerialName("currency_id")
        val currencyId: String,
        val name: String,
        val code: String,
        @SerialName("initial_supply")
        val initialSupply: String,
        @SerialName("total_supply")
        val totalSupply: String,
        @SerialName("blockchain_id")
        val blockchainId: String,
        val address: String? = null,
        val type: String,
        val denominations: List<Denomination>,
        val verified: Boolean
) {

    @Serializable
    data class Denomination(
            val name: String,
            @SerialName("short_name")
            val code: String,
            val decimals: Int, // TODO: UInt
            val symbol: String? = null // TODO: Use lookupSymbol for default value
    ) {

        fun getSymbolSafe() = symbol ?: lookupSymbol(name)

        companion object {
            private val CURRENCY_SYMBOLS = mapOf(
                    "btc" to "₿",
                    "eth" to "Ξ"
            )

            private fun lookupSymbol(code: String): String {
                val symbol = CURRENCY_SYMBOLS[code]
                return symbol ?: code
            }
        }
    }
}
