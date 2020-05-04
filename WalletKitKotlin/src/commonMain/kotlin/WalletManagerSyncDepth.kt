package com.breadwallet.core

enum class WalletManagerSyncDepth {
    FROM_LAST_CONFIRMED_SEND,
    FROM_LAST_TRUSTED_BLOCK,
    FROM_CREATION;

    companion object {
        fun fromSerialization(serialization: UInt): WalletManagerSyncDepth {
            return when (serialization) {
                0xa0u -> FROM_LAST_CONFIRMED_SEND
                0xb0u -> FROM_LAST_TRUSTED_BLOCK
                0xc0u -> FROM_CREATION
                else -> error("Unknown serialization value: $serialization")
            }
        }
    }

    fun toSerialization(): UInt {
        return when (this) {
            FROM_LAST_CONFIRMED_SEND -> 0xa0u
            FROM_LAST_TRUSTED_BLOCK -> 0xb0u
            FROM_CREATION -> 0xc0u
        }
    }

    fun getShallowerValue(): WalletManagerSyncDepth? {
        return when (this) {
            FROM_CREATION -> FROM_LAST_TRUSTED_BLOCK
            FROM_LAST_TRUSTED_BLOCK -> FROM_LAST_CONFIRMED_SEND
            else -> null
        }
    }

    fun getDeeperValue(): WalletManagerSyncDepth? {
        return when (this) {
            FROM_LAST_CONFIRMED_SEND -> FROM_LAST_TRUSTED_BLOCK
            FROM_LAST_TRUSTED_BLOCK -> FROM_CREATION
            else -> null
        }
    }
}
