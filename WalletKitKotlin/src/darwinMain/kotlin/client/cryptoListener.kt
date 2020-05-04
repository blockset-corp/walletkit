package com.breadwallet.core

import brcrypto.*
import brcrypto.BRCryptoTransferStateType.*
import kotlinx.cinterop.*

internal fun createCryptoListener(
        c: BRCryptoCWMListenerContext
) = nativeHeap.alloc<BRCryptoCWMListener> {
    context = c
    walletEventCallback = staticCFunction(::walletEventHandler)
    transferEventCallback = staticCFunction(::transferEventHandler)
    walletManagerEventCallback = staticCFunction(::walletManagerEventHandler)

    // TODO: This is required or we get a K/N memory related crash when accessed
    runCatching {
        nativeHeap.alloc<BRCryptoWalletManagerEvent>().readValue()
    }
}

fun BRCryptoTransferState.toTransferState(): TransferState = when (type) {
    CRYPTO_TRANSFER_STATE_SUBMITTED -> TransferState.SUBMITTED
    CRYPTO_TRANSFER_STATE_CREATED -> TransferState.CREATED
    CRYPTO_TRANSFER_STATE_SIGNED -> TransferState.SIGNED
    CRYPTO_TRANSFER_STATE_DELETED -> TransferState.DELETED
    CRYPTO_TRANSFER_STATE_ERRORED -> TransferState.FAILED(
            TransferSubmitError.UNKNOWN // TODO: process error
    )
    CRYPTO_TRANSFER_STATE_INCLUDED -> TransferState.INCLUDED(
            TransferConfirmation(
                    u.included.blockNumber,
                    u.included.transactionIndex,
                    u.included.timestamp,
                    u.included.feeBasis?.let { feeBasis ->
                        TransferFeeBasis(feeBasis, false)
                    }?.fee
            )
    )
}
