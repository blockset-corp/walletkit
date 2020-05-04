package com.breadwallet.core

import com.breadwallet.core.api.BdbService
import com.breadwallet.core.common.Cipher
import com.breadwallet.core.common.Key
import com.breadwallet.core.migration.BlockBlob
import com.breadwallet.core.migration.PeerBlob
import com.breadwallet.core.migration.TransactionBlob
import com.breadwallet.core.model.BdbCurrency
import com.breadwallet.corenative.crypto.*
import com.breadwallet.corenative.utility.Cookie
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.onEach
import kotlinx.coroutines.flow.toList
import java.io.File
import java.util.*
import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.atomic.AtomicInteger
import kotlin.collections.ArrayList
import kotlin.collections.set

actual class System internal constructor(
        private val callbackCoordinator: SystemCallbackCoordinator,
        private val listener: SystemListener,
        actual val account: Account,
        internal actual val isMainnet: Boolean,
        actual val storagePath: String,
        internal actual val query: BdbService,
        private val context: Cookie,
        private val cwmListener: BRCryptoCWMListener,
        private val cwmClient: BRCryptoClient
) {

    internal val scope = CoroutineScope(
            SupervisorJob() + Dispatchers.Default + CoroutineExceptionHandler { _, throwable ->
                throwable.printStackTrace(java.lang.System.err)
            })

    private var isNetworkReachable = true
    private val _walletManagers = hashSetOf<WalletManager>()
    private val _networks = hashSetOf<Network>()

    actual val networks: List<Network>
        get() = _networks.toList()

    actual val walletManagers: List<WalletManager>
        get() = _walletManagers.toList()

    actual val wallets: List<Wallet>
        get() = walletManagers.flatMap(WalletManager::wallets)

    init {
        announceSystemEvent(SystemEvent.Created)
    }

    actual fun configure(appCurrencies: List<BdbCurrency>) {
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

    actual fun createWalletManager(
            network: Network,
            mode: WalletManagerMode,
            addressScheme: AddressScheme,
            currencies: Set<Currency>
    ): Boolean {
        require(network.supportsWalletManagerMode(mode))
        require(network.supportsAddressScheme(addressScheme))

        val walletManager = BRCryptoWalletManager.create(
                cwmListener,
                cwmClient,
                account.core,
                network.core,
                BRCryptoSyncMode.fromCore(mode.core.toInt()),
                BRCryptoAddressScheme.fromCore(addressScheme.core.toInt()),
                storagePath
        ).orNull()?.let {
            WalletManager(it, this, callbackCoordinator)
        } ?: return false

        currencies
                .filter(network::hasCurrency)
                .forEach { currency ->
                    walletManager.registerWalletFor(currency)
                }

        walletManager.setNetworkReachable(isNetworkReachable)
        _walletManagers.add(walletManager)
        announceSystemEvent(SystemEvent.ManagerAdded(walletManager))
        return true
    }

    actual fun wipe(network: Network) {
        val hasManager = walletManagers.any { it.network == network }

        if (!hasManager) {
            BRCryptoWalletManager.wipe(network.core, storagePath)
        }
    }

    actual fun connectAll() {
        walletManagers.forEach { manager ->
            manager.connect(null)
        }
    }

    actual fun disconnectAll() {
        walletManagers.forEach { manager ->
            manager.disconnect()
        }
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

            val networks2 = ArrayList<Network>()
            blockchains.forEach { blockchain ->
                val network = networksByUuid[blockchain.id] ?: return@forEach

                // We always have a feeUnit for network
                val feeUnit = checkNotNull(network.baseUnitFor(network.currency))

                network.fees = blockchain.feeEstimates
                        .mapNotNull {
                            Amount.create(it.fee.value, feeUnit, false)?.let { amount ->
                                NetworkFee(it.confirmationTimeInMilliseconds.toULong(), amount)
                            }
                        }

                announceNetworkEvent(network, NetworkEvent.FeesUpdated)
                networks2.add(network)
            }

            completion?.invoke(networks, null)
        }
    }

    actual fun setNetworkReachable(isNetworkReachable: Boolean) {
        this.isNetworkReachable = isNetworkReachable
        walletManagers.forEach { manager ->
            manager.setNetworkReachable(isNetworkReachable)
        }
    }

    actual fun migrateRequired(network: Network): Boolean {
        return network.requiresMigration()
    }

    actual fun migrateStorage(
            network: Network,
            transactionBlobs: List<TransactionBlob>,
            blockBlobs: List<BlockBlob>,
            peerBlobs: List<PeerBlob>
    ) {
        if (!migrateRequired(network)) {
            throw MigrateError.Invalid
        }

        migrateStorageAsBtc(network, transactionBlobs, blockBlobs, peerBlobs)
    }

    private fun migrateStorageAsBtc(
            network: Network,
            transactionBlobs: List<TransactionBlob>,
            blockBlobs: List<BlockBlob>,
            peerBlobs: List<PeerBlob>
    ) {
        // TODO: Implement WalletMigrator
        /*val migrator = WalletMigrator.create(network, storagePath) ?: throw MigrateError.Create

        transactionBlobs.forEach { blob ->
          val btc = blob.btc ?: throw MigrateError.Transaction
          if (!migrator.handleTransactionAsBtc(
                  btc.bytes,
                  btc.blockHeight,
                  btc.timestamp)) {
            throw MigrateError.Transaction
          }
        }
        blockBlobs.forEach { blob ->
          val btc = blob.btc ?: throw MigrateError.Block
          if (!migrator.handleBlockAsBtc(btc.block, btc.height)) {
            throw MigrateError.Block
          }
        }
        peerBlobs.forEach { blob ->
          val btc = blob.btc ?: throw MigrateError.Peer
          // On a `nil` timestamp, by definition skip out, don't migrate this blob
          if (btc.timestamp != null) {
            if (!migrator.handlePeerAsBtc(
                    btc.address,
                    btc.port,
                    btc.services,
                    btc.timestamp)) {
              throw MigrateError.Peer
            }
          }
        }*/
    }

    private fun announceSystemEvent(event: SystemEvent) {
        // TODO: Run on executor
        scope.launch {
            listener.handleSystemEvent(this@System, event)
        }
    }

    private fun announceNetworkEvent(network: Network, event: NetworkEvent) {
        // TODO: Run on executor
        scope.launch {
            listener.handleNetworkEvent(this@System, network, event)
        }
    }

    internal fun announceWalletManagerEvent(walletManager: WalletManager, event: WalletManagerEvent) {
        // TODO: Run on executor
        scope.launch {
            listener.handleManagerEvent(this@System, walletManager, event)
        }
    }

    internal fun announceWalletEvent(walletManager: WalletManager, wallet: Wallet, event: WalletEvent) {
        // TODO: Run on executor
        scope.launch {
            listener.handleWalletEvent(this@System, walletManager, wallet, event)
        }
    }

    private fun announceTransferEvent(
            walletManager: WalletManager,
            wallet: Wallet,
            transfer: Transfer,
            event: TransferEvent
    ) {
        // TODO: Run on executor
        scope.launch {
            listener.handleTransferEvent(this@System, walletManager, wallet, transfer, event)
        }
    }

    internal fun createWalletManager(coreWalletManager: BRCryptoWalletManager): WalletManager =
            WalletManager(coreWalletManager.take(), this, callbackCoordinator)
                    .also { walletManager ->
                        _walletManagers.add(walletManager)
                    }

    internal fun getWalletManager(coreWalletManager: BRCryptoWalletManager): WalletManager? {
        val walletManager = WalletManager(coreWalletManager.take(), this, callbackCoordinator)
        return if (_walletManagers.contains(walletManager)) walletManager else null
    }

    actual companion object {

        val Cookie.system get() = SYSTEMS_ACTIVE[this]

        private val SYSTEM_IDS = AtomicInteger(0)

        private const val SYSTEMS_INACTIVE_RETAIN = true

        private val SYSTEMS_ACTIVE = ConcurrentHashMap<Cookie, System>()

        private val SYSTEMS_INACTIVE = mutableListOf<System>()

        private fun ensurePath(storagePath: String): Boolean {
            val storageFile = File(storagePath)
            return ((storageFile.exists() || storageFile.mkdirs())
                    && storageFile.isDirectory
                    && storageFile.canWrite())
        }

        actual fun create(
                executor: ScheduledExecutorService,
                listener: SystemListener,
                account: Account,
                isMainnet: Boolean,
                storagePath: String,
                query: BdbService
        ): System? {
            val pathSeparator = if (storagePath.endsWith(File.separator)) "" else File.separator
            val accountStoragePath = storagePath + pathSeparator + account.core.filesystemIdentifier

            check(ensurePath(storagePath)) {
                "Failed to find or create storage directory."
            }

            val id = SYSTEM_IDS.incrementAndGet()
            val context = Cookie(id)

            val cwmListener = BRCryptoCWMListener(
                    context,
                    WalletManagerEventCallback,
                    WalletEventCallback,
                    TransferEventCallback
            )

            val cwmClient = cryptoClient(context)

            val system = System(
                    "", // TODO: SystemCallbackCoordinator
                    listener,
                    account,
                    isMainnet,
                    accountStoragePath,
                    query,
                    context,
                    cwmListener,
                    cwmClient
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

            val typeLowerCase = type.toLowerCase(Locale.ROOT)
            if ("erc20" != type && "native" != type) return null

            val codeLowerCase = code.toLowerCase(Locale.ROOT)
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
            val cipher = Cipher.createForChaCha20Poly1305(key, nonce12, authenticatedData)
            return cipher.core.migrateBRCoreKeyCiphertext(ciphertext).orNull()
        }

        actual fun wipe(system: System) {
            val storagePath = system.storagePath

            destroy(system)

            deleteRecursively(storagePath)
        }

        actual fun wipeAll(storagePath: String, exemptSystems: List<System>) {
            val exemptSystemPath = exemptSystems
                    .map(System::storagePath)
                    .toHashSet()

            File(storagePath)
                    .listFiles()
                    ?.filterNot { exemptSystemPath.contains(it.absolutePath) }
                    ?.forEach(::deleteRecursively)
        }

        private fun destroy(system: System) {
            SYSTEMS_ACTIVE.remove(system.context)

            system.disconnectAll()

            system.walletManagers.forEach(WalletManager::stop)

            system.scope.coroutineContext.cancelChildren()

            @Suppress("ConstantConditionIf")
            if (SYSTEMS_INACTIVE_RETAIN) {
                SYSTEMS_INACTIVE.add(system)
            }
        }

        private fun deleteRecursively(toDeletePath: String) {
            deleteRecursively(File(toDeletePath))
        }

        private fun deleteRecursively(toDelete: File) {
            if (toDelete.isDirectory) {
                toDelete.listFiles()?.forEach(::deleteRecursively)
            }
            if (toDelete.exists() && !toDelete.delete()) {
                // Log.log(Level.SEVERE, "Failed to delete " + toDelete.absolutePath)
            }
        }
    }
}
