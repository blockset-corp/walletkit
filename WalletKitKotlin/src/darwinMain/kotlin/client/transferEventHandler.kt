package com.breadwallet.core

import brcrypto.*
import brcrypto.BRCryptoTransferEventType.*
import com.breadwallet.core.System.Companion.system
import kotlinx.cinterop.CValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.pointed


internal fun transferEventHandler(
        ctx: BRCryptoCWMListenerContext?,
        cwm: BRCryptoWalletManager?,
        cw: BRCryptoWallet?,
        ct: BRCryptoTransfer?,
        eventCval: CValue<BRCryptoTransferEvent>
) {
    initRuntimeIfNeeded()
    try {
        memScoped {
            checkNotNull(ctx)
            checkNotNull(cwm)
            checkNotNull(cw)
            checkNotNull(ct)

            val system = ctx.system
            val manager = checkNotNull(system.getWalletManager(cwm))
            val wallet = checkNotNull(manager.getWallet(cw))
            val transfer = checkNotNull(wallet.transferByCoreOrCreate(ct))
            val event = eventCval.getPointer(this).pointed

            when (event.type) {
                CRYPTO_TRANSFER_EVENT_CREATED -> {
                    system.announceTransferEvent(manager, wallet, transfer, TransferEvent.Created)
                }
                CRYPTO_TRANSFER_EVENT_CHANGED -> {
                    val oldState = event.u.state.old.toTransferState()
                    val newState = event.u.state.new.toTransferState()
                    system.announceTransferEvent(manager, wallet, transfer, TransferEvent.Changed(oldState, newState))
                }
                CRYPTO_TRANSFER_EVENT_DELETED -> {
                    system.announceTransferEvent(manager, wallet, transfer, TransferEvent.Deleted)
                }
            }

            defer {
                cryptoTransferGive(ct)
                cryptoWalletGive(cw)
                cryptoWalletManagerGive(cwm)
            }
        }
    } catch (e: Exception) {
        println("Error handling transfer event")
        e.printStackTrace()
    }
}