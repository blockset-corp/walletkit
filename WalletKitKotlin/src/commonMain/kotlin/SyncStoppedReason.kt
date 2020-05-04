package com.breadwallet.core

sealed class SyncStoppedReason {

    object COMPLETE : SyncStoppedReason()
    object REQUESTED : SyncStoppedReason()
    object UNKNOWN : SyncStoppedReason()

    data class POSIX(
            val errNum: Int,
            val errMessage: String?
    ) : SyncStoppedReason()

    override fun toString() = when (this) {
        REQUESTED -> "Requested"
        UNKNOWN -> "Unknown"
        is POSIX -> "Posix ($errNum: $errMessage)"
        COMPLETE -> "Complete"
    }
}
