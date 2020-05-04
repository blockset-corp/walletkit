package com.breadwallet.core

import brcrypto.BRCryptoWalletManagerState
import brcrypto.BRCryptoWalletManagerStateType.*


fun BRCryptoWalletManagerState.asApiState(): WalletManagerState =
        when (type) {
            CRYPTO_WALLET_MANAGER_STATE_CREATED -> WalletManagerState.CREATED
            CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED ->
                WalletManagerState.DISCONNECTED(u.disconnected.reason.asApiReason())
            CRYPTO_WALLET_MANAGER_STATE_CONNECTED -> WalletManagerState.CONNECTED
            CRYPTO_WALLET_MANAGER_STATE_SYNCING -> WalletManagerState.SYNCING
            CRYPTO_WALLET_MANAGER_STATE_DELETED -> WalletManagerState.DELETED
            else -> error("Unknown wallet manager state")
        }