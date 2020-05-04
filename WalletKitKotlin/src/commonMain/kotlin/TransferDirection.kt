package com.breadwallet.core

enum class TransferDirection {
    SENT, RECEIVED, RECOVERED;

    override fun toString() = when (this) {
        RECOVERED -> "Recovered"
        SENT -> "Sent"
        RECEIVED -> "Received"
    }
}
