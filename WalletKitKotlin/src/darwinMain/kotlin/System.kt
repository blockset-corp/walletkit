package com.breadwallet.core

import brcrypto.*
import co.touchlab.stately.collections.IsoMutableList
import co.touchlab.stately.collections.IsoMutableMap
import co.touchlab.stately.collections.IsoMutableSet
import com.breadwallet.core.api.BdbService
import com.breadwallet.core.api.BdbService2
import com.breadwallet.core.common.Key
import com.breadwallet.core.migration.BlockBlob
import com.breadwallet.core.migration.PeerBlob
import com.breadwallet.core.migration.TransactionBlob
import com.breadwallet.core.model.BdbCurrency
import kotlinx.atomicfu.atomic
import kotlinx.cinterop.*
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.flow.toList
import platform.Foundation.NSFileManager
import kotlin.native.concurrent.TransferMode
import kotlin.native.concurrent.Worker
import kotlin.native.concurrent.freeze

actual class System(
        private val callbackCoordinator: SystemCallbackCoordinator,
        val listener: SystemListener,
        actual val account: Account,
        internal actual val isMainnet: Boolean,
        actual val storagePath: String,
        internal actual val query: BdbService,
        private val context: BRCryptoClientContext,
        private val cwmListener: BRCryptoCWMListener,
        private val cwmClient: BRCryptoClient
) {

    // TODO: Remove when BdbService can function in a background worker
    internal val query2 = BdbService2.createForTest(query.http, query.bdbAuthToken!!)

    internal val scope = CoroutineScope(
            SupervisorJob() + Dispatchers.Main + CoroutineExceptionHandler { _, throwable ->
                println("ERROR: ${throwable.message}")
                throwable.printStackTrace()
            })

    private val isNetworkReachable = atomic(true)
    private val _walletManagers = IsoMutableSet<WalletManager>()
    private val _networks = IsoMutableSet<Network>()

    actual val networks: List<Network>
        get() = _networks.toList()

    actual val walletManagers: List<WalletManager>
        get() = _walletManagers.toList()

    actual val wallets: List<Wallet>
        get() = walletManagers.flatMap(WalletManager::wallets)

    init {
        // TODO: announceSystemEvent(SystemEvent.Created)
    }

    actual fun configure(appCurrencies: List<BdbCurrency>) {
        val query = query
        val isMainnet = isMainnet
        val _networks = _networks

        scope.launch {
            val networks = NetworkDiscovery.discoverNetworks(query, isMainnet, appCurrencies)
                    .onEach { network ->
                        if (_networks.add(network)) {
                            announceNetworkEvent(network, NetworkEvent.Created)
                            announceSystemEvent(SystemEvent.NetworkAdded(network))
                        }
                    }
                    .toList()

            announceSystemEvent(SystemEvent.DiscoveredNetworks(networks))
        }
    }

    // Creates an event loop to support coroutines when starting from swift
    fun configureFromSwift(appCurrencies: List<BdbCurrency>) {
        runBlocking { configure(appCurrencies) }
    }

    actual fun createWalletManager(
            network: Network,
            mode: WalletManagerMode,
            addressScheme: AddressScheme,
            currencies: Set<Currency>
    ): Boolean {
        require(network.supportsWalletManagerMode(mode))
        require(network.supportsAddressScheme(addressScheme))

        val walletManager = cryptoWalletManagerCreate(
                cwmListener.readValue(),
                cwmClient.readValue(),
                account.core,
                network.core,
                BRCryptoSyncMode.byValue(mode.core),
                BRCryptoAddressScheme.byValue(addressScheme.core),
                storagePath
        )?.let { cwm ->
            WalletManager(cwm, this, callbackCoordinator, false)
        } ?: return false

        currencies
                .filter(network::hasCurrency)
                .forEach(walletManager::registerWalletFor)

        walletManager.setNetworkReachable(isNetworkReachable.value)
        _walletManagers.add(walletManager)
        announceSystemEvent(SystemEvent.ManagerAdded(walletManager))
        return true
    }

    actual fun wipe(network: Network) {
        if (walletManagers.none { it.network == network }) {
            cryptoWalletManagerWipe(network.core, storagePath)
        }
    }

    actual fun connectAll() {
        walletManagers.forEach { manager ->
            manager.connect(null)
        }
    }

    actual fun disconnectAll() {
        walletManagers.forEach(WalletManager::disconnect)
    }

    actual fun subscribe(subscriptionToken: String) {
        TODO("Not implemented")
    }

    actual fun updateNetworkFees(completion: CompletionHandler<List<Network>, NetworkFeeUpdateError>?) {
        scope.launch {
            val blockchains = try {
                query.getBlockchains(isMainnet).embedded.blockchains
            } catch (e: Exception) {
                completion?.invoke(null, NetworkFeeUpdateError.FeesUnavailable)
                return@launch
            }

            val networksByUuid = hashMapOf<String, Network>()
            _networks.forEach { network ->
                networksByUuid[network.uids] = network
            }

            val networks = blockchains.mapNotNull { blockchain ->
                networksByUuid[blockchain.id]?.also { network ->
                    // We always have a feeUnit for network
                    val feeUnit = checkNotNull(network.baseUnitFor(network.currency))

                    network.fees = blockchain.feeEstimates.mapNotNull { bdbFee ->
                        Amount.create(bdbFee.fee.value, feeUnit, false)?.let { amount ->
                            NetworkFee(bdbFee.confirmationTimeInMilliseconds.toULong(), amount)
                        }
                    }

                    announceNetworkEvent(network, NetworkEvent.FeesUpdated)
                }
            }

            completion?.invoke(networks, null)
        }
    }

    actual fun setNetworkReachable(isNetworkReachable: Boolean) {
        this.isNetworkReachable.value = isNetworkReachable
        walletManagers.forEach { manager ->
            manager.setNetworkReachable(isNetworkReachable)
        }
    }

    actual fun migrateRequired(network: Network): Boolean {
        return network.requiresMigration()
    }

    actual fun migrateStorage(network: Network, transactionBlobs: List<TransactionBlob>, blockBlobs: List<BlockBlob>, peerBlobs: List<PeerBlob>) {
        TODO("not implemented")
    }

    private val listenerWorker = Worker.start().freeze()
    private fun announceSystemEvent(event: SystemEvent) {
        listenerWorker.execute(TransferMode.SAFE, {
            Triple(listener, context, event).freeze()
        }) { (listener, ctx, event) ->
            listener.handleSystemEvent(ctx.system, event)
        }
    }

    private fun announceNetworkEvent(network: Network, event: NetworkEvent) {
        listenerWorker.execute(TransferMode.SAFE, {
            Triple(listener, context to network, event).freeze()
        }) { (listener, ctxAndNetwork, event) ->
            val (ctx, network) = ctxAndNetwork
            listener.handleNetworkEvent(ctx.system, network, event)
        }
    }

    internal fun announceWalletManagerEvent(walletManager: WalletManager, event: WalletManagerEvent) {
        listenerWorker.execute(TransferMode.SAFE, {
            Triple(listener, this to walletManager, event).freeze()
        }) { (listener, systemAndManager, event) ->
            val (system, manager) = systemAndManager
            listener.handleManagerEvent(system, manager, event)
        }
    }

    internal fun announceWalletEvent(walletManager: WalletManager, wallet: Wallet, event: WalletEvent) {
        listenerWorker.execute(TransferMode.SAFE, {
            Triple(listener, this to walletManager, wallet to event).freeze()
        }) { (listener, systemAndManager, walletAndEvent) ->
            val (system, manager) = systemAndManager
            val (wallet, event) = walletAndEvent
            listener.handleWalletEvent(system, manager, wallet, event)
        }
    }

    internal fun announceTransferEvent(
            walletManager: WalletManager,
            wallet: Wallet,
            transfer: Transfer,
            event: TransferEvent
    ) {
        listenerWorker.execute(TransferMode.SAFE, {
            Triple(listener to transfer, this to walletManager, wallet to event).freeze()
        }) { (listenerAndTransfer, systemAndManager, walletAndEvent) ->
            val (listener, transfer) = listenerAndTransfer
            val (system, manager) = systemAndManager
            val (wallet, event) = walletAndEvent
            listener.handleTransferEvent(system, manager, wallet, transfer, event)
        }
    }

    internal fun createWalletManager(coreWalletManager: BRCryptoWalletManager): WalletManager =
            WalletManager(coreWalletManager, this, callbackCoordinator, true)
                    .also { walletManager ->
                        _walletManagers.add(walletManager)
                    }

    internal fun getWalletManager(coreWalletManager: BRCryptoWalletManager): WalletManager? {
        val walletManager = WalletManager(coreWalletManager, this, callbackCoordinator, true)
        // TODO: return if (_walletManagers.contains(walletManager)) walletManager else null
        return walletManager
    }

    actual companion object {

        private val SYSTEM_IDS = atomic(0)

        private const val SYSTEMS_INACTIVE_RETAIN = true

        private val SYSTEMS_ACTIVE = IsoMutableMap<BRCookie, System>()
        private val SYSTEMS_INACTIVE = IsoMutableList<System>()

        val BRCryptoClientContext.system
            get() = checkNotNull(SYSTEMS_ACTIVE[this])

        actual fun create(
                executor: ScheduledExecutorService,
                listener: SystemListener,
                account: Account,
                isMainnet: Boolean,
                storagePath: String,
                query: BdbService
        ): System? {
            val accountStoragePath = "${storagePath.trimEnd('/')}/${account.filesystemIdentifier}"
            check(ensurePath(accountStoragePath)) {
                "Failed to validate storage path."
            }

            val context = nativeHeap.alloc<IntVar> {
                value = SYSTEM_IDS.incrementAndGet()
            }.ptr

            val system = System(
                    "", // TODO: SystemCallbackCoordinator
                    listener,
                    account,
                    isMainnet,
                    accountStoragePath,
                    query,
                    context,
                    createCryptoListener(context),
                    createCryptoClient(context)
            )

            SYSTEMS_ACTIVE[context] = system

            return system
        }

        actual fun asBdbCurrency(
                uids: String,
                name: String,
                code: String,
                type: String,
                decimals: UInt
        ): BdbCurrency? {
            val index = uids.indexOf(':')
            if (index == -1) return null

            val typeLowerCase = type.toLowerCase()
            if ("erc20" != type && "native" != type) return null

            val codeLowerCase = code.toLowerCase()
            val blockchainId = uids.substring(0, index)
            val address = uids.substring(index)

            // TODO(fix): What should the supply values be here?
            return BdbCurrency(
                    currencyId = uids,
                    name = name,
                    code = codeLowerCase,
                    type = typeLowerCase,
                    blockchainId = blockchainId,
                    address = if (address == "__native__") null else address,
                    verified = true,
                    denominations = Blockchains.makeCurrencyDenominationsErc20(codeLowerCase, decimals),
                    initialSupply = "0",
                    totalSupply = "0"
            )
        }

        actual fun migrateBRCoreKeyCiphertext(
                key: Key,
                nonce12: ByteArray,
                authenticatedData: ByteArray,
                ciphertext: ByteArray
        ): ByteArray? {
            TODO("not implemented")
        }

        actual fun wipe(system: System) {
            //TODO("not implemented")
        }

        actual fun wipeAll(storagePath: String, exemptSystems: List<System>) {
            //TODO("not implemented")
        }

        private fun ensurePath(path: String): Boolean {
            try {
                NSFileManager.defaultManager.createDirectoryAtPath(path, true, null, null)
            } catch (e: Exception) {
                println("File creation error")
                e.printStackTrace()
                return false
            }

            return NSFileManager.defaultManager.isWritableFileAtPath(path)
        }
    }
}
