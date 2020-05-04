package com.breadwallet.core

import com.breadwallet.core.api.BdbService
import com.breadwallet.core.common.Key

/**
 * A WallettManager manages one or more wallets one of which is designated the `primaryWallet`.
 *
 * (For example, an EthereumWalletManager will manage an ETH wallet and one wallet for each
 * ERC20Token; the ETH wallet will be the primaryWallet.  A BitcoinWalletManager manages one
 * and only one wallet holding BTC.).
 *
 * At least conceptually, a WalletManager is an 'Active Object' (whereas Transfer and Wallet are
 * 'Passive Objects'
 */
expect class WalletManager {

    /** The owning system */
    public val system: System

    /** The account */
    public val account: Account

    /** The network */
    public val network: Network

    /** The BlockChainDB for BRD Server Assisted queries. */
    internal val query: BdbService

    /** The default unit - as the networks default unit */
    internal val unit: CUnit

    /** The mode determines how the manager manages the account and wallets on network. */
    public var mode: WalletManagerMode

    /** The file-system path to use for persistent storage. */
    public val path: String

    /** The current state */
    public val state: WalletManagerState

    /** The current network block height */
    internal val height: ULong

    /**
     * The primaryWallet - holds the network's currency - this is typically the wallet where
     * fees are applied which may or may not differ from the specific wallet used for a
     * transfer (like BRD transfer => ETH fee)
     */
    public val primaryWallet: Wallet

    /** The managed wallets - often will just be listOf(primaryWallet) */
    public val wallets: List<Wallet>

    /**
     * The network's/primaryWallet's currency.
     *
     * This is the currency used for transfer fees.
     */
    public val currency: Currency

    /** The name is simply the network currency's code - e.g. BTC, ETH */
    public val name: String

    /** The baseUnit for the network's currency. */
    public val baseUnit: CUnit

    /** The defaultUnit for the network's currency. */
    public val defaultUnit: CUnit

    /** A manager `isActive` if connected or syncing */
    public val isActive: Boolean

    /** The default network fee. */
    public val defaultNetworkFee: NetworkFee

    /** The address scheme to use */
    public var addressScheme: AddressScheme

    /**
     * Connect to the network and begin managing wallets.
     *
     * - Note: If peer is provided, there is a precondition on the networks matching.
     *
     * @param peer An optional NetworkPeer to use on the P2P network.  It is unusual to
     *      provide a peer as P2P networks will dynamically discover suitable peers.
     */
    public fun connect(peer: NetworkPeer? = null)

    /** Disconnect from the network */
    public fun disconnect()

    public fun sync()

    public fun stop()

    public fun createSweeper(wallet: Wallet, key: Key, completion: CompletionHandler<WalletSweeper, WalletSweeperError>)

    public fun syncToDepth(depth: WalletManagerSyncDepth)

    public fun submit(transfer: Transfer, phraseUtf8: ByteArray)

    internal fun setNetworkReachable(isNetworkReachable: Boolean)

    /**
     * Ensure that a wallet for currency exists.  If the wallet already exists, it is returned.
     * If the wallet needs to be created then `nil` is returned and a series of events will
     * occur - notably WalletEvent.created and WalletManagerEvent.walletAdded if the wallet is
     * created
     *
     * Note: There is a precondition on `currency` being one in the managers' network
     *
     * @return The wallet for currency if it already exists, otherwise "absent"
     */
    public fun registerWalletFor(currency: Currency): Wallet?
}
