package com.breadwallet.core

import com.breadwallet.core.SyncStoppedReason.*
import com.breadwallet.corenative.crypto.BRCryptoSyncStoppedReason
import com.breadwallet.corenative.crypto.BRCryptoSyncStoppedReasonType
import com.breadwallet.corenative.crypto.BRCryptoSyncStoppedReasonType.*


fun BRCryptoSyncStoppedReason.asApiReason(): SyncStoppedReason =
        when (type()) {
            CRYPTO_SYNC_STOPPED_REASON_COMPLETE -> COMPLETE
            CRYPTO_SYNC_STOPPED_REASON_REQUESTED -> REQUESTED
            CRYPTO_SYNC_STOPPED_REASON_UNKNOWN -> UNKNOWN
            CRYPTO_SYNC_STOPPED_REASON_POSIX -> POSIX(u.posix.errnum, message.orNull())
            else -> error("unknown sync stopped reason")
        }
