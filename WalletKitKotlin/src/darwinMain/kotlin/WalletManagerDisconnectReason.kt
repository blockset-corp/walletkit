package com.breadwallet.core

import brcrypto.BRCryptoWalletManagerDisconnectReason
import brcrypto.BRCryptoWalletManagerDisconnectReasonType
import brcrypto.BRCryptoWalletManagerDisconnectReasonType.*
import brcrypto.cryptoWalletManagerDisconnectReasonGetMessage
import com.breadwallet.core.WalletManagerDisconnectReason
import kotlinx.cinterop.ptr
import kotlinx.cinterop.toKStringFromUtf8


fun BRCryptoWalletManagerDisconnectReason.asApiReason(): WalletManagerDisconnectReason =
        when (type) {
            CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_REQUESTED ->
                WalletManagerDisconnectReason.REQUESTED
            CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_UNKNOWN ->
                WalletManagerDisconnectReason.UNKNOWN
            CRYPTO_WALLET_MANAGER_DISCONNECT_REASON_POSIX ->
                WalletManagerDisconnectReason.POSIX(
                        u.posix.errnum,
                        cryptoWalletManagerDisconnectReasonGetMessage(ptr)?.toKStringFromUtf8()
                )
            else -> error("unknown disconnect reason")
        }