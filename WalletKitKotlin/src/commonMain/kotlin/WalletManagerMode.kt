package com.breadwallet.core

enum class WalletManagerMode(
        internal val core: UInt
) {
    API_ONLY(0u),
    API_WITH_P2P_SUBMIT(1u),
    P2P_ONLY(2u),
    P2P_WITH_API_SYNC(3u);

    companion object {
        fun fromCoreInt(core: UInt): WalletManagerMode = when (core) {
            0u -> API_ONLY
            1u -> API_WITH_P2P_SUBMIT
            2u -> P2P_WITH_API_SYNC
            3u -> P2P_ONLY
            else -> error("core value ($core) is unknown")
        }
    }
}
