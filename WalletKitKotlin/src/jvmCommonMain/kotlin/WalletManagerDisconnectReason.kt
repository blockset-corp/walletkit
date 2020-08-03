package com.breadwallet.core

import com.breadwallet.core.WalletManagerDisconnectReason.*
import com.breadwallet.corenative.crypto.BRCryptoWalletManagerDisconnectReason
import com.breadwallet.corenative.crypto.BRCryptoWalletManagerDisconnectReasonType.*


fun BRCryptoWalletManagerDisconnectReason.asApiReason(): WalletManagerDisconnectReason =
        when (type()) {
            CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_REQUESTED -> REQUESTED
            CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_UNKNOWN -> UNKNOWN
            CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_POSIX ->
                POSIX(u.posix.errnum, message.orNull())
            else -> error("unknown disconnect reason")
        }
