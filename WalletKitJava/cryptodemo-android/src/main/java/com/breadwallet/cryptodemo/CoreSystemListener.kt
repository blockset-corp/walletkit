/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.cryptodemo

import com.breadwallet.crypto.*
import com.breadwallet.crypto.errors.AccountInitializationError
import com.breadwallet.crypto.errors.AccountInitializationMultipleHederaAccountsError
import com.breadwallet.crypto.events.network.NetworkEvent
import com.breadwallet.crypto.events.system.*
import com.breadwallet.crypto.events.transfer.TranferEvent
import com.breadwallet.crypto.events.wallet.WalletCreatedEvent
import com.breadwallet.crypto.events.wallet.WalletEvent
import com.breadwallet.crypto.events.walletmanager.WalletManagerEvent
import com.breadwallet.crypto.utility.CompletionHandler
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers.IO
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.launch
import java.util.logging.Level
import java.util.logging.Logger
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException
import kotlin.coroutines.suspendCoroutine

class CoreSystemListener(
        private val preferredMode: WalletManagerMode,
        private val isMainnet: Boolean,
        private val currencyCodesNeeded: List<String>
) : SystemListener {
    companion object {
        private val Log = Logger.getLogger(CoreSystemListener::class.java.name)
    }

    private val scope = CoroutineScope(SupervisorJob() + IO)

    // SystemListener Handlers
    override fun handleSystemEvent(system: System, event: SystemEvent) {
        scope.launch {
            Log.log(Level.FINE, "System: $event")
            when (event) {
                is SystemNetworkAddedEvent ->
                    createWalletManager(system, event.network)
                is SystemManagerAddedEvent ->
                    connectWalletManager(event.walletManager)
                is SystemDiscoveredNetworksEvent ->
                    logDiscoveredCurrencies(event.networks)
            }
        }
    }

    override fun handleNetworkEvent(system: System, network: Network, event: NetworkEvent) {
        scope.launch {
            Log.log(Level.FINE, "Network: $event")
        }
    }

    override fun handleManagerEvent(system: System, manager: WalletManager, event: WalletManagerEvent) {
        scope.launch {
            Log.log(Level.FINE, "Manager (${manager.name}): $event")
        }
    }

    override fun handleWalletEvent(system: System, manager: WalletManager, wallet: Wallet, event: WalletEvent) {
        scope.launch {
            Log.log(Level.FINE, "Wallet (${manager.name}:${wallet.name}): $event")
            when (event) {
                is WalletCreatedEvent -> logWalletAddresses(wallet)
            }
        }
    }

    override fun handleTransferEvent(
            system: System,
            manager: WalletManager,
            wallet: Wallet,
            transfer: Transfer,
            event: TranferEvent?
    ) {
        scope.launch {
            Log.log(Level.FINE, "Transfer (${manager.name}:${wallet.name}): $event")
        }
    }

    // Misc.
    private suspend fun createWalletManager(system: System, network: Network) {
        val isNetworkNeeded = currencyCodesNeeded.any {
            network.getCurrencyByCode(it).isPresent
        }
        if (isMainnet == network.isMainnet && isNetworkNeeded) {
            val addressScheme = network.defaultAddressScheme
            val mode = when {
                network.supportsWalletManagerMode(preferredMode) -> preferredMode
                else -> network.defaultWalletManagerMode
            }
            Log.log(Level.FINE, "Creating $network WalletManager with $mode and $addressScheme")
            val success = system.createWalletManager(network, mode, addressScheme, emptySet())
            if (!success) {
                val account = system.account
                system.wipe(network)
                if (!system.accountIsInitialized(account, network)) {
                    check(network.type === NetworkType.HBAR)
                    try {
                        system.accountInitialize(account, network, true)
                    } catch (e: AccountInitializationError) {
                        (e as? AccountInitializationMultipleHederaAccountsError)?.run {
                            // TODO: Sort accounts?
                            system.accountInitializeUsingHedera(account, network, accounts.first())
                        }?.orNull()
                    }?.let { accountBytes ->
                        createWalletManager(accountBytes, system, network, mode, addressScheme)
                    }
                }
            }
        }
    }

    private fun createWalletManager(
            serializationData: ByteArray,
            system: System,
            network: Network,
            mode: WalletManagerMode,
            addressScheme: AddressScheme
    ) {
        if (serializationData.isNotEmpty()) {
            val hexCoder = Coder.createForAlgorithm(Coder.Algorithm.HEX)

            // Normally, save the `serializationData`; but not here - DEMO-SPECIFIC
            Log.log(Level.INFO, "Account: SerializationData: ${hexCoder.encode(serializationData)}")
            check(system.createWalletManager(network, mode, addressScheme, emptySet())) {
                "Failed to create wallet manager: $network, $mode, $addressScheme"
            }
        }
    }

    private fun connectWalletManager(walletManager: WalletManager) {
        walletManager.connect(null)
    }

    private fun logWalletAddresses(wallet: Wallet) {
        Log.log(Level.FINE, "Wallet (target) addresses: ${wallet.target}")
    }

    private fun logDiscoveredCurrencies(networks: List<Network>) {
        networks.forEach { network ->
            network.currencies.forEach { currency ->
                Log.log(Level.FINE, "Discovered: ${currency.code} for $network")
            }
        }
    }

    @Throws(AccountInitializationError::class)
    private suspend fun System.accountInitialize(
            account: Account,
            network: Network,
            create: Boolean
    ): ByteArray = suspendCoroutine { continuation ->
        accountInitialize(
                account,
                network,
                create,
                object : CompletionHandler<ByteArray, AccountInitializationError> {
                    override fun handleData(data: ByteArray) {
                        continuation.resume(data)
                    }

                    override fun handleError(error: AccountInitializationError) {
                        continuation.resumeWithException(error)
                    }
                })
    }
}