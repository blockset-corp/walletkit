package com.breadwallet.core

sealed class WalletManagerDisconnectReason {

    object REQUESTED : WalletManagerDisconnectReason()
    object UNKNOWN : WalletManagerDisconnectReason()

    data class POSIX(
            val errNum: Int,
            val errMessage: String?
    ) : WalletManagerDisconnectReason()

    override fun toString() = when (this) {
        REQUESTED -> "Requested"
        UNKNOWN -> "Unknown"
        is POSIX -> "Posix ($errNum: $errMessage)"
    }
}
