package com.breadwallet.core

import brcrypto.BRCryptoSyncStoppedReason
import brcrypto.BRCryptoSyncStoppedReasonType.*
import brcrypto.cryptoSyncStoppedReasonGetMessage
import kotlinx.cinterop.readValue
import kotlinx.cinterop.toKStringFromUtf8

fun BRCryptoSyncStoppedReason.asApiReason(): SyncStoppedReason = when (type) {
    CRYPTO_SYNC_STOPPED_REASON_COMPLETE -> SyncStoppedReason.COMPLETE
    CRYPTO_SYNC_STOPPED_REASON_REQUESTED -> SyncStoppedReason.REQUESTED
    CRYPTO_SYNC_STOPPED_REASON_UNKNOWN -> SyncStoppedReason.UNKNOWN
    CRYPTO_SYNC_STOPPED_REASON_POSIX -> SyncStoppedReason.POSIX(
            u.posix.errnum,
            cryptoSyncStoppedReasonGetMessage(readValue())?.toKStringFromUtf8()
    )
    else -> error("unknown sync stopped reason")
}