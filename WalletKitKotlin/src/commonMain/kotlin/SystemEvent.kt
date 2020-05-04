package com.breadwallet.core

sealed class SystemEvent {

    object Created : SystemEvent()

    data class DiscoveredNetworks(
            val networks: List<Network>
    ) : SystemEvent()

    data class ManagerAdded(
            val manager: WalletManager
    ) : SystemEvent()

    data class NetworkAdded(
            val network: Network
    ) : SystemEvent()
}
