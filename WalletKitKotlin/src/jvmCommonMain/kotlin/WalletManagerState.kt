package com.breadwallet.core

import com.breadwallet.corenative.crypto.BRCryptoWalletManagerState
import com.breadwallet.corenative.crypto.BRCryptoWalletManagerStateType


fun BRCryptoWalletManagerState.asApiState(): WalletManagerState =
        when (type()) {
            BRCryptoWalletManagerStateType.CRYPTO_WALLET_MANAGER_STATE_CREATED -> WalletManagerState.CREATED
            BRCryptoWalletManagerStateType.CRYPTO_WALLET_MANAGER_STATE_DISCONNECTED ->
                WalletManagerState.DISCONNECTED(u.disconnected.reason.asApiReason())
            BRCryptoWalletManagerStateType.CRYPTO_WALLET_MANAGER_STATE_CONNECTED -> WalletManagerState.CONNECTED
            BRCryptoWalletManagerStateType.CRYPTO_WALLET_MANAGER_STATE_SYNCING -> WalletManagerState.SYNCING
            BRCryptoWalletManagerStateType.CRYPTO_WALLET_MANAGER_STATE_DELETED -> WalletManagerState.DELETED
            else -> error("Unknown wallet manager state")
        }
