package com.breadwallet.core

interface SystemListener : WalletManagerListener, WalletListener, TransferListener, NetworkListener {
    fun handleSystemEvent(system: System, event: SystemEvent) = Unit
}

interface WalletManagerListener {
    fun handleManagerEvent(system: System, manager: WalletManager, event: WalletManagerEvent) = Unit
}

interface WalletListener {
    fun handleWalletEvent(system: System, manager: WalletManager, wallet: Wallet, event: WalletEvent) = Unit
}


interface TransferListener {
    fun handleTransferEvent(system: System, manager: WalletManager, wallet: Wallet, transfer: Transfer, event: TransferEvent) = Unit
}


interface NetworkListener {
    fun handleNetworkEvent(system: System, network: Network, event: NetworkEvent) = Unit
}
