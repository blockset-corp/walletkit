package com.breadwallet.core

import brcrypto.*
import brcrypto.BRCryptoWalletEventType.*
import brcrypto.BRCryptoWalletState.CRYPTO_WALLET_STATE_CREATED
import brcrypto.BRCryptoWalletState.CRYPTO_WALLET_STATE_DELETED
import com.breadwallet.core.System.Companion.system
import kotlinx.cinterop.CValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.pointed


internal fun walletEventHandler(
        ctx: BRCryptoCWMListenerContext?,
        cwm: BRCryptoWalletManager?,
        cw: BRCryptoWallet?,
        eventCval: CValue<BRCryptoWalletEvent>
) {
    initRuntimeIfNeeded()
    try {
        memScoped {
            checkNotNull(ctx)
            checkNotNull(cwm)
            checkNotNull(cw)
            defer {
                cryptoWalletGive(cw)
                cryptoWalletManagerGive(cwm)
            }

            val system = ctx.system
            val manager = checkNotNull(system.getWalletManager(cwm))
            val event = eventCval.getPointer(this).pointed

            when (event.type) {
                CRYPTO_WALLET_EVENT_BALANCE_UPDATED -> {
                    val wallet = checkNotNull(manager.getWallet(cw))
                    val balance = Amount(event.u.balanceUpdated.amount!!, false)
                    system.announceWalletEvent(manager, wallet, WalletEvent.BalanceUpdated(balance))
                }
                CRYPTO_WALLET_EVENT_CREATED -> {
                    val wallet = checkNotNull(manager.getWallet(cw))
                    system.announceWalletEvent(manager, wallet, WalletEvent.Created)
                }
                CRYPTO_WALLET_EVENT_CHANGED -> {
                    val wallet = checkNotNull(manager.getWallet(cw))
                    val oldState = when (event.u.state.oldState) {
                        CRYPTO_WALLET_STATE_CREATED -> WalletState.CREATED
                        CRYPTO_WALLET_STATE_DELETED -> WalletState.DELETED
                    }
                    val newState = when (event.u.state.oldState) {
                        CRYPTO_WALLET_STATE_CREATED -> WalletState.CREATED
                        CRYPTO_WALLET_STATE_DELETED -> WalletState.DELETED
                    }
                    system.announceWalletEvent(manager, wallet, WalletEvent.Change(oldState, newState))
                }
                CRYPTO_WALLET_EVENT_DELETED -> {
                    val wallet = checkNotNull(manager.getWallet(cw))
                    system.announceWalletEvent(manager, wallet, WalletEvent.Deleted)
                }
                CRYPTO_WALLET_EVENT_TRANSFER_ADDED -> {
                    val coreTransfer = checkNotNull(event.u.transfer.value)
                    memScoped {
                        defer { cryptoTransferGive(coreTransfer) }
                        val wallet = checkNotNull(manager.getWallet(cw))
                        val transfer = checkNotNull(wallet.getTransfer(coreTransfer))

                        system.announceWalletEvent(manager, wallet, WalletEvent.TransferAdded(transfer))
                    }
                }
                CRYPTO_WALLET_EVENT_TRANSFER_CHANGED -> {
                    val coreTransfer = checkNotNull(event.u.transfer.value)
                    memScoped {
                        defer { cryptoTransferGive(coreTransfer) }
                        val wallet = checkNotNull(manager.getWallet(cw))
                        val transfer = checkNotNull(wallet.getTransfer(coreTransfer))

                        system.announceWalletEvent(manager, wallet, WalletEvent.TransferChanged(transfer))
                    }
                }
                CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED -> {
                    val coreTransfer = checkNotNull(event.u.transfer.value)
                    memScoped {
                        defer { cryptoTransferGive(coreTransfer) }
                        val wallet = checkNotNull(manager.getWallet(cw))
                        val transfer = checkNotNull(wallet.getTransfer(coreTransfer))

                        system.announceWalletEvent(manager, wallet, WalletEvent.TransferSubmitted(transfer))
                    }
                }
                CRYPTO_WALLET_EVENT_TRANSFER_DELETED -> {
                    val coreTransfer = checkNotNull(event.u.transfer.value)
                    memScoped {
                        defer { cryptoTransferGive(coreTransfer) }
                        val wallet = checkNotNull(manager.getWallet(cw))
                        val transfer = checkNotNull(wallet.getTransfer(coreTransfer))

                        system.announceWalletEvent(manager, wallet, WalletEvent.TransferDeleted(transfer))
                    }
                }
                CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED -> {
                    val feeBasis = TransferFeeBasis(checkNotNull(event.u.feeBasisUpdated.basis), false)
                    val wallet = checkNotNull(manager.getWallet(cw))
                    system.announceWalletEvent(manager, wallet, WalletEvent.FeeBasisUpdated(feeBasis))
                }
                CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED -> {
                    /* TODO: handle fee basis estimated callback
                    val feeBasis = TransferFeeBasis(checkNotNull(event.u.feeBasisUpdated.basis), false)
                    val wallet = checkNotNull(manager.getWallet(cw))
                    system.announceWalletEvent(manager, wallet, WalletEvent.FeeBasisEstimated(feeBasis))
                     */
                }
            }
        }
    } catch (e: Exception) {
        println("Error handling wallet event")
        e.printStackTrace()
    }
}
