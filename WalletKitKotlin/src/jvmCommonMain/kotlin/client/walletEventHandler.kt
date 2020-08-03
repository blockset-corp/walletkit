package com.breadwallet.core

import com.breadwallet.core.System.Companion.system
import com.breadwallet.corenative.crypto.BRCryptoCWMListener
import com.breadwallet.corenative.crypto.BRCryptoWalletEventType.*
import com.breadwallet.corenative.crypto.BRCryptoWalletState
import com.breadwallet.corenative.crypto.BRCryptoWalletState.CRYPTO_WALLET_STATE_CREATED
import com.breadwallet.corenative.crypto.BRCryptoWalletState.CRYPTO_WALLET_STATE_DELETED
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch

private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Default)

internal val WalletEventCallback =
        BRCryptoCWMListener.WalletEventCallback { context, coreWalletManager, coreWallet, event ->
            scope.launch {
                fun give() {
                    coreWallet.give()
                    coreWalletManager.give()
                }

                try {
                    val system = checkNotNull(context.system)

                    when (checkNotNull(event.type())) {
                        CRYPTO_WALLET_EVENT_CREATED -> {
                            val walletManager = checkNotNull(system.getWalletManager(coreWalletManager))
                            val wallet = walletManager.createWallet(coreWallet)
                            system.announceWalletEvent(walletManager, wallet, WalletEvent.Created)
                        }
                        CRYPTO_WALLET_EVENT_CHANGED -> {
                            val oldState = when (checkNotNull(event.u.state.oldState())) {
                                CRYPTO_WALLET_STATE_CREATED -> WalletState.CREATED
                                CRYPTO_WALLET_STATE_DELETED -> WalletState.DELETED
                            }
                            val newState = when (checkNotNull(event.u.state.oldState())) {
                                CRYPTO_WALLET_STATE_CREATED -> WalletState.CREATED
                                CRYPTO_WALLET_STATE_DELETED -> WalletState.DELETED
                            }
                            val walletManager = checkNotNull(system.getWalletManager(coreWalletManager))
                            val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                            system.announceWalletEvent(walletManager, wallet, WalletEvent.Change(oldState, newState))
                        }
                        CRYPTO_WALLET_EVENT_DELETED -> {
                            val walletManager = checkNotNull(system.getWalletManager(coreWalletManager))
                            val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                            system.announceWalletEvent(walletManager, wallet, WalletEvent.Deleted)
                        }
                        CRYPTO_WALLET_EVENT_TRANSFER_ADDED -> {
                            val coreTransfer = event.u.transfer.value
                            try {
                                val walletManager = checkNotNull(system.getWalletManager(coreWalletManager))
                                val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                                val transfer = checkNotNull(wallet.getTransfer(coreTransfer))
                                system.announceWalletEvent(walletManager, wallet, WalletEvent.TransferAdded(transfer))
                            } finally {
                                coreTransfer.give()
                            }
                        }
                        CRYPTO_WALLET_EVENT_TRANSFER_CHANGED -> {
                            val coreTransfer = event.u.transfer.value
                            try {
                                val walletManager = checkNotNull(system.getWalletManager(coreWalletManager))
                                val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                                val transfer = checkNotNull(wallet.getTransfer(coreTransfer))
                                system.announceWalletEvent(walletManager, wallet, WalletEvent.TransferChanged(transfer))
                            } finally {
                                coreTransfer.give()
                            }
                        }
                        CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED -> {
                            val coreTransfer = event.u.transfer.value
                            try {
                                val walletManager = checkNotNull(system.getWalletManager(coreWalletManager))
                                val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                                val transfer = checkNotNull(wallet.getTransfer(coreTransfer))
                                system.announceWalletEvent(walletManager, wallet, WalletEvent.TransferSubmitted(transfer))
                            } finally {
                                coreTransfer.give()
                            }
                        }
                        CRYPTO_WALLET_EVENT_TRANSFER_DELETED -> {
                            val coreTransfer = event.u.transfer.value
                            try {
                                val walletManager = checkNotNull(system.getWalletManager(coreWalletManager))
                                val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                                val transfer = checkNotNull(wallet.getTransfer(coreTransfer))
                                system.announceWalletEvent(walletManager, wallet, WalletEvent.TransferDeleted(transfer))
                            } finally {
                                coreTransfer.give()
                            }
                        }
                        CRYPTO_WALLET_EVENT_BALANCE_UPDATED -> {
                            val amount = Amount(event.u.balanceUpdated.amount)
                            val walletManager = checkNotNull(system.getWalletManager(coreWalletManager))
                            val wallet = checkNotNull(walletManager.getWallet(coreWallet))
                            system.announceWalletEvent(walletManager, wallet, WalletEvent.BalanceUpdated(amount))
                        }
                        CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED -> {
                        }
                        CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED -> {
                        }
                    }
                } finally {
                    give()
                }
            }
        }
