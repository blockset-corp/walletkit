package com.breadwallet.core

import com.breadwallet.core.model.BdbCurrency

internal object Blockchains {

    const val ADDRESS_BRD_MAINNET = "0x558ec3152e2eb2174905cd19aea4e34a23de9ad6"
    const val ADDRESS_BRD_TESTNET = "0x7108ca7c4718efa810457f228305c9c71390931a"

    fun makeCurrencyDenominationsErc20(
            code: String,
            decimals: UInt
    ): List<BdbCurrency.Denomination> {
        val name = code.toUpperCase()
        val codeLowerCase = code.toLowerCase()
        return listOf(
                BdbCurrency.Denomination(
                        name = "$name Token INT",
                        code = "${codeLowerCase}i",
                        decimals = 0,
                        symbol = "${codeLowerCase}i"
                ),
                BdbCurrency.Denomination(
                        name = "$name Token",
                        code = codeLowerCase,
                        decimals = decimals.toInt(),
                        symbol = codeLowerCase
                )
        )
    }
}
