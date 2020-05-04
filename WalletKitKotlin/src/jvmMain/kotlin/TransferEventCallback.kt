package com.breadwallet.core

import com.breadwallet.corenative.crypto.BRCryptoCWMListener
import com.breadwallet.corenative.crypto.BRCryptoTransferEventType

val TransferEventCallback =
        BRCryptoCWMListener.TransferEventCallback { context, coreWalletManager, coreWallet, coreTransfer, event ->
            when (event.type()) {
                BRCryptoTransferEventType.CRYPTO_TRANSFER_EVENT_CREATED -> {
                }
                BRCryptoTransferEventType.CRYPTO_TRANSFER_EVENT_CHANGED -> {
                }
                BRCryptoTransferEventType.CRYPTO_TRANSFER_EVENT_DELETED -> {
                }
            }

            coreTransfer.give()
            coreWallet.give()
            coreWalletManager.give()
        }
