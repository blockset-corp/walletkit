package com.breadwallet.core

sealed class NetworkType {
    object BTC : NetworkType()
    object BCH : NetworkType()
    object ETH : NetworkType()
    //object XRP : NetworkType()
    //object HBAR : NetworkType()

    fun toCoreInt(): Int =
            when (this) {
                BTC -> 0
                BCH -> 1
                ETH -> 2
            }

    companion object {
        fun fromCoreInt(coreInt: Int): NetworkType =
                when (coreInt) {
                    0 -> BTC
                    1 -> BCH
                    2 -> ETH
                    else -> error("Unknown core network type ($coreInt)")
                }
    }
}
