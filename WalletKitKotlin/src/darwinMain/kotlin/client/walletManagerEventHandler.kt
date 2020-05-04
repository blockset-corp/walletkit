package com.breadwallet.core

import brcrypto.*
import brcrypto.BRCryptoWalletManagerEventType.*
import com.breadwallet.core.System.Companion.system
import kotlinx.cinterop.CValue
import kotlinx.cinterop.memScoped
import kotlinx.cinterop.pointed

internal fun walletManagerEventHandler(
        ctx: BRCryptoCWMListenerContext?,
        cwm: BRCryptoWalletManager?,
        eventCval: CValue<BRCryptoWalletManagerEvent>
) {
    initRuntimeIfNeeded()
    try {
        checkNotNull(ctx)
        checkNotNull(cwm)
        memScoped {
            defer { cryptoWalletManagerGive(cwm) }

            val system = ctx.system
            val event = eventCval.getPointer(this).pointed

            when (event.type) {
                CRYPTO_WALLET_MANAGER_EVENT_CREATED -> {
                    val walletManager = system.createWalletManager(cwm)
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.Created)
                }
                CRYPTO_WALLET_MANAGER_EVENT_CHANGED -> {
                    val oldState = event.u.state.oldValue.asApiState()
                    val newState = event.u.state.newValue.asApiState()

                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    val systemEvent = WalletManagerEvent.Changed(oldState, newState)
                    system.announceWalletManagerEvent(walletManager, systemEvent)
                }
                CRYPTO_WALLET_MANAGER_EVENT_DELETED -> {
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.Deleted)
                }
                CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED -> {
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    val wallet = checkNotNull(walletManager.createWallet(event.u.wallet.value!!))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.WalletAdded(wallet))
                }
                CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED -> {
                    val coreWallet = checkNotNull(event.u.wallet.value)
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.WalletChanged(wallet))
                }
                CRYPTO_WALLET_MANAGER_EVENT_WALLET_DELETED -> {
                    val coreWallet = checkNotNull(event.u.wallet.value)
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.WalletDeleted(wallet))
                }
                CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED -> {
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.SyncStarted)
                }
                CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES -> {
                    val percent = event.u.syncContinues.percentComplete
                    val timestamp = event.u.syncContinues.timestamp.let { if (0u == it) null else it.toLong() }
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.SyncProgress(timestamp, percent))
                }
                CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED -> {
                    val reason = event.u.syncStopped.reason.asApiReason()
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.SyncStopped(reason))
                }
                CRYPTO_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED -> {
                    val depth = WalletManagerSyncDepth.fromSerialization(event.u.syncRecommended.depth.value)
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.SyncRecommended(depth))
                }
                CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED -> {
                    val blockHeight = event.u.blockHeight.value
                    val walletManager = checkNotNull(system.getWalletManager(cwm))
                    system.announceWalletManagerEvent(walletManager, WalletManagerEvent.BlockUpdated(blockHeight))
                }
            }
        }
    } catch (e: Exception) {
        println("Error handling wallet manager event")
        e.printStackTrace()
    }
}