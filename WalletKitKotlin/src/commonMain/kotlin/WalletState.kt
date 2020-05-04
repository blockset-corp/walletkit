package com.breadwallet.core

sealed class WalletState {
    object CREATED : WalletState()
    object DELETED : WalletState()

    override fun toString() = when (this) {
        CREATED -> "Created"
        DELETED -> "Deleted"
    }
}
