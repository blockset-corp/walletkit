package com.breadwallet.core

import com.breadwallet.core.System.Companion.system
import com.breadwallet.corenative.crypto.BRCryptoCWMListener
import com.breadwallet.corenative.crypto.BRCryptoWalletManagerEventType.*
import com.google.common.primitives.UnsignedLong
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch

private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Default)

internal val WalletManagerEventCallback =
        BRCryptoCWMListener.WalletManagerEventCallback { context, coreWalletManager, event ->
            scope.launch {
                // Log.log(Level.FINE, "WalletManagerEventCallback")

                val system = context.system
                if (system == null) {
                    //Log.log(Level.SEVERE, "WalletManagerChanged: missed system");
                    coreWalletManager.give()
                    return@launch
                }
                try {
                    when (event.type()) {
                        CRYPTO_WALLET_MANAGER_EVENT_CREATED -> {
                            // Log.log(Level.FINE, "WalletManagerCreated");
                            val walletManager = system.createWalletManager(coreWalletManager)
                            system.announceWalletManagerEvent(walletManager, WalletManagerEvent.Created)
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_CHANGED -> {
                            val oldState = event.u.state.oldValue.asApiState()
                            val newState = event.u.state.newValue.asApiState()

                            // Log.log(Level.FINE, String.format("WalletManagerChanged (%s -> %s)", oldState, newState));

                            val walletManager = system.getWalletManager(coreWalletManager)

                            if (walletManager != null) {
                                val systemEvent = WalletManagerEvent.Changed(oldState, newState)
                                system.announceWalletManagerEvent(walletManager, systemEvent)
                            } else {
                                // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_DELETED -> {
                            // Log.log(Level.FINE, "WalletManagerDeleted");

                            val walletManager = system.getWalletManager(coreWalletManager)
                            if (walletManager != null) {
                                system.announceWalletManagerEvent(walletManager, WalletManagerEvent.Deleted)
                            } else {
                                // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED -> {
                            val coreWallet = event.u.wallet.value
                            try {
                                // Log.log(Level.FINE, "WalletManagerWalletAdded");
                                val walletManager = system.getWalletManager(coreWalletManager)
                                if (walletManager != null) {
                                    val wallet = walletManager.getWallet(coreWallet)
                                    if (wallet != null) {
                                        system.announceWalletManagerEvent(walletManager, WalletManagerEvent.WalletAdded(wallet))
                                    } else {
                                        // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet");
                                    }
                                } else {
                                    // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                                }
                            } finally {
                                coreWallet.give()
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED -> {
                            val coreWallet = event.u.wallet.value
                            try {
                                // Log.log(Level.FINE, "WalletManagerWalletChanged");
                                val walletManager = system.getWalletManager(coreWalletManager)
                                if (walletManager != null) {
                                    val wallet = walletManager.getWallet(coreWallet)
                                    if (wallet != null) {
                                        system.announceWalletManagerEvent(walletManager, WalletManagerEvent.WalletChanged(wallet))
                                    } else {
                                        // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet");
                                    }
                                } else {
                                    // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                                }
                            } finally {
                                coreWallet.give()
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_WALLET_DELETED -> {
                            val coreWallet = event.u.wallet.value
                            try {
                                // Log.log(Level.FINE, "WalletManagerWalletDeleted");
                                val walletManager = system.getWalletManager(coreWalletManager)
                                if (walletManager != null) {
                                    val wallet = walletManager.getWallet(coreWallet)
                                    if (wallet != null) {
                                        system.announceWalletManagerEvent(walletManager, WalletManagerEvent.WalletDeleted(wallet))
                                    } else {
                                        // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet");
                                    }
                                } else {
                                    // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                                }
                            } finally {
                                coreWallet.give()
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED -> {
                            // Log.log(Level.FINE, "WalletManagerSyncStarted");

                            val walletManager = system.getWalletManager(coreWalletManager)
                            if (walletManager != null) {
                                system.announceWalletManagerEvent(walletManager, WalletManagerEvent.SyncStarted)
                            } else {
                                // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES -> {
                            val percent = event.u.syncContinues.percentComplete
                            val timestamp = event.u.syncContinues.timestamp.let { if (0 == it) null else it.toLong() }

                            // Log.log(Level.FINE, String.format("WalletManagerSyncProgress (%s)", percent));

                            val walletManager = system.getWalletManager(coreWalletManager)
                            if (walletManager != null) {
                                system.announceWalletManagerEvent(walletManager, WalletManagerEvent.SyncProgress(timestamp, percent))
                            } else {
                                // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED -> {
                            val reason = event.u.syncStopped.reason.asApiReason()

                            // Log.log(Level.FINE, String.format("WalletManagerSyncStopped: (%s)", reason));

                            val walletManager = system.getWalletManager(coreWalletManager)
                            if (walletManager != null) {
                                system.announceWalletManagerEvent(walletManager, WalletManagerEvent.SyncStopped(reason))
                            } else {
                                // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED -> {
                            val coreDepth = event.u.syncRecommended.depth().toCore().toUInt()
                            val depth = WalletManagerSyncDepth.fromSerialization(coreDepth)

                            // Log.log(Level.FINE, String.format("WalletManagerSyncRecommended: (%s)", depth));

                            val walletManager = system.getWalletManager(coreWalletManager)
                            if (walletManager != null) {
                                system.announceWalletManagerEvent(walletManager, WalletManagerEvent.SyncRecommended(depth))
                            } else {
                                // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                            }
                        }
                        CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED -> {
                            val blockHeight = UnsignedLong.fromLongBits(event.u.blockHeight.value).toLong().toULong()

                            // Log.log(Level.FINE, String.format("WalletManagerBlockHeightUpdated (%s)", blockHeight));

                            val walletManager = system.getWalletManager(coreWalletManager)
                            if (walletManager != null) {
                                system.announceWalletManagerEvent(walletManager, WalletManagerEvent.BlockUpdated(blockHeight))
                            } else {
                                // Log.log(Level.SEVERE, "WalletManagerChanged: missed wallet manager");
                            }
                        }
                    }
                } finally {
                    coreWalletManager.give()
                }
            }
        }
