package com.breadwallet.core

import com.breadwallet.core.api.BdbService
import com.breadwallet.core.common.Key
import com.breadwallet.core.migration.BlockBlob
import com.breadwallet.core.migration.PeerBlob
import com.breadwallet.core.migration.TransactionBlob
import com.breadwallet.core.model.BdbCurrency
import kotlin.jvm.JvmOverloads


typealias ScheduledExecutorService = String

expect class System {

    internal val query: BdbService

    internal val isMainnet: Boolean

    public val account: Account

    public val storagePath: String

    public val networks: List<Network>

    public val walletManagers: List<WalletManager>

    public val wallets: List<Wallet>

    /**
     * Configure the system.  This will query various BRD services, notably the BlockChainDB, to
     * establish the available networks (aka blockchains) and their currencies.  For each
     * `Network` there will be `SystemEvent` which can be used by the App to create a
     * `WalletManager`.
     *
     * @param appCurrencies If the BlockChainDB does not return any currencies, then
     * use `applicationCurrencies` merged into the defaults.
     */
    public fun configure(appCurrencies: List<BdbCurrency>)

    /**
     * Create a wallet manager for `network` using `mode.
     *
     * Note: There are two preconditions - "network" must support "mode" and "addressScheme".
     * Thus a fatal error arises if, for example, the network is BTC and the scheme is ETH.
     *
     * @param network the wallet manager's network
     * @param mode the wallet manager mode to use
     * @param addressScheme the address scheme to use
     * @param currencies the currencies to register.  A wallet will be created for each one.  It
     * is safe to pass currencies not in "network" as they will be filtered (but bad form
     * to do so). The "primaryWallet", for the network's currency, is always created; if
     * the primaryWallet's currency is in `currencies` then it is effectively ignored.
     *
     * @return true on success; false on failure.
     */
    @JvmOverloads
    public fun createWalletManager(
            network: Network,
            mode: WalletManagerMode = network.defaultWalletManagerMode,
            addressScheme: AddressScheme = network.defaultAddressScheme,
            currencies: Set<Currency> = emptySet()
    ): Boolean

    /**
     * Remove (aka 'wipe') the persistent storage associated with `network` at `path`.
     *
     * This should be used solely to recover from a failure of `createWalletManager`.  A failure
     * to create a wallet manager is most likely due to corruption of the persistently stored data
     * and the only way to recover is to wipe that data.
     *
     * @param network the network to wipe data for
     */
    public fun wipe(network: Network)

    /**
     * Connect all wallet managers.
     *
     * They will be connected w/o an explict NetworkPeer.
     */
    public fun connectAll()

    /**
     * Disconnect all wallet managers.
     */
    public fun disconnectAll()

    public fun subscribe(subscriptionToken: String)

    /**
     * Update the NetworkFees for all known networks.  This will query the `BlockChainDB` to
     * acquire the fee information and then update each of system's networks with the new fee
     * structure.  Each updated network will generate a NetworkEvent.feesUpdated event (even if
     * the actual fees did not change).
     *
     * And optional completion handler can be provided.  If provided the completion handler is
     * invoked with an array of the networks that were updated or with an error.
     *
     * It is appropriate to call this function anytime a network's fees are to be used, such as
     * when a transfer is created and the User can choose among the different fees.
     *
     * @param completion An optional completion handler
     */
    public fun updateNetworkFees(completion: CompletionHandler<List<Network>, NetworkFeeUpdateError>?)

    /**
     * Set the network reachable flag for all managers.
     *
     * Setting or clearing this flag will NOT result in a connect/disconnect attempt by a [WalletManager].
     * Callers must use the [WalletManager.connect]/[WalletManager.disconnect] methods to
     * change a WalletManager's connectivity state. Instead, WalletManagers MAY consult this flag when performing
     * network operations to determine viability.
     */
    public fun setNetworkReachable(isNetworkReachable: Boolean)

    /**
     * If migration is required, return the currency code; otherwise, return nil.
     *
     * Note: it is not an error not to migrate.
     */
    public fun migrateRequired(network: Network): Boolean


    //public fun accountIsInitialized(account: Account, nework: Network): Boolean

    //public fun accountInitialize(account: Account, network: Network, create: Boolean, handler: CompletionHandler<ByteArray, AccountInitializationError>)

    //fun accountInitializeUsingData(account: Account, network: Network, data: ByteArray): ByteArray?

    //fun accountInitializeUsingHedera(account: Account?, network: Network?, hedera: HederaAccount): ByteArray?

    /**
     * Migrate the storage for a network given transaction, block and peer blobs.
     *
     * Support for persistent storage migration to allow prior App versions to migrate their SQLite
     * database representations of BTC/BTC transations, blocks and peers into 'Generic Crypto' - where
     * these entities are persistently
     *
     * The provided blobs must be consistent with `network`.  For exmaple, if `network` represents BTC or BCH
     * then the blobs must be of type [TransactionBlob.Btc]; otherwise a MigrateError is thrown.
     */
    public fun migrateStorage(
            network: Network,
            transactionBlobs: List<TransactionBlob>,
            blockBlobs: List<BlockBlob>,
            peerBlobs: List<PeerBlob>
    )

    companion object {
        /**
         * Create a new system.
         *
         * @param executor
         * @param listener the listener for handling events.
         * @param account the account, derived from a paper key, that will be used for all networks.
         * @param isMainnet flag to indicate if the system is for mainnet or for testnet; as blockchains
         * are announced, we'll filter them to be for mainent or testnet.
         * @param storagePath the path to use for persistent storage of data, such as for blocks, peers, transactions and
         * logs.
         * @param query the BlockchainDB query engine.
         */
        public fun create(
                executor: ScheduledExecutorService,
                listener: SystemListener,
                account: Account,
                isMainnet: Boolean,
                storagePath: String,
                query: BdbService
        ): System?

        /**
         * Create a BlockChainDB.Model.Currency to be used in the event that the BlockChainDB does
         * not provide its own currency model.
         *
         * @param uids the currency uids (ex: "ethereum-mainnet:0x558ec3152e2eb2174905cd19aea4e34a23de9ad6")
         * @param name the currency name (ex: "BRD Token"
         * @param code the currency code (ex: "code")
         * @param type the currency type (ex: "erc20" or "native")
         * @param decimals the number of decimals for the currency's default unit (ex: 18)
         * @return a currency mode for us with [.configure]; [Optional.absent] otherwise
         */
        public fun asBdbCurrency(
                uids: String,
                name: String,
                code: String,
                type: String,
                decimals: UInt
        ): BdbCurrency?

        /**
         * Re-encrypt ciphertext blobs that were encrypted using the BRCoreKey::encryptNative routine.
         *
         * The ciphertext will be decrypted using the previous decryption routine. The plaintext from that
         * operation will be encrypted using the current [Cipher.encrypt] routine with a
         * [Cipher.createForChaCha20Poly1305] cipher. That updated ciphertext
         * is then returned and should be used to immediately overwrite the old ciphertext blob so that
         * it can be properly decrypted using the [Cipher.decrypt] routine going forward.
         *
         * @param key The cipher key
         * @param nonce12 The 12 byte nonce data
         * @param authenticatedData The authenticated data
         * @param ciphertext The ciphertext to update the encryption on
         *
         * @return The updated ciphertext, if decryption and re-encryption succeeds; absent otherwise.
         */
        public fun migrateBRCoreKeyCiphertext(
                key: Key,
                nonce12: ByteArray,
                authenticatedData: ByteArray,
                ciphertext: ByteArray
        ): ByteArray?

        /**
         * Cease use of `system` and remove (aka 'wipe') its persistent storage.
         *
         * Caution is highly warranted; none of the System's references, be they Wallet Managers,
         * Wallets, Transfers, etc. should be *touched* once the system is wiped.
         *
         * Note: This function blocks until completed.  Be sure that all references are dereferenced
         * *before* invoking this function and remove the reference to `system` after this
         * returns.
         */
        public fun wipe(system: System): Unit

        /**
         * Remove (aka 'wipe') the persistent storage associated with any and all systems located
         * within `storagePath` except for a specified array of systems to preserve.  Generally, this
         * function should be called on startup after all systems have been created.  When called at
         * that time, any 'left over' systems will have their persistent storeage wiped.
         *
         * Note: This function will perform no action if `storagePath` does not exist or is
         * not a directory.
         *
         * @param storagePath the file system path where system data is persistently stored
         * @param exemptSystems the list of systems that should not have their data wiped.
         */
        public fun wipeAll(storagePath: String, exemptSystems: List<System>): Unit
    }
}
