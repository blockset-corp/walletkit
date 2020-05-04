package com.breadwallet.core

import com.breadwallet.core.api.BdbService
import com.breadwallet.core.common.Key
import com.breadwallet.corenative.crypto.*
import java.util.*

actual class WalletManager(
        internal val core: BRCryptoWalletManager,
        actual val system: System,
        private val callbackCoordinator: SystemCallbackCoordinator
) {

    actual val account: Account = Account(core.account)

    actual val network: Network = Network(core.network)

    internal actual val query: BdbService = system.query

    internal actual val unit: CUnit =
            checkNotNull(network.defaultUnitFor(network.currency))

    actual var mode: WalletManagerMode
        get() = WalletManagerMode.fromCoreInt(core.mode.toCore().toUInt())
        set(value) {
            require(network.supportsWalletManagerMode(value))
            core.mode = BRCryptoSyncMode.fromCore(value.core.toInt())
        }

    actual val path: String = core.path

    actual val state: WalletManagerState
        get() = core.state.asApiState()

    internal actual val height: ULong
        get() = network.height

    actual val primaryWallet: Wallet by lazy {
        Wallet(core.wallet, this, callbackCoordinator)
    }

    actual val wallets: List<Wallet> by lazy {
        core.wallets.map { Wallet(it, this, callbackCoordinator) }
    }

    actual val currency: Currency = network.currency

    actual val name: String = currency.code

    actual val baseUnit: CUnit = checkNotNull(network.baseUnitFor(network.currency))

    actual val defaultUnit: CUnit = checkNotNull(network.defaultUnitFor(network.currency))

    actual val isActive: Boolean
        get() = when (state) {
            WalletManagerState.CONNECTED,
            WalletManagerState.SYNCING -> true
            else -> false
        }

    actual val defaultNetworkFee: NetworkFee = network.minimumFee

    actual var addressScheme: AddressScheme
        get() = AddressScheme.fromCoreInt(core.addressScheme.toCore().toUInt())
        set(value) {
            core.addressScheme = BRCryptoAddressScheme.fromCore(value.core.toInt())
        }

    actual fun connect(peer: NetworkPeer?) {
        core.connect(peer?.core)
    }

    actual fun disconnect() {
        core.disconnect()
    }

    actual fun sync() {
        core.sync()
    }

    actual fun stop() {
        core.stop()
    }

    actual fun syncToDepth(depth: WalletManagerSyncDepth) {
        core.syncToDepth(BRCryptoSyncDepth.fromCore(depth.toSerialization().toInt()))
    }

    actual fun submit(transfer: Transfer, phraseUtf8: ByteArray) {
        core.submit(primaryWallet.core, transfer.core, phraseUtf8)
    }

    internal actual fun setNetworkReachable(isNetworkReachable: Boolean) {
        core.setNetworkReachable(isNetworkReachable)
    }

    actual fun registerWalletFor(currency: Currency): Wallet? {
        return core.registerWallet(currency.core)?.orNull()?.let { coreWallet ->
            Wallet(coreWallet, this, callbackCoordinator)
        }
    }

    actual fun createSweeper(
            wallet: Wallet,
            key: Key,
            completion: CompletionHandler<WalletSweeper, WalletSweeperError>
    ) {
        TODO("not implemented")
    }

    internal fun getWallet(coreWallet: BRCryptoWallet): Wallet? =
            if (core.containsWallet(coreWallet))
                Wallet(coreWallet.take(), this, callbackCoordinator)
            else null

    override fun toString(): String = name

    override fun hashCode(): Int = Objects.hash(core)

    override fun equals(other: Any?): Boolean =
            other is WalletManager && core == other.core

    internal fun createWallet(coreWallet: BRCryptoWallet): Wallet {
        return Wallet(coreWallet.take(), this, callbackCoordinator)
    }
}
