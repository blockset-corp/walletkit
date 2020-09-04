//
//  BRSystemSystem.swift
//  WalletKit
//
//  Created by Ed Gamble on 3/27/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
import Foundation  // Data, DispatchQueue
import WalletKitCore

///
/// System (a singleton)
///
public final class System {

    /// The Core representation
    internal private(set) var core: BRCryptoSystem! = nil

    /// The listener.  Gets all events for {Network, WalletManger, Wallet, Transfer}
    public private(set) weak var listener: SystemListener?

    /// The listenerQueue where all listener 'handle events' are asynchronously performed.
    internal let listenerQueue: DispatchQueue

    // TODO: The 'account uids' only; if 'universal', the 'network uid' is needed
    public let uids: String
    
    /// The account
    public let account: Account

    /// The path for persistent storage
    public let path: String

    /// If on mainnet
    public let onMainnet: Bool

    /// The client to use for queries
    public let client: SystemClient

    /// The queue for asynchronous functionality.  Notably this is used in `configure()` to
    /// gather all of the `query` results as Networks are created and added.
    internal let queue = DispatchQueue (label: "Crypto System")

    internal let callbackCoordinator: SystemCallbackCoordinator

    /// Flag indicating if the network is reachable; defaults to true
    internal var isNetworkReachable = true

    // the number of networks
    public var networksCount: Int {
        return cryptoSystemGetNetworksCount (core)
    }

    /// the networks, unsorted.
    public var networks: [Network] {
        var count: BRCryptoCount = 0;
        let pointers = cryptoSystemGetNetworks (core, &count);
        defer { if let ptr = pointers { cryptoMemoryFree (ptr) }}

        let networks: [BRCryptoNetwork] = pointers?
            .withMemoryRebound (to: BRCryptoNetwork.self, capacity: count) {
                Array (UnsafeBufferPointer (start: $0, count: count))
        } ?? []

        return networks.map { Network (core: $0, take: false) }
    }

     internal func networkBy (uids: String) -> Network? {
        return cryptoSystemGetNetworkForUids (core, uids)
            .map { Network (core: $0, take: false) }
      }

    internal func networkBy (core: BRCryptoNetwork) -> Network? {
        return networks.first { $0.core == core }
     }

    // the number of managers
    public var managersCount: Int {
        return cryptoSystemGetWalletManagersCount (core)
    }

     /// The wallet managers, unsorted.  A WalletManager will hold an 'unowned'
    /// reference back to `System`
    public var managers: [WalletManager] {
        var count: BRCryptoCount = 0;
        var pointers = cryptoSystemGetWalletManagers (core, &count);
        defer { if let ptr = pointers { cryptoMemoryFree (ptr) }}

        let managers: [BRCryptoWalletManager] = pointers?
            .withMemoryRebound (to: BRCryptoWalletManager.self, capacity: count) {
                Array (UnsafeBufferPointer (start: $0, count: count))
        } ?? []

        return managers.map {
            WalletManager (core: $0,
                           system: self,
                           callbackCoordinator: callbackCoordinator,
                           take: false) }
    }

     internal func managerBy (core: BRCryptoWalletManager) -> WalletManager? {
        return (CRYPTO_TRUE == cryptoSystemHasWalletManager (self.core, core)
            ? WalletManager (core: core, system: self, callbackCoordinator: callbackCoordinator, take: true)
            : nil)
    }

    internal func managerBy (network: BRCryptoNetwork) -> WalletManager? {
        return cryptoSystemGetWalletManagerByNetwork (core, network)
            .map { WalletManager (core: $0,
                                  system: self,
                                  callbackCoordinator: callbackCoordinator,
                                  take: false)
        }
    }

    ///
    /// Create a wallet manager for `network` using `mode`, `addressScheme`, and `currencies`.  A
    /// wallet will be 'registered' for each of:
    ///    a) the network's currency - this is the primaryWallet
    ///    b) for each of `currences` that are in `network`.
    /// These wallets are announced using WalletEvent.created and WalletManagerEvent.walledAdded.
    ///
    /// The created wallet manager is announed using WalletManagerEvent.created and
    /// SystemEvent.managerAdded.
    ///
    /// - Parameters:
    ///   - network: the wallet manager's network
    ///   - mode: the wallet manager mode to use
    ///   - addressScheme: the address scheme to use
    ///   - currencies: the currencies to 'register'.  A wallet will be created for each one.  It
    ///       is safe to pass currencies not in `network` as they will be filtered (but bad form
    ///       to do so).  The 'primaryWallet', for the network's currency, is always created; if
    ///       the primaryWallet's currency is in `currencies` then it is effectively ignored.
    ///
    /// - Returns: `true` on success; `false` on failure.
    ///
    /// - Note: There are two preconditions - `network` must support `mode` and `addressScheme`.
    ///     Thus a fatal error arises if, for example, the network is BTC and the scheme is ETH.
    ///
    public func createWalletManager (network: Network,
                                     mode: WalletManagerMode,
                                     addressScheme: AddressScheme,
                                     currencies: Set<Currency>) -> Bool {
        precondition (network.supportsMode(mode))
        precondition (network.supportsAddressScheme(addressScheme))

        var coreCurrences: [BRCryptoCurrency?] = currencies.map { $0.core }

        return nil != cryptoSystemCreateWalletManager (core,
                                                       network.core,
                                                       mode.core,
                                                       addressScheme.core,
                                                       &coreCurrences,
                                                       coreCurrences.count)

//        guard accountIsInitialized (account, onNetwork: network),
//            let manager = WalletManager (system: self,
//                                         callbackCoordinator: callbackCoordinator,
//                                         account: account,
//                                         network: network,
//                                         mode: mode,
//                                         addressScheme: addressScheme,
//                                         currencies: currencies,
//                                         storagePath: path,
//                                         listener: cryptoListener,
//                                         client: cryptoClient)
//            else { return false }
//        
//        manager.setNetworkReachable(isNetworkReachable)
//        self.add (manager: manager)
//        return true
    }

    ///
    /// Remove (aka 'wipe') the persistent storage associated with `network` at `path`.  This should
    /// be used solely to recover from a failure of `createWalletManager`.  A failure to create
    /// a wallet manager is most likely due to corruption of the persistently stored data and the
    /// only way to recover is to wipe that data.
    ///
    /// - Parameters:
    ///   - network: network to wipe data for
    ///
    public func wipe (network: Network) {
        // Racy - but if there is no wallet manager for `network`... then
        if !managers.contains(where: { network == $0.network }) {
            cryptoWalletManagerWipe (network.core, path);
        }
    }

    // Wallets - derived as a 'flatMap' of the managers' wallets.
    public var wallets: [Wallet] {
        return managers.flatMap { $0.wallets }
    }

    // Account Initialization
    
    public enum AccountInitializationError: Error {
        /// The function `accountInitialize(...callback)` was called but `accountIsInitialized()`
        /// return `true`.  No account initialization is required.
        case alreadyInitialized

        /// If account initialization requires a remote/HTTP query, then the query has failed.
        /// It is worth retrying.  Could occur if network connectivity is lost or remote/HTTP
        /// services are down.
        case queryFailure(String)

        /// An attempt was made to get the required account initialization data (aka 'create an
        /// account on the specified network') but the creation failed.  It is worth retrying but
        /// a delay may be warrented.  That is 'creating an account' might be in progress but
        /// got reported as failed; waiting a bit might allow the 'create' to complete.
        case cantCreate

        /// An attempt was made to get the required account initialization data (aka 'create an
        /// account on the specified network') but multiple accounts already exist.  This is
        /// unusual and may indicate a) a sophisticated User creating accounts outside of BRD, or
        /// b) and error during a BRD create that 'missed' a creation and now multiple exist.
        /// Chose the preferred account from among the array - like the one with the largest
        /// balance and then call `accountInitialize(...hedera)`
        case multipleHederaAccounts ([SystemClient.HederaAccount])
    }

    ///
    /// Check if `Account` is initialized.  A WalletManger for {account, network} can not be
    /// created unless the account is initialized.  Some blockchains, such as Hedera, require
    /// a distinct initialization process.
    ///
    /// - Parameters:
    ///   - account: The Account
    ///   - network: The Network
    ///
    /// - Returns: `true` if initialized, `false` otherwise.
    ///
    public func accountIsInitialized (_ account: Account, onNetwork network: Network) -> Bool {
        return account.isInitialized(onNetwork: network)
    }

    ///
    /// Initialize an account for network.
    ///
    /// - Parameters:
    ///   - account: The Account
    ///   - network: The Network
    ///   - completion: A Handler that is invoked once initialization is complete.  On success,
    ///       the `Result` contains `account serialization data` that must be persistently stored.
    ///
    public func accountInitialize (_ account: Account,
                                   onNetwork network: Network,
                                   createIfDoesNotExist create: Bool,
                                   completion: @escaping  (Result<Data, AccountInitializationError>) -> Void) {
        guard !accountIsInitialized(account, onNetwork: network)
            else {
                accountInitializeReportResult (Result.failure(.alreadyInitialized), completion)
                return
        }

        switch network.type {
        case .hbar:
            // For Hedera, the account initialization data is the public key.
            guard let publicKey = account.getInitializationdData(onNetwork: network)
                .flatMap ({ CoreCoder.hex.encode(data: $0) })
            else {
                accountInitializeReportResult (Result.failure(.queryFailure("No initialization data")), completion)
                return
            }

            print ("SYS: Account: Hedera: publicKey: \(publicKey)")

            // Find a pre-existing account or create one if necessary.
            client.getHederaAccount (blockchainId: network.uids, publicKey: publicKey) {
                (res: Result<[SystemClient.HederaAccount], SystemClientError>) in
                self.accountInitializeHandleHederaResult (create: create,
                                                          network: network,
                                                          publicKey: publicKey,
                                                          res: res,
                                                          completion: completion)
            }

        default:
            preconditionFailure("Initialization Is Not Supported: \(network.type)")
        }
    }

    ///
    /// If somehow, magically, you've got `initializationData`, use it to initialize `account`.
    /// Note that `initializationData` is not `account serialization data`; don't confuse them.
    ///
    /// - Parameters:
    ///   - account: The account
    ///   - network: The network
    ///   - data: accounit initialization data
    ///
    /// - Returns: An Account serialization that must be persistently stored; otherwise really bad
    ///     things will happen.
    ///
    public func accountInitialize (_ account: Account,
                                   onNetwork network: Network,
                                   using data: Data) -> Data? {
        return account.initialize (onNetwork: network, using: data)
    }

    ///
    /// Initialize a Hedera account based on a 'selection'.  On `accountInitialize` one fo the
    /// errors is `multipleHederaAccounts`.  If that occurs, you should select one of them and
    /// call this function with the one selected.
    ///
    /// - Parameters:
    ///   - account: The Account
    ///   - network: The Network
    ///   - hedera:  A HederaAccount selected from among
    ///         `AccountInitializationError.multipleHederaAccounts`
    ///
    /// - Returns: An Account serialization that must be persistently stored; otherwise really bad
    ///     things will happen.
    ///
    public func accountInitialize (_ account: Account,
                                   onNetwork network: Network,
                                   hedera: SystemClient.HederaAccount) -> Data? {
        return hedera.id.data(using: .utf8)
            .flatMap { accountInitialize (account, onNetwork: network, using: $0) }
    }

    private func accountInitializeHandleHederaResult (create: Bool,
                                                      network: Network,
                                                      publicKey: String,
                                                      res: Result<[SystemClient.HederaAccount], SystemClientError>,
                                                      completion: @escaping  (Result<Data, AccountInitializationError>) -> Void) {
        res.resolve (
            success: { (accounts: [SystemClient.HederaAccount]) in
                switch accounts.count {
                case 0:
                    if !create { accountInitializeReportResult (Result.failure(.cantCreate), completion) }
                    else {
                        client.createHederaAccount (blockchainId: network.uids, publicKey: publicKey) {
                            (res: Result<[SystemClient.HederaAccount], SystemClientError>) in
                            self.accountInitializeHandleHederaResult (create: false,
                                                                      network: network,
                                                                      publicKey: publicKey,
                                                                      res: res,
                                                                      completion: completion)
                        }
                    }

                case 1:
                    print ("SYS: Account: Hedera: AccountID: \(accounts[0].id), Balance: \(accounts[0].balance?.description ?? "<none>")")

                    let serialization = accountInitialize (account,
                                                           onNetwork: network,
                                                           hedera: accounts[0])
                    accountInitializeReportResult (Result.success(serialization), completion)

                default:
                    accountInitializeReportResult (Result.failure (.multipleHederaAccounts (accounts)), completion)
                }},

            failure: { (error: SystemClientError) in
                accountInitializeReportResult (Result.failure (.queryFailure (error.localizedDescription)), completion)
        })
    }

    private func accountInitializeReportResult (_ res: Result<Data?, AccountInitializationError>,
                                                _ completion: @escaping  (Result<Data, AccountInitializationError>) -> Void) {
        queue.async { completion (res.flatMap {
            $0.map { Result.success ($0) } ?? Result.failure(.queryFailure("No Data")) }) }
    }

    /// System Initialization

    static func ensurePath (_ path: String) -> Bool {
        // Apple on `fileExists`, `isWritableFile`
        //    "The following methods are of limited utility. Attempting to predicate behavior
        //     based on the current state of the filesystem or a particular file on the filesystem
        //     is encouraging odd behavior in the face of filesystem race conditions. It's far
        //     better to attempt an operation (like loading a file or creating a directory) and
        //     handle the error gracefully than it is to try to figure out ahead of time whether
        //     the operation will succeed."

        do {
            // Ensure that `storagePath` exists.  Seems the return value can be ignore:
            //    "true if the directory was created, true if createIntermediates is set and the
            //     directory already exists, or false if an error occurred."
            // The `attributes` need not be provided as we have write permission :
            //    "Permissions are set according to the umask of the current process"
            try FileManager.default.createDirectory (atPath: path,
                                                     withIntermediateDirectories: true,
                                                     attributes: nil)
        }
        catch { return false }

        // Ensure `path` is writeable.
        return FileManager.default.isWritableFile (atPath: path)
    }

    ///
    /// Initialize System
    ///
    /// - Parameters:
    ///   - listener: The listener for handlng events.
    ///
    ///   - client: The client for external data
    ///
    ///   - account: The account, derived from the `paperKey`, that will be used for all networks.
    ///
    ///   - onMainnet: boolean to indicate if this is for mainnet or for testnet.  As blockchains
    ///       are announced, we'll filter them to be for mainent or testnet.
    ///
    ///   - path: The path to use for persistent storage of data, such as for BTC and ETH blocks,
    ///       peers, transaction and logs.
    ///
    ///   - query: The query for BlockchiainDB interaction.
    ///
    ///   - listenerQueue: The queue to use when performing listen event handler callbacks.  If a
    ///       queue is not specficied (default to `nil`), then one will be provided.
    ///
    internal init (client: SystemClient,
                   listener: SystemListener,
                   account: Account,
                   onMainnet: Bool,
                   path: String,
                   listenerQueue: DispatchQueue? = nil) {

        let basePath = path.hasSuffix("/") ? String(path.dropLast()) : path
        let uids     = account.fileSystemIdentifier
        precondition (System.ensurePath (basePath + "/" + uids))

        self.uids      = uids
        self.path      = basePath + "/" + uids

        self.listener  = listener
        self.client    = client
        self.account   = account
        self.onMainnet = onMainnet
        self.listenerQueue = listenerQueue ?? DispatchQueue (label: "Crypto System Listener")
        self.callbackCoordinator = SystemCallbackCoordinator (queue: self.listenerQueue)

        // Assign a system identifier.  This happens here so that `cryptoClient` and
        // `cryptoListener` will have their context (which is based on `self.index`)
        self.index = System.systemIndexIncrement;

        self.core = cryptoSystemCreate (self.cryptoClient,
                                        self.cryptoListener,
                                        account.core,
                                        basePath,
                                        onMainnet ? CRYPTO_TRUE : CRYPTO_FALSE)

        // Add `system` to our known set of systems; this allows event callbacks from Core to
        // find the initiating Swift instance.
        System.systemExtend (with: self)

        // Start `system` so that the event handlers are started and thus events are processed.
        // There is not explicit interface for start/stop; and thus no stopping `system` now.
        cryptoSystemStart (self.core)
        
    }

    ///
    /// Subscribe (or unsubscribe) to BlockChainDB notifications.  Notifications provide an
    /// asynchronous announcement of DB changes pertinent to the User and are used to avoid
    /// polling the DB for such changes.
    ///
    /// The Subscription includes an `endpoint` which is optional.  If provided, subscriptions
    /// are enabled; if not provided, subscriptions are disabled.  Disabling a sbuscription is
    /// required, even though polling in undesirable, because Notifications are user configured.
    ///
    /// - Parameter subscription: the subscription to enable or to disable notifications
    ///
    public func subscribe (using subscription: SystemClient.Subscription) {
        self.client.subscribe (walletId: account.uids,
                              subscription: subscription)
    }

    ///
    /// Announce a BlockChainDB transaction.  This should be called upon the "System User's"
    /// receipt of a BlockchainDB notification.
    ///
    /// - Parameters:
    ///   - transaction: the transaction id which can be used in `getTransfer` to query the
    ///         blockchainDB for details on the transaction
    ///   - data: The transaction JSON data (a dictionary) if available
    ///
    public func announce (transaction id: String, data: [String: Any]) {
        print ("SYS: Announce: \(id)")
    }
    #if false
    internal func updateSubscribedWallets () {
        let currencyKeyValues = wallets.map { ($0.currency.code, [$0.source.description]) }
        let wallet = (id: account.uids,
                      currencies: Dictionary (uniqueKeysWithValues: currencyKeyValues))
        self.client.updateWallet (wallet) { (res: Result<SystemClient.Wallet, SystemClientError>) in
            print ("SYS: SubscribedWallets: \(res)")
        }
    }
    #endif
    /// MARK: - Network Fees

    ////
    /// A NetworkFeeUpdateError
    ///
    public enum NetworkFeeUpdateError: Error {
        /// The query endpoint for netowrk fees is unresponsive
        case feesUnavailable
    }

    ///
    /// Update the NetworkFees for all known networks.  This will query the `BlockChainDB` to
    /// acquire the fee information and then update each of system's networks with the new fee
    /// structure.  Each updated network will generate a NetworkEvent.feesUpdated event (even if
    /// the actual fees did not change).
    ///
    /// And optional completion handler can be provided.  If provided the completion handler is
    /// invoked with an array of the networks that were updated or with an error.
    ///
    /// It is appropriate to call this function anytime a network's fees are to be used, such as
    /// when a transfer is created and the User can choose among the different fees.
    ///
    /// - Parameter completion: An optional completion handler
    ///
    public func updateNetworkFees (_ completion: ((Result<[Network],NetworkFeeUpdateError>) -> Void)? = nil) {
        self.client.getBlockchains (mainnet: self.onMainnet) {
            (blockChainResult: Result<[SystemClient.Blockchain],SystemClientError>) in

            // On an error, just skip out; we'll query again later, presumably
            guard case let .success (blockChainModels) = blockChainResult
                else {
                    completion? (Result.failure (NetworkFeeUpdateError.feesUnavailable))
                    return
            }

            let networks = blockChainModels.compactMap { (blockChainModel: SystemClient.Blockchain) -> Network? in
                guard let network = self.networkBy (uids: blockChainModel.id)
                    else { return nil }

                // We always have a feeUnit for network
                let feeUnit = network.baseUnitFor(currency: network.currency)!

                // Get the fees
                let fees = blockChainModel.feeEstimates
                    // Well, quietly ignore a fee if we can't parse the amount.
                    .compactMap { (fee: SystemClient.BlockchainFee) -> NetworkFee? in
                        return Amount.create (string: fee.amount, unit: feeUnit)
                            .map { NetworkFee (timeIntervalInMilliseconds: fee.confirmationTimeInMilliseconds,
                                               pricePerCostFactor: $0) }
                }

                // The fees are unlikely to change; but we'll announce .feesUpdated anyways.
                network.fees = fees

                return network
            }

            completion? (Result.success(networks))
        }
    }

    ///
    /// Set the network reachable flag for all managers. Setting or clearing this flag will
    /// NOT result in a connect/disconnect operation by a manager. Callers must use the
    /// `connect`/`disconnect` calls to change a WalletManager's connectivity state. Instead,
    /// managers MAY consult this flag when performing network operations to determine their
    /// viability.
    ///
    public func setNetworkReachable (_ isNetworkReachable: Bool) {
        self.isNetworkReachable = isNetworkReachable
        managers.forEach { $0.setNetworkReachable(isNetworkReachable) }
    }

    // MARK: - Pause/Resume

    ///
    /// Pause by disconnecting all wallet managers among other things.
    ///
    public func pause () {
        // If called when we've no networks, then we've never been configured.  Ignore pausing.
        if !networks.isEmpty {
            print ("SYS: Pause")
            managers.forEach { $0.disconnect() }
            client.cancelAll()
        }
    }

    /// Resume by connecting all wallet managers among other things
    ///
    public func resume () {
        // If called when we've no networks, then we've never been configured.  Ignore resuming.
        if 0 != managersCount {
            print ("SYS: Resume")
            configure(withCurrencyModels: [])
            managers.forEach { $0.connect() }
        }
    }

    // MARK: - Configure

    ///
    /// Configure the system.  This will query various BRD services, notably the BlockChainDB, to
    /// establish the available networks (aka blockchains) and their currencies.  For each
    /// `Network` there will be `SystemEvent` which can be used by the App to create a
    /// `WalletManager`
    ///
    /// - Parameter applicationCurrencies: If the BlockChainDB does not return any currencies, then
    ///     use `applicationCurrencies` merged into the deafults.  Appropriate currencies can be
    ///     created from `System::asBlockChainDBModelCurrency` (see below)
    ///
    public func configure (network: Network, currenciesFrom currencyModels: [SystemClient.Currency]) {
        func currencyDenominationToBaseUnit (currency: Currency, model: SystemClient.CurrencyDenomination) -> Unit {
            return Unit (currency: currency,
                         code:   model.code,
                         name:   model.name,
                         symbol: model.symbol)
        }

        func currencyToDefaultBaseUnit (currency: Currency) -> Unit {
            return Unit (currency: currency,
                         code:   "\(currency.code.lowercased())i",
                         name:   "\(currency.name) INT",
                         symbol: "\(currency.code.uppercased())I")
        }

        func currencyDenominationToUnit (currency: Currency, model: SystemClient.CurrencyDenomination, base: Unit) -> Unit {
            return Unit (currency: currency,
                         code:   model.code,
                         name:   model.name,
                         symbol: model.symbol,
                         base:   base,
                         decimals: model.decimals)
        }

        currencyModels
            .filter { $0.blockchainID == network.uids }
            .forEach { (currencyModel: SystemClient.Currency) in
                // Create the currency
                let currency = Currency (uids: currencyModel.id,
                                         name: currencyModel.name,
                                         code: currencyModel.code,
                                         type: currencyModel.type,
                                         issuer: currencyModel.address)

                // Create the base unit
                let baseUnit: Unit = currencyModel.demoninations.first { 0 == $0.decimals }
                    .map { currencyDenominationToBaseUnit(currency: currency, model: $0) }
                    ?? currencyToDefaultBaseUnit (currency: currency)

                // Create the other units
                var units: [Unit] = [baseUnit]
                units += currencyModel.demoninations.filter { 0 != $0.decimals }
                    .map { currencyDenominationToUnit (currency: currency, model: $0, base: baseUnit) }

                // Find the default unit
                let maximumDecimals = units.reduce (0) { max ($0, $1.decimals) }
                let defaultUnit = units.first { $0.decimals == maximumDecimals }!

                // Add the currency - this will not replace an existing currency
                // Note, a builtin network *always* has a native currency at
                // least which is set as the network's currency.
                network.addCurrency (currency, baseUnit: baseUnit, defaultUnit: defaultUnit)

                // Add the units - this will not replace an existing unti
                units.forEach { network.addUnitFor (currency: currency, unit: $0) }
        }
    }

    func configure (network: Network, feesFrom blockchainModel: SystemClient.Blockchain) {
        guard let feeUnitForParse = network.baseUnitFor (currency: network.currency),
            // The feeUnit is always the network currency's default unit
            let feeUnit = network.defaultUnitFor(currency: network.currency)
            else { return }

        // Extract the network fees from the blockchainModel
        let fees = blockchainModel.feeEstimates
            // Well, quietly ignore a fee if we can't parse the amount.
            .compactMap { (fee: SystemClient.BlockchainFee) -> NetworkFee? in
                let timeInterval  = fee.confirmationTimeInMilliseconds
                return Amount.create (string: fee.amount, unit: feeUnitForParse)
                    .map { $0.convert(to: feeUnit)! }
                    .map { NetworkFee (timeIntervalInMilliseconds: timeInterval,
                                       pricePerCostFactor: $0) }
        }

        // We require fees
        guard !fees.isEmpty
            else { print ("SYS: CONFIGURE: Missed Fees (\(blockchainModel.name)) on '\(blockchainModel.network)'"); return }

        // Update the network's fees.
        network.fees = fees
    }

     ///
    /// Configure the system.  This will query various BRD services, notably the BlockChainDB, to
    /// establish the available networks (aka blockchains) and their currencies.  For each
    /// `Network` there will be `SystemEvent` which can be used by the App to create a
    /// `WalletManager`
    ///
    /// - Parameter applicationCurrencies: If the BlockChainDB does not return any currencies, then
    ///     use `applicationCurrencies` merged into the deafults.  Appropriate currencies can be
    ///     created from `System::asBlockChainDBModelCurrency` (see below)
    ///
    public func configure (withCurrencyModels applicationCurrencies: [SystemClient.Currency]) {
        // The networks we've already processed
        let existingNetworks   = networks

        // The 'discovered networks' will all be announced at once.
        var discoveredNetworks = [Network]()

        // The 'supported networks' will be the built-in networks matching 'onMainnet'.  There is
        // no harm in calling this multiple times.
        let supportedNetworks = self.networks

        func announceNetwork (_ network: Network) {
            // Save the network
//             self.networks.append (network)

             self.listenerQueue.async {
                 // Announce NetworkEvent.created...
                 self.listener?.handleNetworkEvent (system: self, network: network, event: NetworkEvent.created)

                 // Announce SystemEvent.networkAdded - this will likely be handled with
                 // system.createWalletManager(network:...) which will then announce
                 // numerous events as wallets are created.
                 self.listener?.handleSystemEvent  (system: self, event: SystemEvent.networkAdded(network: network))
             }

             // Keep a running total of discovered networks
             discoveredNetworks.append(network)
        }

        func announceNetworkUpdate (_ network: Network) {
            self.listenerQueue.async {
                self.listener?.handleNetworkEvent (system: self, network: network, event: .updated)
            }
        }

        // Query for blockchains.
        self.client.getBlockchains (mainnet: self.onMainnet) {
            (blockchainResult: Result<[SystemClient.Blockchain],SystemClientError>) in

            // If the query failed, soldier on without any models
            let blockchainModels = blockchainResult
                .getWithRecovery {
                    print ("SYS: CONFIGURE: Missed Blockchains Query: \($0)")
                    return []
            }

            // Make a map from the supported models
            let blockchainModelsMap = Dictionary (uniqueKeysWithValues:
                blockchainModels.map { ($0.id, $0) })

            // Query currencies for all blockchains on mainnet/testnet
            self.client.getCurrencies (mainnet: self.onMainnet) {
                (currencyResult: Result<[SystemClient.Currency],SystemClientError>) in

                // If the query failed, soldier on with the `applicationCurrencies`.  We may have
                // a mixture of 'mainnet' and 'testnet' in this result.
                let currencyModels = currencyResult
                    .getWithRecovery {
                        print ("SYS: CONFIGURE: Missed Currencies Query: \($0)")
                        return applicationCurrencies
                            .filter { $0.verified == true }
                }

                // Handle each network
                supportedNetworks
                    .forEach { (network: Network) in

                        // Record if we already know about `network`
                        let existing = false // existingNetworks.contains(network)

                        // Configure the network using the currencyModels.  If currency models have
                        // been added, then we'll add those to `network`.
                        self.configure (network: network, currenciesFrom: currencyModels)

                        // If we have a blockChainModel, configure the network fees.
                        if let blockchainModel = blockchainModelsMap[network.uids] {
                            if let blockHeight = blockchainModel.blockHeight {
                                cryptoNetworkSetHeight (network.core, blockHeight)
                            }

                            self.configure (network: network, feesFrom: blockchainModel)
                        }
                        else { print ("SYS: CONFIGURE: Missed model for network: \(network.uids)") }

                        // If this is the first we've learn of this network, then announce it
                        if !existing {
                            announceNetwork(network)
                        }

                        // Otherwise, announce a network update
                        else {
                            announceNetworkUpdate(network)
                        }
                }

                // Announce the newly discoveredNetworks on completion
                if existingNetworks.isEmpty || !discoveredNetworks.isEmpty {
                    self.listenerQueue.async {
                        self.listener?.handleSystemEvent(system: self, event: SystemEvent.discoveredNetworks (networks: discoveredNetworks))
                    }
                }
            }
        }
    }

    private static func makeCurrencyDemominationsERC20 (_ code: String, decimals: UInt8) -> [SystemClient.CurrencyDenomination] {
        let name = code.uppercased()
        let code = code.lowercased()

        return [
            (name: "\(name) Token INT", code: "\(code)i", decimals: 0,        symbol: "\(code)i"),   // BRDI -> BaseUnit
            (name: "\(name) Token",     code: code,       decimals: decimals, symbol: code)
        ]
    }

    ///
    /// Create a SystemClient.Currency to be used in the event that the BlockChainDB does
    /// not provide its own currency model.
    ///
    public static func asBlockChainDBModelCurrency (uids: String, name: String, code: String, type: String, decimals: UInt8) -> SystemClient.Currency? {
        // convert to lowercase to match up with built-in blockchains
        let type = type.lowercased()
        guard "erc20" == type || "native" == type else { return nil }
        return uids.firstIndex(of: ":")
            .map {
                let code         = code.lowercased()
                let blockchainID = uids.prefix(upTo: $0).description
                let address      = uids.suffix(from: uids.index (after: $0)).description

                return (id:   uids,
                        name: name,
                        code: code,
                        type: type,
                        blockchainID: blockchainID,
                        address: (address != "__native__" ? address : nil),
                        verified: true,
                        demoninations: System.makeCurrencyDemominationsERC20 (code, decimals: decimals))
        }
    }


    //
    // Static Weak System References
    //

    /// A serial queue to protect `systemIndex` and `systemMapping`
    static let systemQueue = DispatchQueue (label: "System", attributes: .concurrent)

    /// A index to globally identify systems.
    static var systemIndex: Int32 = 0;

    /// Increment the index
    static var systemIndexIncrement: Int32 {
        systemQueue.sync {
            systemIndex += 1
            return systemIndex
        }
    }

    /// A dictionary mapping an index to a system.
    static var systemMapping: [Int32 : System] = [:]

    ///
    /// Lookup a `System` from an `index
    ///
    /// - Parameter index:
    ///
    /// - Returns: A System if it is mapped by the index and has not been GCed.
    ///
    static func systemLookup (index: Int32) -> System? {
        return systemQueue.sync {
            return systemMapping[index]
        }
    }

    /// An array of removed systems.  This is a workaround for systems that have been destroyed.
    /// We do not correctly handle 'release' and thus C-level memory issues are introduced; rather
    /// than solving those memory issues now, we'll avoid 'release' by holding a reference.
    private static var systemRemovedSystems = [System]()

    /// If true, save removed system in the above array. Set to `false` for debugging 'release'.
    private static var systemRemovedSystemsSave = true;

    static func systemRemove (index: Int32) {
        return systemQueue.sync (flags: .barrier) {
            systemMapping.removeValue (forKey: index)
                .map {
                    if systemRemovedSystemsSave {
                        systemRemovedSystems.append ($0)
                    }
            }
        }
    }

    ///
    /// Add a systme to the mapping.  Create a new index in the process and assign as system.index
    ///
    /// - Parameter system:
    ///
    /// - Returns: the system's index
    ///
    static func systemExtend (with system: System) {
        systemQueue.async (flags: .barrier) {
            systemMapping[system.index] = system
        }
    }

    static func systemExtract (_ context: BRCryptoListenerContext!) -> System? {
        precondition (nil != context)

        let index = 1 + Int32(UnsafeMutableRawPointer(bitPattern: 1)!.distance(to: context))

        return systemLookup(index: index)
    }

    static func systemExtract (_ context: BRCryptoListenerContext!,
                               _ cwm: BRCryptoWalletManager!) -> (System, WalletManager)? {
        precondition (nil != context  && nil != cwm)

        return systemExtract(context)
            .map { ($0, WalletManager (core: cwm,
                                       system: $0,
                                       callbackCoordinator: $0.callbackCoordinator,
                                       take: true))
        }
    }

    static func systemExtract (_ context: BRCryptoListenerContext!,
                               _ cwm: BRCryptoWalletManager!,
                               _ wid: BRCryptoWallet!) -> (System, WalletManager, Wallet)? {
        precondition (nil != context  && nil != cwm && nil != wid)

        return systemExtract (context, cwm)
            .map { ($0.0,
                    $0.1,
                    $0.1.walletByCoreOrCreate (wid, create: true)!)
        }
    }

    static func systemExtract (_ context: BRCryptoListenerContext!,
                               _ cwm: BRCryptoWalletManager!,
                               _ wid: BRCryptoWallet!,
                               _ tid: BRCryptoTransfer!) -> (System, WalletManager, Wallet, Transfer)? {
        precondition (nil != context  && nil != cwm && nil != wid && nil != tid)

        // create -> CRYPTO_TRANSFER_EVENT_CREATED == event.type
        return systemExtract (context, cwm, wid)
            .map { ($0.0,
                    $0.1,
                    $0.2,
                    $0.2.transferByCoreOrCreate (tid, create: true)!)
        }
    }

    /// The index of this system.  Set by `System.systemExtend()`
    var index: Int32 = 0

    var systemContext: BRCryptoClientContext? {
        let index = Int(self.index)
        return UnsafeMutableRawPointer (bitPattern: index)
    }

    public static func create (client: SystemClient,
                               listener: SystemListener,
                               account: Account,
                               onMainnet: Bool,
                               path: String,
                               listenerQueue: DispatchQueue? = nil) -> System {
        return System (client: client,
                       listener: listener,
                       account: account,
                       onMainnet: onMainnet,
                       path: path,
                       listenerQueue: listenerQueue)
    }

    static func destroy (system: System) {
        // Stop all callbacks.  This might be inconsistent with 'deleted' events.
        System.systemRemove (index: system.index)

        // Disconnect all wallet managers
        system.pause ()

        // Stop all the wallet managers.
        system.managers.forEach { $0.stop() }

        // Stop event handling, etc
        cryptoSystemStop (system.core)
    }

    ///
    /// Cease use of `system` and remove (aka 'wipe') its persistent storage.  Caution is highly
    /// warranted; none of the System's references, be they Wallet Managers, Wallets, Transfers, etc
    /// should be *touched* once the system is wiped.
    ///
    /// - Note: This function blocks until completed.  Be sure that all references are dereferenced
    ///         *before* invoking this function and remove the reference to `system` after this
    ///         returns.
    ///
    /// - Parameter system: the system to wipe
    ///
    public static func wipe (system: System) {
        // Save the path to the persistent storage
        let storagePath = system.path;

        // Destroy the system.
        destroy (system: system)

        // Clear out persistent storage
        do {
            if FileManager.default.fileExists(atPath: storagePath) {
                try FileManager.default.removeItem(atPath: storagePath)
            }
        }
        catch let error as NSError {
            print("Error: \(error.localizedDescription)")
        }
    }

    ///
    /// Remove (aka 'wipe') the persistent storage associated with any and all systems located
    /// within `atPath` except for a specified array of systems to preserve.  Generally, this
    /// function should be called on startup after all systems have been created.  When called at
    /// that time, any 'left over' systems will have their persistent storeage wiped.
    ///
    /// - Note: This function will perform no action if `atPath` does not exist or is
    ///         not a directory.
    ///
    /// - Parameter atPath: the file system path where system data is persistently stored
    /// - Parameter systems: the array of systems that shouldn not have their data wiped.
    ///
    public static func wipeAll (atPath: String, except systems: [System]) {
        do {
            try FileManager.default.contentsOfDirectory (atPath: atPath)
                .map     { (path) in atPath + "/" + path }
                .filter  { (path) in !systems.contains { path == $0.path } }
                .forEach { (path) in
                    do {
                        try FileManager.default.removeItem (atPath: path)
                    }
                    catch {}
            }
        }
        catch {}
    }
}

public enum SystemState {
    case created
    case deleted

    init (core: BRCryptoSystemState) {
        switch core {
        case CRYPTO_SYSTEM_STATE_CREATED:
            self = .created

        case CRYPTO_SYSTEM_STATE_DELETED:
            self = .deleted

        default:
            preconditionFailure()
        }
    }
}

public enum SystemEvent {
    case created
    case changed (old: SystemState, new: SystemState)
    case deleted

    /// A network has been added to the system.  This event is generated during `configure` as
    /// each BlockChainDB blockchain is discovered.
    case networkAdded   (network: Network)

    /// During `configure` once all networks have been discovered, this event is generated to
    /// mark the completion of network discovery.  The provided networks are the newly added ones;
    /// if all the known networks are required, use `system.networks`.
    case discoveredNetworks (networks: [Network])

    /// A wallet manager has been added to the system.  WalletMangers are added by the APP
    /// generally as a subset of the Networks and through a call to System.craeteWalletManager.
    case managerAdded (manager: WalletManager)

    init (system: System, core: BRCryptoSystemEvent) {
        switch core.type {
        case CRYPTO_SYSTEM_EVENT_CREATED:
            self = .created

        case CRYPTO_SYSTEM_EVENT_CHANGED:
            self = .changed(old: SystemState (core: core.u.state.old),
                            new: SystemState (core: core.u.state.new))

        case CRYPTO_SYSTEM_EVENT_DELETED:
            self = .deleted

        case CRYPTO_SYSTEM_EVENT_NETWORK_ADDED:
            self = .networkAdded (network: Network (core: core.u.network, take: false))

        case CRYPTO_SYSTEM_EVENT_NETWORK_CHANGED:
            preconditionFailure()
        case CRYPTO_SYSTEM_EVENT_NETWORK_DELETED:
            preconditionFailure()

        case CRYPTO_SYSTEM_EVENT_MANAGER_ADDED:
            self = .managerAdded (manager: WalletManager (core: core.u.manager,
                                                          system: system,
                                                          callbackCoordinator: system.callbackCoordinator,
                                                          take: false))

        case CRYPTO_SYSTEM_EVENT_MANAGER_CHANGED:
            preconditionFailure()
        case CRYPTO_SYSTEM_EVENT_MANAGER_DELETED:
            preconditionFailure()

        default:
            preconditionFailure()
        }
    }
}

///
/// A SystemListener recieves asynchronous events announcing state changes to Networks, to Managers,
/// to Wallets and to Transfers.  This is an application's sole mechanism to learn of asynchronous
/// state changes.  The `SystemListener` is built upon other listeners (for WalletManager, etc) and
/// adds in `func handleSystemEvent(...)`.
///
/// All event handlers will be asynchronously performed on a listener-specific queue.  Handler will
/// be invoked as `queue.async { listener.... (...) }`.  This queue can be serial or parallel as
/// any listener calls back into System are multi-thread protected.  The queue is provided as part
/// of the System initalizer - if a queue is not specified a default one will be provided.
///
/// Note: This must be 'class bound' as System holds a 'weak' reference (for GC reasons).
///
public protocol SystemListener : /* class, */ WalletManagerListener, WalletListener, TransferListener, NetworkListener {
    ///
    /// Handle a System Event
    ///
    /// - Parameters:
    ///   - system: the system
    ///   - event: the event
    ///
    func handleSystemEvent (system: System,
                            event: SystemEvent)
}

/// A Functional Interface for a Handler
public typealias SystemEventHandler = (System, SystemEvent) -> Void

///
/// A SystemCallbackCoordinator coordinates callbacks for non-event based announcement interfaces.
///
internal final class SystemCallbackCoordinator {
    enum Handler {
        case walletFeeEstimate (Wallet.EstimateFeeHandler)
    }

    private var index: Int32 = 0;
    private var handlers: [Int32: Handler] = [:]

    // The queue upon which to invoke handlers.
    private let queue: DispatchQueue

    public typealias Cookie = UnsafeMutableRawPointer

    private func cookieToIndex (_ cookie: Cookie) -> Int32 {
        // Convert `cookie` back to an Int.  The trick is that `bitPattern` cannot be `0`.  So
        // we use `1` when computing the 'distance' and need to add `1` to the result.
        return 1 + Int32(UnsafeMutableRawPointer(bitPattern: 1)!.distance(to: cookie))
    }

    public func addWalletFeeEstimateHandler(_ handler: @escaping Wallet.EstimateFeeHandler) -> Cookie {
        return System.systemQueue.sync (flags: .barrier) {
            index += 1
            handlers[index] = Handler.walletFeeEstimate(handler)
            // A new cookie using `index` as a pointer.
            return UnsafeMutableRawPointer (bitPattern: Int (index))!  // `index` is neve `1`
        }
    }

    private func remWalletFeeEstimateHandler (_ cookie: UnsafeMutableRawPointer) -> Wallet.EstimateFeeHandler? {
        return System.systemQueue.sync (flags: .barrier) {
            return handlers.removeValue (forKey: cookieToIndex(cookie))
                .flatMap {
                    switch $0 {
                    case .walletFeeEstimate (let handler): return handler
                    }
            }
        }
    }

    func handleWalletFeeEstimateSuccess (_ cookie: UnsafeMutableRawPointer, estimate: TransferFeeBasis) {
        if let handler = remWalletFeeEstimateHandler(cookie) {
            queue.async {
                handler (Result.success (estimate))
            }
        }
    }

    func handleWalletFeeEstimateFailure (_ cookie: UnsafeMutableRawPointer, error: Wallet.FeeEstimationError) {
        if let handler = remWalletFeeEstimateHandler(cookie) {
            queue.async {
                handler (Result.failure(error))
            }
        }
    }

    init (queue: DispatchQueue) {
        self.queue = queue
    }
}


extension System {
    internal var cryptoListener: BRCryptoListener {
        // These methods are invoked direclty on a BWM, EWM, or GWM thread.
        return cryptoListenerCreate(
            systemContext,

            // BRCryptoListenerSystemCallback
            { (context, sys, event) in
                precondition (nil != context && nil != sys)
                defer { cryptoSystemGive(sys) }

                guard let system = System.systemExtract(context)
                else { print ("SYS: Event: \(event.type): Missed (sys)"); return }

                system.listener?.handleSystemEvent(system: system,
                                                   event: SystemEvent.init (system: system,
                                                                            core: event))
        },

            // BRCryptoListenerNetworkCallback
            { (context, net, event) in
                precondition (nil != context && nil != net)
                defer { cryptoNetworkGive (net) }

                guard let system = System.systemExtract(context),
                    let network = system.networkBy(core: net!)
                    else { print ("SYS: Event: \(event.type): Missed (net)"); return }

                system.listener?.handleNetworkEvent(system: system,
                                                    network: network,
                                                    event: NetworkEvent.init(core: event))
        },
            // BRCryptoListenerWalletManagerCallback
            { (context, cwm, event) in
                precondition (nil != context  && nil != cwm)
                defer { cryptoWalletManagerGive(cwm) }

                guard let (system, manager) = System.systemExtract (context, cwm)
                    else { print ("SYS: Event: \(event.type): Missed {cwm}"); return }

                if event.type != CRYPTO_WALLET_MANAGER_EVENT_CHANGED {
                    print ("SYS: Event: Manager (\(manager.name)): \(event.type)")
                }
                var walletManagerEvent: WalletManagerEvent? = nil

                switch event.type {
                case CRYPTO_WALLET_MANAGER_EVENT_CREATED:
                    walletManagerEvent = WalletManagerEvent.created

                case CRYPTO_WALLET_MANAGER_EVENT_CHANGED:
                    print ("SYS: Event: Manager (\(manager.name)): \(event.type): {\(WalletManagerState (core: event.u.state.old)) -> \(WalletManagerState (core: event.u.state.new))}")
                    walletManagerEvent = WalletManagerEvent.changed (oldState: WalletManagerState (core: event.u.state.old),
                                                                     newState: WalletManagerState (core: event.u.state.new))

                case CRYPTO_WALLET_MANAGER_EVENT_DELETED:
                    walletManagerEvent = WalletManagerEvent.deleted

                case CRYPTO_WALLET_MANAGER_EVENT_WALLET_ADDED:
                    defer { if let wid = event.u.wallet { cryptoWalletGive (wid) }}
                    guard let wallet = manager.walletBy (core: event.u.wallet)
                        else { print ("SYS: Event: \(event.type): Missed (wallet)"); return }
                    walletManagerEvent = WalletManagerEvent.walletAdded (wallet: wallet)

                case CRYPTO_WALLET_MANAGER_EVENT_WALLET_CHANGED:
                    defer { if let wid = event.u.wallet { cryptoWalletGive (wid) }}
                    guard let wallet = manager.walletBy (core: event.u.wallet)
                        else { print ("SYS: Event: \(event.type): Missed (wallet)"); return }
                    walletManagerEvent = WalletManagerEvent.walletChanged(wallet: wallet)

                case CRYPTO_WALLET_MANAGER_EVENT_WALLET_DELETED:
                    defer { if let wid = event.u.wallet { cryptoWalletGive (wid) }}
                    guard let wallet = manager.walletBy (core: event.u.wallet)
                        else { print ("SYS: Event: \(event.type): Missed (wallet)"); return }
                    walletManagerEvent = WalletManagerEvent.walletDeleted(wallet: wallet)

                case CRYPTO_WALLET_MANAGER_EVENT_SYNC_STARTED:
                    walletManagerEvent = WalletManagerEvent.syncStarted

                case CRYPTO_WALLET_MANAGER_EVENT_SYNC_CONTINUES:
                    let timestamp: Date? = (0 == event.u.syncContinues.timestamp // NO_CRYPTO_TIMESTAMP
                        ? nil
                        : Date (timeIntervalSince1970: TimeInterval(event.u.syncContinues.timestamp)))

                    walletManagerEvent = WalletManagerEvent.syncProgress (
                        timestamp: timestamp,
                        percentComplete: event.u.syncContinues.percentComplete)

                case CRYPTO_WALLET_MANAGER_EVENT_SYNC_STOPPED:
                    let reason = WalletManagerSyncStoppedReason(core: event.u.syncStopped.reason)
                    walletManagerEvent = WalletManagerEvent.syncEnded(reason: reason)

                case CRYPTO_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED:
                    let depth = WalletManagerSyncDepth(core: event.u.syncRecommended.depth)
                    walletManagerEvent = WalletManagerEvent.syncRecommended(depth: depth)

                case CRYPTO_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED:
                    walletManagerEvent = WalletManagerEvent.blockUpdated(height: event.u.blockHeight)

                default: preconditionFailure()
                }

                walletManagerEvent.map { (event) in
                    system.listener?.handleManagerEvent (system: system,
                                                         manager: manager,
                                                         event: event)
                }
        },

            // BRCryptoListenerWalletCallback
            { (context, cwm, wid, event) in
                precondition (nil != context  && nil != cwm && nil != wid)
                defer { cryptoWalletManagerGive(cwm); cryptoWalletGive(wid) }

                guard let (system, manager, wallet) = System.systemExtract (context, cwm, wid)
                    else { print ("SYS: Event: \(event.type): Missed {cwm, wid}"); return }

                if event.type != CRYPTO_WALLET_EVENT_CHANGED {
                    print ("SYS: Event: Wallet (\(wallet.name)): \(event.type)")
                }

                var walletEvent: WalletEvent? = nil

                switch event.type {
                case CRYPTO_WALLET_EVENT_CREATED:
                    walletEvent = WalletEvent.created

                case CRYPTO_WALLET_EVENT_CHANGED:
                    print ("SYS: Event: Wallet (\(manager.name)): \(event.type): {\(WalletState (core: event.u.state.old)) -> \(WalletState (core: event.u.state.new))}")
                    walletEvent = WalletEvent.changed (oldState: WalletState (core: event.u.state.old),
                                                       newState: WalletState (core: event.u.state.new))

                case CRYPTO_WALLET_EVENT_DELETED:
                    walletEvent = WalletEvent.deleted

                case CRYPTO_WALLET_EVENT_TRANSFER_ADDED:
                    defer { if let tid = event.u.transfer { cryptoTransferGive(tid) }}
                    guard let transfer = wallet.transferBy (core: event.u.transfer)
                        else { print ("SYS: Event: \(event.type): Missed (transfer)"); return }
                    walletEvent = WalletEvent.transferAdded (transfer: transfer)

                case CRYPTO_WALLET_EVENT_TRANSFER_CHANGED:
                    defer { if let tid = event.u.transfer { cryptoTransferGive(tid) }}
                    guard let transfer = wallet.transferBy (core: event.u.transfer)
                        else { print ("SYS: Event: \(event.type): Missed (transfer)"); return }
                    walletEvent = WalletEvent.transferChanged (transfer: transfer)

                case CRYPTO_WALLET_EVENT_TRANSFER_SUBMITTED:
                    defer { if let tid = event.u.transfer { cryptoTransferGive(tid) }}
                    guard let transfer = wallet.transferBy (core: event.u.transfer)
                        else { print ("SYS: Event: \(event.type): Missed (transfer)"); return }
                    walletEvent = WalletEvent.transferSubmitted (transfer: transfer, success: true)

                case CRYPTO_WALLET_EVENT_TRANSFER_DELETED:
                    defer { if let tid = event.u.transfer { cryptoTransferGive(tid) }}
                    guard let transfer = wallet.transferBy (core: event.u.transfer)
                        else { print ("SYS: Event: \(event.type): Missed (transfer)"); return }
                    walletEvent = WalletEvent.transferDeleted (transfer: transfer)

                case CRYPTO_WALLET_EVENT_BALANCE_UPDATED:
                    let amount = Amount (core: event.u.balanceUpdated.amount,
                                         take: false)
                    walletEvent = WalletEvent.balanceUpdated(amount: amount)

                case CRYPTO_WALLET_EVENT_FEE_BASIS_UPDATED:
                    let feeBasis = TransferFeeBasis (core: event.u.feeBasisUpdated.basis, take: false)
                    walletEvent = WalletEvent.feeBasisUpdated(feeBasis: feeBasis)

                case CRYPTO_WALLET_EVENT_FEE_BASIS_ESTIMATED:
                    let cookie = event.u.feeBasisEstimated.cookie!
                    if CRYPTO_SUCCESS == event.u.feeBasisEstimated.status,
                        let feeBasis = event.u.feeBasisEstimated.basis
                            .map ({ TransferFeeBasis (core: $0, take: false) }) {
                        system.callbackCoordinator.handleWalletFeeEstimateSuccess (cookie, estimate: feeBasis)
                    }
                    else {
                        let feeError = Wallet.FeeEstimationError.fromStatus(event.u.feeBasisEstimated.status)
                        system.callbackCoordinator.handleWalletFeeEstimateFailure (cookie, error: feeError)
                    }
                default: preconditionFailure()
                }

                walletEvent.map { (event) in
                    system.listener?.handleWalletEvent (system: manager.system,
                                                        manager: manager,
                                                        wallet: wallet,
                                                        event: event)
                }
        },

            // BRCryptoListenerWalletCallback
            { (context, cwm, wid, tid, event) in
                precondition (nil != context  && nil != cwm && nil != wid && nil != tid)
                defer { cryptoWalletManagerGive(cwm); cryptoWalletGive(wid); cryptoTransferGive(tid) }

                guard let (system, manager, wallet, transfer) = System.systemExtract (context, cwm, wid, tid)
                    else { print ("SYS: Event: \(event.type): Missed {cwm, wid, tid}"); return }

                if event.type != CRYPTO_TRANSFER_EVENT_CHANGED {
                    print ("SYS: Event: Transfer (\(wallet.name) @ \(transfer.hash?.description ?? "pending")): \(event.type)")
                }

                var transferEvent: TransferEvent? = nil

                switch (event.type) {
                case CRYPTO_TRANSFER_EVENT_CREATED:
                    transferEvent = TransferEvent.created

                case CRYPTO_TRANSFER_EVENT_CHANGED:
                    print ("SYS: Event: Transfer (\(wallet.name)): \(event.type): {\(TransferState (core: event.u.state.old)) -> \(TransferState (core: event.u.state.new))}")

                    // The event.u.state.{old,new} references to BRCryptoTransferState are 'passed'
                    // to the TransferState initializer.
                    transferEvent = TransferEvent.changed (old: TransferState (core: event.u.state.old),
                                                           new: TransferState (core: event.u.state.new))

                case CRYPTO_TRANSFER_EVENT_DELETED:
                    transferEvent = TransferEvent.deleted

                default: preconditionFailure()
                }

                transferEvent.map { (event) in
                    system.listener?.handleTransferEvent (system: system,
                                                          manager: manager,
                                                          wallet: wallet,
                                                          transfer: transfer,
                                                          event: event)
                }
        })
    }
}

extension System {
    private static func cleanup (_ message: String,
                                 cwm: BRCryptoWalletManager? = nil,
                                 wid: BRCryptoWallet? = nil,
                                 tid: BRCryptoTransfer? = nil) -> Void {
        print (message)
        cwm.map { cryptoWalletManagerGive ($0) }
        wid.map { cryptoWalletGive ($0) }
        tid.map { cryptoTransferGive ($0) }
    }

    private static func getTransferStatus (_ modelStatus: String) -> BRCryptoTransferStateType {
        switch modelStatus {
        case "confirmed": return CRYPTO_TRANSFER_STATE_INCLUDED
        case "submitted", "reverted": return CRYPTO_TRANSFER_STATE_SUBMITTED
        case "failed", "rejected":    return CRYPTO_TRANSFER_STATE_ERRORED
        default: preconditionFailure()
        }
    }
}

extension System {
    private static func mergeTransfers (_ transaction: SystemClient.Transaction, with addresses: [String])
        -> [(transfer: SystemClient.Transfer, fee: SystemClient.Amount?)] {
            // Only consider transfers w/ `address`
            var transfers = transaction.transfers.filter {
                ($0.source.map { addresses.caseInsensitiveContains($0) } ?? false) ||
                    ($0.target.map { addresses.caseInsensitiveContains($0) } ?? false)
            }

            // Note for later: all transfers have a unique id

            let partition = transfers.partition { "__fee__" != $0.target }
            switch (0..<partition).count {
            case 0:
                // There is no "__fee__" entry
                return transfers[partition...]
                    .map { (transfer: $0, fee: nil) }

            case 1:
                // There is a single "__fee__" entry
                let transferWithFee = transfers[..<partition][0]

                // We may or may not have a non-fee transfer matching `transferWithFee`.  We
                // may or may not have more than one non-fee transfers matching `transferWithFee`

                // Find the first of the non-fee transfers matching `transferWithFee`.
                let transferMatchingFee = transfers[partition...]
                    .first {
                        $0.transactionId == transferWithFee.transactionId &&
                            $0.source == transferWithFee.source &&
                            $0.amount.currency == transferWithFee.amount.currency
                }

                // We must have a transferMatchingFee; if we don't add one
                let transfers = transfers[partition...] +
                    (nil != transferMatchingFee
                        ? []
                        : [(id: transferWithFee.id,
                            source: transferWithFee.source,
                            target: "unknown",
                            amount: (currency: transferWithFee.amount.currency,
                                     value: "0"),
                            acknowledgements: transferWithFee.acknowledgements,
                            index: transferWithFee.index,
                            transactionId: transferWithFee.transactionId,
                            blockchainId: transferWithFee.blockchainId,
                            metaData: transferWithFee.metaData)])

                // Hold the Id for the transfer that we'll add a fee to.
                let transferForFeeId = transferMatchingFee.map { $0.id } ?? transferWithFee.id

                // Map transfers adding the fee to the `transferforFeeId`
                return transfers
                    .map { (transfer: $0,
                            fee: ($0.id == transferForFeeId ? transferWithFee.amount : nil))
                }

            default:
                // There is more than one "__fee__" entry
                preconditionFailure()
            }
    }
}

extension System {
    internal static func makeAddresses (_ addresses: UnsafeMutablePointer<UnsafePointer<Int8>?>?,
                                        _ addressesCount: Int) -> [String] {
        var cAddresses = addresses!
        var addresses:[String] = Array (repeating: "", count: addressesCount)
        for index in 0..<addressesCount {
            addresses[index] = asUTF8String (cAddresses.pointee!)
            cAddresses = cAddresses.advanced(by: 1)
        }
        return addresses
    }

    internal static func makeTransactionBundle (_ model: SystemClient.Transaction) -> BRCryptoClientTransactionBundle {
        let timestamp = model.timestamp.map { $0.asUnixTimestamp } ?? 0
        let height    = model.blockHeight ?? 0
        let status    = System.getTransferStatus (model.status)

        var data = model.raw!
        let bytesCount = data.count
        return data.withUnsafeMutableBytes { (bytes: UnsafeMutableRawBufferPointer) -> BRCryptoClientTransactionBundle in
            let bytesAsUInt8 = bytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
            return cryptoClientTransactionBundleCreate (status, bytesAsUInt8, bytesCount, timestamp, height)
        }
    }

    internal static func makeTransferBundles (_ transaction: SystemClient.Transaction, addresses:[String]) -> [BRCryptoClientTransferBundle] {
        let blockTimestamp = transaction.timestamp.map { $0.asUnixTimestamp } ?? 0
        let blockHeight    = transaction.blockHeight ?? BLOCK_HEIGHT_UNBOUND
        let blockConfirmations = transaction.confirmations ?? 0
        let blockTransactionIndex = transaction.index ?? 0
        let blockHash             = transaction.blockHash
        let status    = System.getTransferStatus (transaction.status)

        return System.mergeTransfers (transaction, with: addresses)
            .map { (arg: (transfer: SystemClient.Transfer, fee: SystemClient.Amount?)) in
                let (transfer, fee) = arg

                let metaData = (transaction.metaData ?? [:]).merging (transfer.metaData ?? [:]) { (cur, new) in new }

                var metaKeysPtr = Array(metaData.keys)
                    .map { UnsafePointer<Int8>(strdup($0)) }
                defer { metaKeysPtr.forEach { cryptoMemoryFree (UnsafeMutablePointer(mutating: $0)) } }

                var metaValsPtr = Array(metaData.values)
                    .map { UnsafePointer<Int8>(strdup($0)) }
                defer { metaValsPtr.forEach { cryptoMemoryFree (UnsafeMutablePointer(mutating: $0)) } }

                return cryptoClientTransferBundleCreate (status,
                                                         transaction.hash,
                                                         transfer.id,
                                                         transfer.source,
                                                         transfer.target,
                                                         transfer.amount.value,
                                                         transfer.amount.currency,
                                                         fee.map { $0.value },
                                                         blockTimestamp,
                                                         blockHeight,
                                                         blockConfirmations,
                                                         blockTransactionIndex,
                                                         blockHash,
                                                         metaKeysPtr.count,
                                                         &metaKeysPtr,
                                                         &metaValsPtr)
        }
    }

    internal var cryptoClient: BRCryptoClient {
        return BRCryptoClient (
            context: systemContext,

            funcGetBlockNumber: { (context, cwm, sid) in
                precondition (nil != context  && nil != cwm)

                guard let (_, manager) = System.systemExtract (context, cwm)
                    else { System.cleanup("SYS: GetBlockNumber: Missed {cwm}", cwm: cwm); return }
                print ("SYS: GetBlockNumber")

                manager.client.getBlockchain (blockchainId: manager.network.uids) {
                    (res: Result<SystemClient.Blockchain, SystemClientError>) in
                    defer { cryptoWalletManagerGive (cwm!) }
                    res.resolve (
                        success: {        cwmAnnounceBlockNumber (cwm, sid, CRYPTO_TRUE,  $0.blockHeight ?? 0) },
                        failure: { (_) in cwmAnnounceBlockNumber (cwm, sid, CRYPTO_FALSE, 0) })
                }},

            funcGetTransactions: { (context, cwm, sid, addresses, addressesCount, begBlockNumber, endBlockNumber) in
                precondition (nil != context  && nil != cwm)

                guard let (_, manager) = System.systemExtract (context, cwm)
                    else { System.cleanup ("SYS: BTC: GetTransactions: Missed {cwm}", cwm: cwm); return }
                print ("SYS: BTC: GetTransactions: Blocks: {\(begBlockNumber), \(endBlockNumber)}")

                let addresses = System.makeAddresses (addresses, addressesCount)

                manager.client.getTransactions (blockchainId: manager.network.uids,
                                               addresses: addresses,
                                               begBlockNumber: (begBlockNumber == BLOCK_HEIGHT_UNBOUND_VALUE ? nil : begBlockNumber),
                                               endBlockNumber: (endBlockNumber == BLOCK_HEIGHT_UNBOUND_VALUE ? nil : endBlockNumber),
                                               includeRaw: true,
                                               includeTransfers: false) {
                                                (res: Result<[SystemClient.Transaction], SystemClientError>) in
                                                defer { cryptoWalletManagerGive (cwm!) }
                                                res.resolve(
                                                    success: {
                                                        var bundles: [BRCryptoClientTransactionBundle?] = $0.map { System.makeTransactionBundle ($0) }
                                                            //.map { UnsafeMutablePointer<BRCryptoClientTransactionBundle?> ($0) }
                                                        cwmAnnounceTransactions (cwm, sid, CRYPTO_TRUE,  &bundles, bundles.count) },
                                                    failure: { (_) in
                                                        cwmAnnounceTransactions (cwm, sid, CRYPTO_FALSE, nil, 0) })
                }},

            funcGetTransfers: { (context, cwm, sid, addresses, addressesCount, begBlockNumber, endBlockNumber) in
                precondition (nil != context  && nil != cwm)

                guard let (_, manager) = System.systemExtract (context, cwm)
                    else { print ("SYS: GetTransfers: Missed {cwm}"); return }
                print ("SYS: GetTransfers: Blocks: {\(begBlockNumber), \(endBlockNumber)}")

                let addresses = System.makeAddresses (addresses, addressesCount)

                manager.client.getTransactions (blockchainId: manager.network.uids,
                                               addresses: addresses,
                                               begBlockNumber: (begBlockNumber == BLOCK_HEIGHT_UNBOUND_VALUE ? nil : begBlockNumber),
                                               endBlockNumber: (endBlockNumber == BLOCK_HEIGHT_UNBOUND_VALUE ? nil : endBlockNumber),
                                               includeRaw: false,
                                               includeTransfers: true) {
                                                (res: Result<[SystemClient.Transaction], SystemClientError>) in
                                                defer { cryptoWalletManagerGive(cwm) }
                                                res.resolve(
                                                    success: {
                                                        var bundles: [BRCryptoClientTransferBundle?]  = $0.flatMap { System.makeTransferBundles ($0, addresses: addresses) }
                                                        cwmAnnounceTransfers (cwm, sid, CRYPTO_TRUE,  &bundles, bundles.count) },
                                                    failure: { (_) in
                                                        cwmAnnounceTransfers (cwm, sid, CRYPTO_FALSE, nil,     0) })
                }},

            funcSubmitTransaction: { (context, cwm, sid, transactionBytes, transactionBytesLength, hashAsHex) in
                precondition (nil != context  && nil != cwm)

                guard let (_, manager) = System.systemExtract (context, cwm)
                    else { System.cleanup  ("SYS: SubmitTransaction: Missed {cwm}", cwm: cwm); return }
                print ("SYS: SubmitTransaction")

                let hash = asUTF8String (hashAsHex!)
                let data = Data (bytes: transactionBytes!, count: transactionBytesLength)

                manager.client.createTransaction (blockchainId: manager.network.uids, hashAsHex: hash, transaction: data) {
                    (res: Result<Void, SystemClientError>) in
                    defer { cryptoWalletManagerGive (cwm!) }
                    res.resolve(
                        success: { (_) in
                            cwmAnnounceSubmitTransfer (cwm, sid, CRYPTO_TRUE,  hash) },
                        failure: { (e) in
                            print ("SYS: SubmitTransaction: Error: \(e)")
                            cwmAnnounceSubmitTransfer (cwm, sid, CRYPTO_FALSE, hash) })
                }},

            funcEstimateTransactionFee: { (context, cwm, sid, transactionBytes, transactionBytesLength, hashAsHex) in
                precondition (nil != context  && nil != cwm)

                guard let (_, manager) = System.systemExtract (context, cwm)
                    else { System.cleanup  ("SYS: EstimateTransactionFee: Missed {cwm}", cwm: cwm); return }
                print ("SYS: EstimateTransactionFee")

                let hash = asUTF8String (hashAsHex!)
                let data = Data (bytes: transactionBytes!, count: transactionBytesLength)

                manager.client.estimateTransactionFee (blockchainId: manager.network.uids, hashAsHex: hash, transaction: data) {
                    (res: Result<SystemClient.TransactionFee, SystemClientError>) in
                    defer { cryptoWalletManagerGive (cwm!) }
                    res.resolve(
                        success: {
                            let properties = $0.properties ?? [:]
                            var metaKeysPtr = Array(properties.keys)
                                .map { UnsafePointer<Int8>(strdup($0)) }
                            defer { metaKeysPtr.forEach { cryptoMemoryFree (UnsafeMutablePointer(mutating: $0)) } }

                            var metaValsPtr = Array(properties.values)
                                .map { UnsafePointer<Int8>(strdup($0)) }
                            defer { metaValsPtr.forEach { cryptoMemoryFree (UnsafeMutablePointer(mutating: $0)) } }
                            
                            cwmAnnounceEstimateTransactionFee (cwm,
                                                               sid,
                                                               CRYPTO_TRUE,
                                                               hash,
                                                               $0.costUnits,
                                                               metaKeysPtr.count,
                                                               &metaKeysPtr,
                                                               &metaValsPtr)
                            
                    },
                        failure: { (e) in
                            print ("SYS: EstimateTransactionFee: Error: \(e)")
                            cwmAnnounceEstimateTransactionFee (cwm, sid, CRYPTO_FALSE, hash, 0, 0, nil, nil) })
                }}
        )
    }
}

/// Support for Persistent Storage Migration.
///
/// Allow prior App version to migrate their SQLite database representations of BTC/BTC
/// transations, blocks and peers into 'Generic Crypto' - where these entities are persistently
/// stored in the file system (by BRFileSystem).
///
extension System {

    ///
    /// A Blob of Transaction Data
    ///
    /// - btc:
    ///
    public typealias TransactionBlobBTCTuple = (
        bytes: [UInt8],
        blockHeight: UInt32,
        timestamp: UInt32 // time interval since unix epoch (including '0'
    )

    public enum TransactionBlob {
        case btc (TransactionBlobBTCTuple)
    }

    ///
    /// A BlockHash is 32-bytes of UInt8 data
    ///
    public typealias BlockHash = [UInt8]

    /// Validate `BlockHash`
    private static func validateBlockHash (_ hash: BlockHash) -> Bool {
        return 32 == hash.count
    }

    ///
    /// A Blob of Block Data
    ///
    /// - btc:
    ///
    public typealias BlockBlobBTCTuple = (
        hash: BlockHash,
        height: UInt32,
        nonce: UInt32,
        target: UInt32,
        txCount: UInt32,
        version: UInt32,
        timestamp: UInt32?,
        flags: [UInt8],
        hashes: [BlockHash],
        merkleRoot: BlockHash,
        prevBlock: BlockHash
    )

    public enum BlockBlob {
        case btc (BlockBlobBTCTuple)
    }

    ///
    /// A Blob of Peer Data
    ///
    /// - btc:
    ///
    public typealias PeerBlobBTCTuple = (
        address: UInt32,  // UInt128 { .u32 = { 0, 0, 0xffff, <address> }}
        port: UInt16,
        services: UInt64,
        timestamp: UInt32?
    )

    public enum PeerBlob {
        case btc (PeerBlobBTCTuple)
    }

    ///
    /// Migrate Errors
    ///
    enum MigrateError: Error {
        /// Migrate does not apply to this network
        case invalid

        /// Migrate couldn't access the file system
        case create

        /// Migrate failed to parse or to save a transaction
        case transaction

        /// Migrate failed to parse or to save a block
        case block

        /// Migrate failed to parse or to save a peer.
        case peer
    }

    ///
    /// Migrate the storage for a network given transaction, block and peer blobs.  The provided
    /// blobs must be consistent with `network`.  For exmaple, if `network` represents BTC or BCH
    /// then the blobs must be of type `.btc`; otherwise a MigrateError is thrown.
    ///
    /// - Parameters:
    ///   - network:
    ///   - transactionBlobs:
    ///   - blockBlobs:
    ///   - peerBlobs:
    ///
    /// - Throws: MigrateError
    ///
    public func migrateStorage (network: Network,
                                transactionBlobs: [TransactionBlob],
                                blockBlobs: [BlockBlob],
                                peerBlobs: [PeerBlob]) throws {
        guard migrateRequired (network: network)
            else { throw MigrateError.invalid }

        switch cryptoNetworkGetType (network.core) {
        case CRYPTO_NETWORK_TYPE_BTC,
             CRYPTO_NETWORK_TYPE_BCH:
            try migrateStorageAsBTC(network: network,
                                    transactionBlobs: transactionBlobs,
                                    blockBlobs: blockBlobs,
                                    peerBlobs: peerBlobs)
        default:
            throw MigrateError.invalid
        }
    }

    ///
    /// Migrate storage for BTC
    ///
    private func migrateStorageAsBTC (network: Network,
                                      transactionBlobs: [TransactionBlob],
                                      blockBlobs: [BlockBlob],
                                      peerBlobs: [PeerBlob]) throws {

        guard let migrator = cryptoWalletMigratorCreate (network.core, path)
            else { throw MigrateError.create }
        defer { cryptoWalletMigratorRelease (migrator) }

        try transactionBlobs.forEach { (blob: TransactionBlob) in
            guard case let .btc (blob) = blob
                else { throw MigrateError.transaction }

            var bytes = blob.bytes
            let status = cryptoWalletMigratorHandleTransactionAsBTC (migrator,
                                                                     &bytes, bytes.count,
                                                                     blob.blockHeight,
                                                                     blob.timestamp)
            if status.type != CRYPTO_WALLET_MIGRATOR_SUCCESS {
                throw MigrateError.transaction
            }
        }

        try blockBlobs.forEach { (blob: BlockBlob) in
            guard case let .btc (blob) = blob
                else { throw MigrateError.block }

            // On a `nil` timestamp, by definition skip out, don't migrate this blob
            guard nil != blob.timestamp
                else { return }

            guard blob.hashes.allSatisfy (System.validateBlockHash(_:)),
                System.validateBlockHash(blob.hash),
                System.validateBlockHash(blob.merkleRoot),
                System.validateBlockHash(blob.prevBlock)
                else { throw MigrateError.block }

            var flags  = blob.flags
            var hashes = blob.hashes.flatMap { $0 }  // [[UInt8 ...] ...] => [UInt8 ... ...]
            let hashesCount = blob.hashes.count

            let hash: BRCryptoData32 = blob.hash.withUnsafeBytes { $0.load (as: BRCryptoData32.self) }
            let merkleRoot: BRCryptoData32 = blob.merkleRoot.withUnsafeBytes { $0.load (as: BRCryptoData32.self) }
            let prevBlock:  BRCryptoData32 = blob.prevBlock.withUnsafeBytes  { $0.load (as: BRCryptoData32.self) }

            try hashes.withUnsafeMutableBytes { (hashesBytes: UnsafeMutableRawBufferPointer) -> Void in
                let hashesAddr = hashesBytes.baseAddress?.assumingMemoryBound(to: BRCryptoData32.self)
                let status = cryptoWalletMigratorHandleBlockAsBTC (migrator,
                                                                   hash,
                                                                   blob.height,
                                                                   blob.nonce,
                                                                   blob.target,
                                                                   blob.txCount,
                                                                   blob.version,
                                                                   blob.timestamp!,
                                                                   &flags, flags.count,
                                                                   hashesAddr, hashesCount,
                                                                   merkleRoot,
                                                                   prevBlock)
                if status.type != CRYPTO_WALLET_MIGRATOR_SUCCESS {
                    throw MigrateError.block
                }
            }
        }

        try peerBlobs.forEach { (blob: PeerBlob) in
            guard case let .btc (blob) = blob
                else { throw MigrateError.peer }

            // On a `nil` timestamp, by definition skip out, don't migrate this blob
            guard nil != blob.timestamp
                else { return }

            let status = cryptoWalletMigratorHandlePeerAsBTC (migrator,
                                                              blob.address,
                                                              blob.port,
                                                              blob.services,
                                                              blob.timestamp!)

            if status.type != CRYPTO_WALLET_MIGRATOR_SUCCESS {
                throw MigrateError.peer
            }
        }
    }

    ///
    /// If migration is required, return the currency code; otherwise, return nil.
    ///
    /// - Note: it is not an error not to migrate.
    ///
    /// - Parameter network:
    ///
    /// - Returns: The currency code or nil
    ///
    public func migrateRequired (network: Network) -> Bool {
        switch cryptoNetworkGetType (network.core) {
        case CRYPTO_NETWORK_TYPE_BTC,
             CRYPTO_NETWORK_TYPE_BCH:
            return true
        default:
            return false
        }
    }

    /// Testing

    ///
    /// Convert `transfer` to a `TransactionBlob`.
    ///
    /// - Parameter transfer:
    ///
    /// - Returns: A `TransactionBlob` or nil (if the transfer's network does not require
    ///     migration, such as for an ETH network).
    ///
    internal func asBlob (transfer: Transfer) -> TransactionBlob? {
        let network = transfer.manager.network
        guard migrateRequired(network: network)
            else { return nil }

        switch cryptoNetworkGetType (network.core) {
        case CRYPTO_NETWORK_TYPE_BTC,
             CRYPTO_NETWORK_TYPE_BCH:
            var blockHeight: UInt32 = 0
            var timestamp:   UInt32 = 0
            var bytesCount:  size_t = 0
            var bytes: UnsafeMutablePointer<UInt8>? = nil
            defer { if nil != bytes { cryptoMemoryFree (bytes) }}

            cryptoTransferExtractBlobAsBTC (transfer.core,
                                            &bytes, &bytesCount,
                                            &blockHeight,
                                            &timestamp)

            return TransactionBlob.btc ((bytes: UnsafeMutableBufferPointer<UInt8> (start: bytes, count: bytesCount).map { $0 },
                                         blockHeight: blockHeight,
                                         timestamp: timestamp))
        default:
            return nil
        }
    }
}

extension BRCryptoTransferEventType: CustomStringConvertible {
    public var description: String {
        return asUTF8String (cryptoTransferEventTypeString(self))
    }
}

extension BRCryptoWalletEventType: CustomStringConvertible {
    public var description: String {
        return asUTF8String (cryptoWalletEventTypeString (self))
    }
}

extension BRCryptoWalletManagerEventType: CustomStringConvertible {
    public var description: String {
        return asUTF8String (cryptoWalletManagerEventTypeString (self))
    }
}

