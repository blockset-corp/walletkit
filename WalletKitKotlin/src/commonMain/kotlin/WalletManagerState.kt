package com.breadwallet.core

sealed class WalletManagerState {

    object CREATED : WalletManagerState()
    data class DISCONNECTED(
            val reason: WalletManagerDisconnectReason
    ) : WalletManagerState()

    object CONNECTED : WalletManagerState()
    object SYNCING : WalletManagerState()
    object DELETED : WalletManagerState()

    override fun toString() = when (this) {
        CREATED -> "Created"
        is DISCONNECTED -> "Disconnected ($reason)"
        CONNECTED -> "Connected"
        SYNCING -> "Syncing"
        DELETED -> "Deleted"
    }
}
