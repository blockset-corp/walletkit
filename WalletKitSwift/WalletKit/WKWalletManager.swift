//
//  WKWalletManager.swift
//  WalletKit
//
//  Created by Ed Gamble on 3/27/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
import Foundation // Data
import WalletKitCore

///
/// A WallettManager manages one or more wallets one of which is designated the `primaryWallet`.
/// (For example, an EthereumWalletManager will manage an ETH wallet and one wallet for each
/// ERC20Token; the ETH wallet will be the primaryWallet.  A BitcoinWalletManager manages one
/// and only one wallet holding BTC.).
///
/// At least conceptually, a WalletManager is an 'Active Object' (whereas Transfer and Wallet are
/// 'Passive Objects'
///
public final class WalletManager: Equatable, CustomStringConvertible {

    /// The Core representation
    internal private(set) var core: WKWalletManager! = nil

    internal let callbackCoordinator: SystemCallbackCoordinator

    /// The owning system
    public unowned let system: System

    /// The account
    public let account: Account

    /// The network
    public let network: Network

    /// The client
    internal let client: SystemClient

    /// The default unit - as the networks default unit
    internal let unit: Unit

    /// The mode determines how the manager manages the account and wallets on network
    public var mode: WalletManagerMode {
        get { return WalletManagerMode (core: wkWalletManagerGetMode (core)) }
        set {
            assert (network.supportsMode(newValue))
            wkWalletManagerSetMode (core, newValue.core)
        }
    }

    /// The file-system path to use for persistent storage.
    public let path: String

    /// The current state
    public var state: WalletManagerState {
        return WalletManagerState (core: wkWalletManagerGetState (core))
    }

    /// The current network block height
    internal var height: UInt64 {
        return network.height
    }

    /// The primaryWallet - holds the network's currency - this is typically the wallet where
    /// fees are applied which may or may not differ from the specific wallet used for a
    /// transfer (like BRD transfer => ETH fee)
    public lazy var primaryWallet: Wallet = {
        // Find a preexisting wallet (unlikely) or create one.
        let coreWallet = wkWalletManagerGetWallet(core)!
        return Wallet (core: coreWallet,
                       manager: self,
                       callbackCoordinator: callbackCoordinator,
                       take: false)
    }()

    ///
    /// Ensure that a wallet for currency exists.  If the wallet already exists, it is returned.
    /// If the wallet needs to be created then `nil` is returned and a series of events will
    /// occur - notably WalletEvent.created and WalletManagerEvent.walletAdded if the wallet is
    /// created
    ///
    /// - Note: There is a precondition on `currency` being one in the managers' network
    ///
    /// - Parameter currency:
    /// - Returns: The wallet for currency if it already exists, othersise `nil`
    ///
    public func registerWalletFor (currency: Currency) -> Wallet? {
        precondition (network.hasCurrency(currency))
        return wkWalletManagerCreateWallet (core, currency.core)
            .map { Wallet (core: $0,
                           manager: self,
                           callbackCoordinator: callbackCoordinator,
                           take: false)
        }
    }

    //    public func unregisterWalletFor (currency: Currency) {
    //        wallets
    //            .first { $0.currency == currency }
    //            .map { unregisterWallet($0) }
    //    }
    //
    //    public func unregisterWallet (_ wallet: Wallet) {
    //    }

    /// The managed wallets - often will just be [primaryWallet]
    public var wallets: [Wallet] {
        let _ = system.listener

        var walletsCount: size_t = 0
        let walletsPtr = wkWalletManagerGetWallets(core, &walletsCount);
        defer { if let ptr = walletsPtr { free (ptr) } }

        let wallets: [WKWallet] = walletsPtr?.withMemoryRebound(to: WKWallet.self, capacity: walletsCount) {
            Array(UnsafeBufferPointer (start: $0, count: walletsCount))
            } ?? []

        return wallets
            .map { Wallet (core: $0,
                           manager: self,
                           callbackCoordinator: callbackCoordinator,
                           take: false) }
    }

    ///
    /// Find a wallet by `impl`
    ///
    /// - Parameter impl: the impl
    /// - Returns: The wallet, if found
    ///
    internal func walletBy (core: WKWallet) -> Wallet? {
        return (WK_FALSE == wkWalletManagerHasWallet (self.core, core)
            ? nil
            : Wallet (core: core,
                      manager: self,
                      callbackCoordinator: callbackCoordinator,
                      take: true))
    }

    internal func walletByCoreOrCreate (_ core: WKWallet,
                                        create: Bool = false) -> Wallet? {
        return walletBy (core: core) ??
            (!create
                ? nil
                : Wallet (core: core,
                          manager: self,
                          callbackCoordinator: callbackCoordinator,
                          take: true))
    }

    /// The default network fee.
    public var defaultNetworkFee: NetworkFee

    /// The address scheme to use
    public var addressScheme: AddressScheme {
        get { return AddressScheme (core: wkWalletManagerGetAddressScheme (core)) }
        set {
            assert (network.supportsAddressScheme(newValue))
            wkWalletManagerSetAddressScheme (core, newValue.core)
        }
    }

    ///
    /// Connect to the network and begin managing wallets.
    ///
    /// - Parameter peer: An optional NetworkPeer to use on the P2P network.  It is unusual to
    ///     provide a peer as P2P networks will dynamically discover suitable peers.
    ///
    /// - Note: If peer is provided, there is a precondition on the networks matching.
    ///
    public func connect (using peer: NetworkPeer? = nil) {
        precondition (peer == nil || peer!.network == network)
        wkWalletManagerConnect (core, peer?.core)
    }

    /// Disconnect from the network.
    public func disconnect () {
        wkWalletManagerDisconnect (core)
    }

    internal func stop () {
        wkWalletManagerStop (core);
    }
    
    public func sync () {
        wkWalletManagerSync (core)
    }

    public func syncToDepth (depth: WalletManagerSyncDepth) {
        wkWalletManagerSyncToDepth (core, depth.core)
    }

    internal func sign (transfer: Transfer, paperKey: String) -> Bool {
        return WK_TRUE == wkWalletManagerSign (core,
                                                       transfer.wallet.core,
                                                       transfer.core,
                                                       paperKey)
    }

    public func submit (transfer: Transfer, paperKey: String) {
        wkWalletManagerSubmit (core,
                                   transfer.wallet.core,
                                   transfer.core,
                                   paperKey)
    }

    internal func submit (transfer: Transfer, key: Key) {
        wkWalletManagerSubmitForKey(core,
                                        transfer.wallet.core,
                                        transfer.core,
                                        key.core)
    }

    internal func submit (transfer: Transfer) {
        wkWalletManagerSubmitSigned (core,
                                         transfer.wallet.core,
                                         transfer.core)
    }

    internal func setNetworkReachable (_ isNetworkReachable: Bool) {
        wkWalletManagerSetNetworkReachable (core,
                                                isNetworkReachable ? WK_TRUE : WK_FALSE)
    }

    public func createSweeper (wallet: Wallet,
                               key: Key,
                               completion: @escaping (Result<WalletSweeper, WalletSweeperError>) -> Void) {
        WalletSweeper.create(wallet: wallet, key: key, client: client, completion: completion)
    }
    
    public func createExportablePaperWallet () -> Result<ExportablePaperWallet, ExportablePaperWalletError> {
        return ExportablePaperWallet.create(manager: self)
    }

    public func createConnector () -> Result<WalletConnector, WalletConnectorError> {
        return WalletConnector.create (manager:self)
    }

    internal init (core: WKWalletManager,
                   system: System,
                   callbackCoordinator: SystemCallbackCoordinator,
                   take: Bool) {

        self.core   = take ? wkWalletManagerTake(core) : core
        self.system = system
        self.callbackCoordinator = callbackCoordinator

        self.account = Account (core: wkWalletManagerGetAccount(core), take: false)
        self.network = Network (core: wkWalletManagerGetNetwork (core), take: false)
        self.unit    = self.network.defaultUnitFor (currency: self.network.currency)!
        self.path    = asUTF8String (wkWalletManagerGetPath(core))
        self.client  = system.client

        self.defaultNetworkFee = self.network.minimumFee
    }

    deinit {
        wkWalletManagerGive (core)
    }

    // Equatable
    public static func == (lhs: WalletManager, rhs: WalletManager) -> Bool {
        return lhs === rhs || lhs.core == rhs.core
    }

    public var description: String {
        return name
    }
}

extension WalletManager {
    ///
    /// Create a wallet for `currency`.  Invokdes the manager's `walletFactory` to create the
    /// wallet.  Generates events: Wallet.created, WalletManager.walletAdded(wallet), perhaps
    /// others.
    ///
    /// - Parameter currency: the wallet's currency
    ///
    /// - Returns: a new wallet.
    ///
    //    func createWallet (currency: Currency) -> Wallet {
    //        return walletFactory.createWallet (manager: self,
    //                                           currency: currency)
    //    }

    /// The network's/primaryWallet's currency.  This is the currency used for transfer fees.
    var currency: Currency {
        return network.currency // don't reference `primaryWallet`; infinitely recurses
    }

    /// The name is simply the network currency's code - e.g. BTC, ETH
    public var name: String {
        return currency.code
    }

    /// The baseUnit for the network's currency.
    var baseUnit: Unit {
        return network.baseUnitFor(currency: network.currency)!
    }

    /// The defaultUnit for the network's currency.
    var defaultUnit: Unit {
        return network.defaultUnitFor(currency: network.currency)!
    }

    /// A manager `isActive` if connected or syncing
    var isActive: Bool {
        return state == .connected || state == .syncing
    }
}

///
/// The WalletSweeper
///
public enum WalletSweeperError: Error {
    case unsupportedCurrency
    case invalidKey
    case invalidSourceWallet
    case insufficientFunds
    case unableToSweep
    case noTransfersFound
    case unexpectedError
    case clientError(SystemClientError)

    internal init? (_ core: WKWalletSweeperStatus) {
        switch core {
        case WK_WALLET_SWEEPER_SUCCESS:                 return nil
        case WK_WALLET_SWEEPER_UNSUPPORTED_CURRENCY:    self = .unsupportedCurrency
        case WK_WALLET_SWEEPER_INVALID_KEY:             self = .invalidKey
        case WK_WALLET_SWEEPER_INVALID_SOURCE_WALLET:   self = .invalidSourceWallet
        case WK_WALLET_SWEEPER_INSUFFICIENT_FUNDS:      self = .insufficientFunds
        case WK_WALLET_SWEEPER_UNABLE_TO_SWEEP:         self = .unableToSweep
        case WK_WALLET_SWEEPER_NO_TRANSFERS_FOUND:      self = .noTransfersFound
        case WK_WALLET_SWEEPER_INVALID_ARGUMENTS:       self = .unexpectedError
        case WK_WALLET_SWEEPER_INVALID_TRANSACTION:     self = .unexpectedError
        case WK_WALLET_SWEEPER_ILLEGAL_OPERATION:       self = .unexpectedError
        default: self = .unexpectedError; preconditionFailure()
        }
    }
}

public final class WalletSweeper {

    internal static func create(wallet: Wallet,
                                key: Key,
                                client: SystemClient,
                                completion: @escaping (Result<WalletSweeper, WalletSweeperError>) -> Void) {
        // check that requested combination of manager, wallet, key can be used for sweeping
        if let e = WalletSweeperError(wkWalletManagerWalletSweeperValidateSupported(wallet.manager.core,
                                                                                        wallet.core,
                                                                                        key.core)) {
            completion(Result.failure(e))
            return
        }

        switch wkNetworkGetType (wallet.manager.network.core) {
        case WK_NETWORK_TYPE_BTC, WK_NETWORK_TYPE_BCH:
            // handle as BTC, creating the underlying WKWalletSweeper and initializing it
            // using the BlockchainDB
            createAsBtc(wallet: wallet,
                        key: key)
                .initAsBTC(client: client,
                           completion: completion)
        default:
            preconditionFailure()
        }
    }

    private static func createAsBtc(wallet: Wallet,
                                    key: Key) -> WalletSweeper {
        return WalletSweeper(core: wkWalletManagerCreateWalletSweeper(wallet.manager.core,
                                                                          wallet.core,
                                                                          key.core),
                             wallet: wallet,
                             key: key)
    }

    internal let core: WKWalletSweeper
    private let manager: WalletManager
    private let wallet: Wallet
    private let key: Key

    private init (core: WKWalletSweeper,
                  wallet: Wallet,
                  key: Key) {
        self.core = core
        self.manager = wallet.manager
        self.wallet = wallet
        self.key = key
    }

    public var balance: Amount? {
        return wkWalletSweeperGetBalance (self.core)
            .map { Amount (core: $0, take: false) }
    }

    public func estimate(fee: NetworkFee,
                         completion: @escaping (Result<TransferFeeBasis, Wallet.FeeEstimationError>) -> Void) {
        wallet.estimateFee(sweeper: self, fee: fee, completion: completion)
    }

    public func submit(estimatedFeeBasis: TransferFeeBasis) -> Transfer? {
        guard let transfer = wallet.createTransfer(sweeper: self, estimatedFeeBasis: estimatedFeeBasis)
            else { return nil }

        manager.submit(transfer: transfer, key: key)
        return transfer
    }

    private func initAsBTC(client: SystemClient,
                           completion: @escaping (Result<WalletSweeper, WalletSweeperError>) -> Void) {
        let network = manager.network
        let address = Address (core: wkWalletSweeperGetAddress(core)!).description

        client.getTransactions(blockchainId: network.uids,
                               addresses: [address],
                               begBlockNumber: 0,
                               endBlockNumber: network.height,
                               includeRaw: true,
                               includeTransfers: false) {
                                (res: Result<[SystemClient.Transaction], SystemClientError>) in
                                res.resolve(
                                    success: {
                                        let bundles: [WKClientTransactionBundle?] = $0.map { System.makeTransactionBundle ($0) }
                                        // populate the underlying WKWalletSweeper with BTC transaction data
                                        for bundle in bundles {
                                            if let e = WalletSweeperError(wkWalletSweeperAddTransactionFromBundle(self.core, bundle)) {
                                                completion(Result.failure(e))
                                                return
                                            }
                                        }
                                        
                                        // validate that the sweeper has the necessary info
                                        if let e = WalletSweeperError(wkWalletSweeperValidate(self.core)) {
                                            completion(Result.failure(e))
                                            return
                                        }
                                        
                                        // return the sweeper for use in estimation/submission
                                        completion(Result.success(self))},
                                    failure: { completion(Result.failure(.clientError($0))) })
        }
    }

    deinit {
        wkWalletSweeperRelease(core)
    }
}

///
/// Exportable Paper Wallet
///

public enum ExportablePaperWalletError: Error {
    case unsupportedCurrency
    case unexpectedError

    internal init? (_ core: WKExportablePaperWalletStatus) {
        switch core {
        case WK_EXPORTABLE_PAPER_WALLET_SUCCESS:                 return nil
        case WK_EXPORTABLE_PAPER_WALLET_UNSUPPORTED_CURRENCY:    self = .unsupportedCurrency
        case WK_EXPORTABLE_PAPER_WALLET_INVALID_ARGUMENTS:       self = .unexpectedError
        default: self = .unexpectedError; preconditionFailure()
        }
    }
}

public final class ExportablePaperWallet {
    internal static func create(manager: WalletManager) -> Result<ExportablePaperWallet, ExportablePaperWalletError> {
        // check that requested wallet supports generating exportable paper wallets
        if let error =  ExportablePaperWalletError (wkExportablePaperWalletValidateSupported (manager.network.core,
                                                                                                  manager.currency.core)) {
            return Result.failure (error)
        }

        return wkExportablePaperWalletCreate (manager.network.core, manager.currency.core)
            .map { Result.success (ExportablePaperWallet (core: $0)) }
            ?? Result.failure(.unexpectedError)
    }

    internal let core: WKWalletSweeper

    private init (core: WKWalletSweeper) {
        self.core = core
    }

    public var privateKey: Key? {
        return wkExportablePaperWalletGetKey (self.core)
            .map { Key (core: $0) }
    }

    public var address: Address? {
        return wkExportablePaperWalletGetAddress (self.core)
            .map { Address (core: $0, take: false) }
    }

    deinit {
        wkExportablePaperWalletRelease(core)
    }
}

///
/// WalletConnector
///

public enum WalletConnectorError: Error {
    
    case unsupportedConnector
    case illegalOperation
    case unknownEntity
    case invalidTransactionArguments
    case missingFee
    case invalidTransactionSerialization
    case invalidKeyForSigning
    case unrecoverableKey
    case unsignedTransaction
    case previouslySignedTransaction
    case invalidDigest
    case invalidSignature
    case invalidJson
    case invalidTypeData
    case submitFailed
    // ...

    internal init (core: WKWalletConnectorStatus) {
        switch core {
        case WK_WALLET_CONNECTOR_STATUS_UNSUPPORTED_CONNECTOR:          self = .unsupportedConnector
        case WK_WALLET_CONNECTOR_STATUS_ILLEGAL_OPERATION:              self = .illegalOperation
        case WK_WALLET_CONNECTOR_STATUS_INVALID_TRANSACTION_ARGUMENTS:  self = .invalidTransactionArguments
        case WK_WALLET_CONNECTOR_STATUS_TRANSACTION_MISSING_FEE:        self = .missingFee
        case WK_WALLET_CONNECTOR_STATUS_INVALID_SIGNATURE:              self = .invalidSignature
        case WK_WALLET_CONNECTOR_STATUS_INVALID_SERIALIZATION:          self = .invalidTransactionSerialization
        case WK_WALLET_CONNECTOR_STATUS_INVALID_DIGEST:                 self = .invalidDigest
        case WK_WALLET_CONNECTOR_STATUS_KEY_RECOVERY_FAILED:            self = .unrecoverableKey
        case WK_WALLET_CONNECTOR_STATUS_INVALID_JSON:                   self = .invalidJson
        case WK_WALLET_CONNECTOR_STATUS_INVALID_TYPED_DATA:             self = .invalidTypeData
    
        // Not an error and should never be passed to this enumeration
        case WK_WALLET_CONNECTOR_STATUS_OK: preconditionFailure()
        default: preconditionFailure()
        }
    }
}

public final class WalletConnector {
    ///
    /// Create a WalletConnector is supported for the manager's network
    ///
    /// - Parameter manager: The `WalletManager` for this connector
    ///
    /// - Returns: On success, a WalletConnector for `manager`.  On failure, a WalletConnectError of:
    ///     .unsupportedConnector - If `manager` does not support the WalletConnect 1.0 specification
    ///
    internal static func create (manager: WalletManager) -> Result<WalletConnector, WalletConnectorError> {
        return wkWalletConnectorCreate (manager.core)
            .map { Result.success (WalletConnector (core: $0, manager: manager)) }
            ?? Result.failure(.unsupportedConnector)
    }

    /// The core reference
    internal let core: WKWalletConnector

    /// The manager for this connector
    internal unowned let manager: WalletManager

    private init (core: WKWalletConnector, manager: WalletManager) {
        self.core = core
        self.manager = manager
    }

    deinit {
        wkWalletConnectorRelease (core)
    }

    /// Create a 'standard message' from 'message'
    internal func createStandardMessage (message: Data) -> Result<Data, WalletConnectorError> {
        return message.withUnsafeBytes { (messageBytes: UnsafeRawBufferPointer) -> Result<Data, WalletConnectorError> in
            let messageAddr   = messageBytes.baseAddress?.assumingMemoryBound(to:UInt8.self)
            let messageLength = messageBytes.count

            var standardMessageStatus: WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK
            var standardMessageLength: size_t = 0

            guard let standardMessageBytes = wkWalletConnectorCreateStandardMessage (self.core,
                                                                                     messageAddr,
                                                                                     messageLength,
                                                                                     &standardMessageLength,
                                                                                     &standardMessageStatus)
            else { return Result.failure (WalletConnectorError (core: standardMessageStatus)) }
            defer { wkMemoryFree (standardMessageBytes) }

            return Result.success (Data (bytes: standardMessageBytes, count: standardMessageLength))
        }
    }

    ///
    /// Creates a WalletConnector compatible signing Key
    ///
    /// - Parameter paperKey: A BIP39 phrase
    ///
    /// - Returns: A Key object which is suitable for
    ///            sign() of either Data or Transaction
    public func createKey (paperKey: String) -> Result<Key, WalletConnectorError> {
        
        var status: WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK
        guard let coreKey = wkWalletConnectorCreateKey(self.core, paperKey, &status)
        else { return Result.failure(WalletConnectorError (core: status)) }
        
        return Result.success(Key (core: coreKey))
    }
    
    ///
    /// Sign arbitrary data
    ///
    /// - Parameter message: Arbitrary data to be signed
    /// - Parameter key: A private key
    /// - Parameter prefix: Indicates to include an optional prefix in the signature
    ///
    /// - Returns: On success a pair {Digest,Signature}.  On failure a WalletConnectorError of:
    ///     .invalidKeyForSigning - if `key` is not private
    ///
    public func sign (message: Data, using key: Key, prefix: Bool = true) -> Result<(digest: Digest, signature: Signature), WalletConnectorError> {
        guard key.hasSecret else { return Result.failure(.invalidKeyForSigning) }

        //
        // Determine the `mesageToSign`
        //

        var messageToSign = message
        if (prefix) {
            // A little awkward but perhaps clearer
            switch (createStandardMessage(message: message)) {
            case .success(let standardMessage):
                messageToSign = standardMessage
            case .failure(let error):
                return Result.failure(error)
            }
        }

        return messageToSign.withUnsafeBytes { (messageBytes: UnsafeRawBufferPointer) -> Result<(digest: Digest, signature: Signature), WalletConnectorError> in

            let messageAddr   = messageBytes.baseAddress?.assumingMemoryBound(to:UInt8.self)
            let messageLength = messageBytes.count

            //
            // Get the Digest
            //

            var digestStatus: WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK
            var digestLength: size_t = 0

            guard let digestBytes = wkWalletConnectorGetDigest (self.core,
                                                                messageAddr,
                                                                messageLength,
                                                                &digestLength,
                                                                &digestStatus)
            else { return Result.failure (WalletConnectorError (core: digestStatus)) }
            defer { wkMemoryFree (digestBytes) }

            let digest = Digest (core: self.core, data32: Data (bytes: digestBytes, count: digestLength))

            //
            // Get the Signature
            //

            var signatureLength: size_t = 0
            var signatureStatus: WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK

            // We pass the `message` - because wkWalletConnectorSignData() has an implicit 'hash'
            // It is guaranteed that `wkWalletConnectorGetDigest()` uses the same 'hash'
            guard let signatureBytes = wkWalletConnectorSignData (self.core,
                                                                  messageAddr,
                                                                  messageLength,
                                                                  key.core,
                                                                  &signatureLength,
                                                                  &signatureStatus )
            else { return Result.failure (WalletConnectorError (core: signatureStatus)) }
            defer { wkMemoryFree (signatureBytes) }

            let signature = Signature (core: self.core, data: Data (bytes: signatureBytes, count: signatureLength))

            //
            // Return the Digest and Signature
            //

            return Result.success((digest: digest, signature: signature))
        }
    }

    ///
    /// Recover the public key
    ///
    /// - Parameters:
    ///   - digest: the digest
    ///   - signature: the corresponding signature
    ///
    /// - Returns: On success, a public key.  On failure, a WalletConnectError of:
    ///       .unknownEntity - `digest` or `signature` are not from `self`
    ///       .unrecoverableKey - in the event `signature` was not produced by a recoverable signing algorithm
    ///
    public func recover (digest: Digest, signature: Signature) -> Result<Key, WalletConnectorError> {
        guard core == digest.core, core == signature.core else { return Result.failure(.unknownEntity) }

        return digest.data32.withUnsafeBytes { (digestBytes: UnsafeRawBufferPointer) -> Result<Key, WalletConnectorError> in
            let digestAddr = digestBytes.baseAddress?.assumingMemoryBound(to:UInt8.self)
            let digestLength = digestBytes.count
            return signature.data.withUnsafeBytes { (signatureBytes: UnsafeRawBufferPointer) -> Result<Key, WalletConnectorError> in
                let signatureAddr = signatureBytes.baseAddress?.assumingMemoryBound(to:UInt8.self);
                let signatureLength = signatureBytes.count
                var status : WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK
                let key = wkWalletConnectorRecoverKey(self.core,
                                                      digestAddr,
                                                      digestLength,
                                                      signatureAddr,
                                                      signatureLength,
                                                      &status)
                if (key == nil) {
                    return Result.failure(WalletConnectorError(core: status))
                }
                
                // Return key will own the walletkit native key memory hereafter
                return Result.success(Key(core: key!));
            }
        }
    }

    ///
    /// Create a Serialization from a transaction's unsigned or signed data.
    ///
    /// - Parameter data: The data
    ///
    /// - Returns: A serialization
    ///
    public func createSerialization (data: Data) -> Serialization {
        return Serialization (core: core, data: data)
    }

    ///
    /// Create a Transaction from a wallet-connect-specific dictionary of arguments applicable to
    /// the connector's network.  For ETH the Dictionary keys are: {...}
    ///
    /// This function is the 'create' part of the ETH JSON-RPC `eth_sendTransaction`
    ///
    /// - Parameter arguments: A dictionary (JSON-RPC-like) of create arguments
    ///
    /// - Returns: On success, an unsigned `Transaction`.  On failure, a WalletConnectError of:
    ///      TBD
    ///
    public func createTransaction (arguments: Dictionary<String,String>, defaultFee:NetworkFee? = nil) -> Result<Transaction, WalletConnectorError> {
        
        let keys = Array(arguments.keys)
        let values = Array(arguments.values)
        var serializationLength :size_t = 0
        var status : WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK
        
        // See WKAccount.swift validatePhrase which passes an array of words as
        // const char *words[] through wkAccountValidatePaperKey
        var keysStrs = keys.map {UnsafePointer<Int8> (strdup($0))}
        var valuesStrs = values.map {UnsafePointer<Int8> (strdup($0))}
        defer {
            keysStrs.forEach { wkMemoryFree (UnsafeMutablePointer (mutating: $0))}
            valuesStrs.forEach { wkMemoryFree (UnsafeMutablePointer (mutating: $0))}
        }
        
        let serializationBytes = wkWalletConnectorCreateTransactionFromArguments(self.core,
                                                                                 &(keysStrs),
                                                                                 &(valuesStrs),
                                                                                 keys.count,
                                                                                 defaultFee?.core,
                                                                                 &serializationLength,
                                                                                 &status)
                                                      
        defer { wkMemoryFree(serializationBytes) }
        if (serializationBytes == nil) {
            return Result.failure(WalletConnectorError(core: status))
        }
        let serializationData = Data(bytes: serializationBytes!, count: serializationLength)
        let serialization = Serialization(core: self.core, data: serializationData)
        return Result.success(Transaction(core: self.core, isSigned: false, serialization: serialization))
    }

    ///
    /// Create a Transaction from a serialization
    ///
    /// - Parameter serialization: A transaction serialization, signed or unsigned
    ///
    /// - Returns: On success, an unsigned or signed `Transaction`.  On failure, a WalletConnectError of:
    ///     .unknownEntity - if serialization is not from `self`
    ///      TBD
    ///
    public func createTransaction (serialization: Serialization) -> Result<Transaction, WalletConnectorError> {
        guard core == serialization.core else { return Result.failure(.unknownEntity) }

        return serialization.data.withUnsafeBytes { (serializationBytes: UnsafeRawBufferPointer) -> Result<Transaction, WalletConnectorError> in
            
            let serializationBytesAddr = serializationBytes.baseAddress?.assumingMemoryBound(to:UInt8.self)
            let serializationBytesLength = serializationBytes.count
            var serializationLength : size_t = 0
            var isSignedCore : WKBoolean = WK_FALSE
            var status : WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK
            
            let serializedBytes = wkWalletConnectorCreateTransactionFromSerialization(self.core,
                                                                                      serializationBytesAddr,
                                                                                      serializationBytesLength,
                                                                                      &serializationLength,
                                                                                      &isSignedCore,
                                                                                      &status           );
            defer { wkMemoryFree(serializedBytes) }
            if (serializedBytes == nil) {
                return Result.failure(WalletConnectorError(core: status))
            }
            let serializationData = Data(bytes: serializedBytes!, count: serializationLength)
            let serialization = Serialization(core: self.core, data: serializationData)
            return Result.success(Transaction(core: self.core,
                                              isSigned: (isSignedCore == WK_TRUE ? true : false),
                                              serialization: serialization))
        }
    }

    ///
    /// Sign a transaction
    ///
    /// This function is the 'sign' part of the ETH JSON-RPC `eth_sendTransaction` and
    /// `eth_sendRawTransaction`.
    ///
    /// - Parameter transaction: The transaction to sign
    /// - Parameter key: A private key
    ///
    /// - Returns: On success, a signed `Transaction` which will be distinct from the provided
    ///  `transaction` argument.  On failure, a WalletConnectError of:
    ///     .unknownEntity - `transaction` is not from `self`
    ///     .invalidKeyForSigning - if `key` is not private
    ///
    public func sign (transaction: Transaction, using key: Key) -> Result<Transaction, WalletConnectorError> {
        guard core == transaction.core else { return Result.failure(.unknownEntity) }
        guard key.hasSecret            else { return Result.failure(.invalidKeyForSigning) }
        guard !transaction.isSigned    else { return Result.failure(.previouslySignedTransaction) }
        
        return transaction.serialization.data.withUnsafeBytes { (transactionBytes: UnsafeRawBufferPointer) -> Result<Transaction, WalletConnectorError> in
            
            let transactionBytesAddr = transactionBytes.baseAddress?.assumingMemoryBound(to:UInt8.self)
            let transactionBytesLength = transactionBytes.count
            var serializationLength : size_t = 0
            var transactionIdentifierLength : size_t = 0;
            var transactionIdentifierBytes : UnsafeMutablePointer<UInt8>!
            var status : WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK
            
            let signedTransactionBytes = wkWalletConnectorSignTransactionData(self.core,
                                                                              transactionBytesAddr,
                                                                              transactionBytesLength,
                                                                              key.core,
                                                                              &transactionIdentifierBytes,
                                                                              &transactionIdentifierLength,
                                                                              &serializationLength,
                                                                              &status)
            defer {
                wkMemoryFree(signedTransactionBytes)
                wkMemoryFree(transactionIdentifierBytes)
            }
            
            if (signedTransactionBytes == nil) {
                return Result.failure(WalletConnectorError(core: status))
            }
            
            let signedSerializationData = Data(bytes: signedTransactionBytes!, count: serializationLength)
            let transactionIdentifier = Data(bytes: transactionIdentifierBytes, count: transactionIdentifierLength)
            let signedSerialization = Serialization(core: self.core, data: signedSerializationData)
            return Result.success(Transaction(core: self.core,
                                              isSigned: true,
                                              identifier: transactionIdentifier,
                                              serialization:signedSerialization))
        }
    }

    ///
    /// Sign typed data. The typedData provided must firstly be a string containing a valid JSON object. Secondly,
    /// the contents of the JSON must be typed data in the form suitable to the network on which the WalletConnector
    /// operates (for example, with Ethereum networks, the typedData must be presented as EIP-712 structured data).
    ///
    /// This function designated for handling JSON-RPC `eth_signTypedData`
    ///
    /// - Parameter typedData: The data to sign
    /// - Parameter key: A private key
    ///
    /// - Returns: On success a pair {Digest,Signature}.  On failure a WalletConnectorError of:
    ///     .invalidKeyForSigning - if `key` is not private
    ///     .invalidJson - if the typedData is not a JSON
    ///     .invalidTypedData - if the JSON is not a valid typed data for the wallet connectors network
    public func sign (typedData: String, using key: Key) -> Result<(digest: Digest, signature: Signature), WalletConnectorError> {
        guard key.hasSecret else { return Result.failure(.invalidKeyForSigning) }

        var digestLen : size_t = 0;
        var signatureLen : size_t = 0;
        var digestData : UnsafeMutablePointer<UInt8>!
        
        var status : WKWalletConnectorStatus = WK_WALLET_CONNECTOR_STATUS_OK;

        let signatureData  = wkWalletConnectorSignTypedData (self.core,
                                                             typedData,
                                                             key.core,
                                                             &digestData,
                                                             &digestLen,
                                                             &signatureLen,
                                                             &status);
        defer {
            wkMemoryFree(signatureData);
            wkMemoryFree(digestData);
        }
        if (signatureData == nil) {
            return Result.failure(WalletConnectorError(core: status))
        }
        
        let digest = Digest (core: self.core, data32: Data (bytes: digestData, count: digestLen))
        let signature = Signature (core: self.core, data: Data (bytes: signatureData!, count: signatureLen))
        return Result.success((digest: digest, signature: signature))
    }
    
    ///
    /// Send a transaction to the connector's network
    ///
    /// This function is the 'submit' part of the ETH JSON-RPC `eth_sendTransaction` and
    /// `eth_sendRawTransaction`.
    ///
    /// - Parameter The `transaction` to submit
    ///
    /// - Returns: On success, a submitted transaction which may be distinct from the provided
    ///   transaction argument.  On failure, a WalletConnectError of:
    ///     .unknownEntity - `transaction` is not from `self`
    ///     .unsignedTransaction - `transaaction` is not signed
    ///     .submitFailed - the `transaction` was not submitted
    ///
    public func submit (transaction: Transaction,
                        completion: @escaping (Result<Transaction, WalletConnectorError>) -> Void) {
        guard core == transaction.core else { return completion (Result.failure(.unknownEntity)) }
        guard transaction.isSigned     else { return completion (Result.failure(.unsignedTransaction)) }

        manager.client.createTransaction (blockchainId: manager.network.uids,
                                          transaction: transaction.serialization.data,
                                          identifier: "WalletConnect: \(manager.network.uids): \(transaction.serialization.data.prefix(through: 10).base64EncodedString())") {
            (res: Result<SystemClient.TransactionIdentifier, SystemClientError>) in

            res.resolve(
                success: { (ti) in completion (Result.success (transaction)) },
                failure: { ( e) in completion (Result.failure (.submitFailed)) }
            )
        }
    }

    ///
    ///  A Key that may be used for signing if it contains a secret
    ///
    public class Key {
        /// The owning key
        internal var core: WKKey
        
        /// Indicates the Key has a private key
        public var hasSecret: Bool {
            return 1 == wkKeyHasSecret  (self.core)
        }
        
        deinit {
            wkKeyGive (core)
        }
        
        ///
        /// Initialize based on chain specific WKKey object
        ///
        /// - Parameter core: The Core representaion
        ///
        internal init (core: WKKey) {
            self.core = core
        }
        
    }
    
    /// A Digest holds '32 hash bytes'
    ///
    public struct Digest {
        /// The owning connector.
        internal var core: WKWalletConnector

        /// The data as 32 bytes (typically)
        public var data32: Data
    }

    ///
    /// A Signature holds the signature bytes in a form specific to the WalletConnector
    ///
    public struct Signature {
        /// The owning connector.
        internal var core: WKWalletConnector

        /// The data
        public var data: Data          // (65 byte RSV Eth signature)
    }

    ///
    /// A Serialization is a byte sequence representing an unsigned or signed transaction
    ///
    public struct Serialization {
        /// The owning connector.
        internal var core: WKWalletConnector

        // The data
        public var data: Data
    }

    ///
    /// A Transaction
    ///
    public struct Transaction {
        /// The owning connector.
        internal var core: WKWalletConnector

        /// Check if signed
        public var isSigned: Bool

        /// The transaction's identifier.  This is optional and will exist if `isSigned`
        public var identifier: Data?
        
        /// The serialization - this could be unsigned (if `!isSigned`) or signed (if `isSigned`)
        public var serialization: Serialization

        // Adding these two properties, for `isSubmitted` and `submissionError` requires a change
        // to the `completion` argument to `func submit (...)`.  I don't want to make that change.
        // The change might be `completion: (Transaction) -> Void` with the transaction having
        // set `isSubmitted=true` and `submissionError=<error`
        #if false
        /// Check if submitted.  A value of `true` does not indicate submission success
        public var isSubmitted: Bool

        /// An optional submission error
        public var submissionError: WalletConnectorError? = nil
        #endif
    }
}

///
///
///
public enum WalletManagerDisconnectReason: Equatable {
    case requested
    case unknown
    case posix(errno: Int32, message: String?)

    internal init (core: WKWalletManagerDisconnectReason) {
        switch core.type {
        case WK_WALLET_MANAGER_DISCONNECT_REASON_REQUESTED:
            self = .requested
        case WK_WALLET_MANAGER_DISCONNECT_REASON_UNKNOWN:
            self = .unknown
        case WK_WALLET_MANAGER_DISCONNECT_REASON_POSIX:
            var c = core
            self = .posix(errno: core.u.posix.errnum,
                          message: wkWalletManagerDisconnectReasonGetMessage(&c).map{ asUTF8String($0, true) })
        default: self = .unknown; preconditionFailure()
        }
    }
}

///
/// The WalletManager state.
///
public enum WalletManagerState: Equatable {
    case created
    case disconnected(reason: WalletManagerDisconnectReason)
    case connected
    case syncing
    case deleted

    internal init (core: WKWalletManagerState) {
        switch core.type {
        case WK_WALLET_MANAGER_STATE_CREATED:      self = .created
        case WK_WALLET_MANAGER_STATE_DISCONNECTED: self = .disconnected(reason: WalletManagerDisconnectReason(core: core.u.disconnected.reason))
        case WK_WALLET_MANAGER_STATE_CONNECTED:    self = .connected
        case WK_WALLET_MANAGER_STATE_SYNCING:      self = .syncing
        case WK_WALLET_MANAGER_STATE_DELETED:      self = .deleted
        default: self = .created; preconditionFailure()
        }
    }
}

///
/// The WalletManager's mode determines how the account and associated wallets are managed.
///
/// - api_only: Use only the defined 'Cloud-Based API' to synchronize the account's transfers.
///
/// - api_with_p2p_submit: Use the defined 'Cloud-Based API' to synchronize the account's transfers
///      but submit transfers using the network's Peer-to-Peer protocol.
///
/// - p2p_with_api_sync: Use the network's Peer-to-Peer protocol to synchronize the account's
///      recents transfers but use the 'Cloud-Based API' to synchronize older transfers.
///
/// - p2p_only: Use the network's Peer-to-Peer protocol to synchronize the account's transfers.
///
public enum WalletManagerMode: Equatable {
    case api_only
    case api_with_p2p_submit
    case p2p_with_api_sync
    case p2p_only

    /// Allow WalletMangerMode to be saved
    public var serialization: UInt8 {
        switch self {
        case .api_only:            return 0xf0
        case .api_with_p2p_submit: return 0xf1
        case .p2p_with_api_sync:   return 0xf2
        case .p2p_only:            return 0xf3
        }
    }

    /// Initialize WalletMangerMode from serialization
    public init? (serialization: UInt8) {
        switch serialization {
        case 0xf0: self = .api_only
        case 0xf1: self = .api_with_p2p_submit
        case 0xf2: self = .p2p_with_api_sync
        case 0xf3: self = .p2p_only
        default: return nil
        }
    }

    internal init (core: WKSyncMode) {
        switch core {
        case WK_SYNC_MODE_API_ONLY: self = .api_only
        case WK_SYNC_MODE_API_WITH_P2P_SEND: self = .api_with_p2p_submit
        case WK_SYNC_MODE_P2P_WITH_API_SYNC: self = .p2p_with_api_sync
        case WK_SYNC_MODE_P2P_ONLY: self = .p2p_only
        default: self = .api_only; preconditionFailure()
        }
    }

    internal var core: WKSyncMode {
        switch self {
        case .api_only: return WK_SYNC_MODE_API_ONLY
        case .api_with_p2p_submit: return WK_SYNC_MODE_API_WITH_P2P_SEND
        case .p2p_with_api_sync: return WK_SYNC_MODE_P2P_WITH_API_SYNC
        case .p2p_only: return WK_SYNC_MODE_P2P_ONLY
        }
    }

    public static let all = [WalletManagerMode.api_only,
                             WalletManagerMode.api_with_p2p_submit,
                             WalletManagerMode.p2p_with_api_sync,
                             WalletManagerMode.p2p_only]
    
    // Equatable: [Swift-generated]
}

///
/// The WalletManager's sync depth determines the range that a sync is performed on.
///
/// - fromLastConfirmedSend: Sync from the block height of the last confirmed send transaction.
///
/// - fromLastTrustedBlock: Sync from the block height of the last trusted block; this is
///      dependent on the blockchain and mode as to how it determines trust.
///
/// - fromCreation: Sync from the block height of the point in time when the account was created.
///
public enum WalletManagerSyncDepth: Equatable {
    case fromLastConfirmedSend
    case fromLastTrustedBlock
    case fromCreation

    /// Allow WalletMangerMode to be saved
    public var serialization: UInt8 {
        switch self {
        case .fromLastConfirmedSend: return 0xa0
        case .fromLastTrustedBlock:  return 0xb0
        case .fromCreation:          return 0xc0
        }
    }

    /// Initialize WalletMangerMode from serialization
    public init? (serialization: UInt8) {
        switch serialization {
        case 0xa0: self = .fromLastConfirmedSend
        case 0xb0: self = .fromLastTrustedBlock
        case 0xc0: self = .fromCreation
        default: return nil
        }
    }

    internal init (core: WKSyncDepth) {
        switch core {
        case WK_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND: self = .fromLastConfirmedSend
        case WK_SYNC_DEPTH_FROM_LAST_TRUSTED_BLOCK: self = .fromLastTrustedBlock
        case WK_SYNC_DEPTH_FROM_CREATION: self = .fromCreation
        default: self = .fromCreation; preconditionFailure()
        }
    }

    internal var core: WKSyncDepth {
        switch self {
        case .fromLastConfirmedSend: return WK_SYNC_DEPTH_FROM_LAST_CONFIRMED_SEND
        case .fromLastTrustedBlock: return WK_SYNC_DEPTH_FROM_LAST_TRUSTED_BLOCK
        case .fromCreation: return WK_SYNC_DEPTH_FROM_CREATION
        }
    }

    public var shallower: WalletManagerSyncDepth? {
        switch self {
        case .fromCreation: return .fromLastTrustedBlock
        case .fromLastTrustedBlock: return .fromLastConfirmedSend
        default: return nil
        }
    }

    public var deeper: WalletManagerSyncDepth? {
        switch self {
        case .fromLastConfirmedSend: return .fromLastTrustedBlock
        case .fromLastTrustedBlock: return .fromCreation
        default: return nil
        }
    }

    // Equatable: [Swift-generated]
}

public enum WalletManagerSyncStoppedReason: Equatable {
    case complete
    case requested
    case unknown
    case posix(errno: Int32, message: String?)

    internal init (core: WKSyncStoppedReason) {
        switch core.type {
        case WK_SYNC_STOPPED_REASON_COMPLETE:
            self = .complete
        case WK_SYNC_STOPPED_REASON_REQUESTED:
            self = .requested
        case WK_SYNC_STOPPED_REASON_UNKNOWN:
            self = .unknown
        case WK_SYNC_STOPPED_REASON_POSIX:
            var c = core
            self = .posix(errno: core.u.posix.errnum,
                          message: wkSyncStoppedReasonGetMessage(&c).map{ asUTF8String($0, true) })
        default: self = .unknown; preconditionFailure()
        }
    }
}

///
/// A WalletManager Event represents a asynchronous announcment of a managera's state change.
///
public enum WalletManagerEvent {
    case created
    case changed (oldState: WalletManagerState, newState: WalletManagerState)
    case deleted

    case walletAdded   (wallet: Wallet)
    case walletChanged (wallet: Wallet)
    case walletDeleted (wallet: Wallet)

    case syncStarted
    case syncProgress (timestamp: Date?, percentComplete: Float)
    case syncEnded (reason: WalletManagerSyncStoppedReason)
    case syncRecommended (depth: WalletManagerSyncDepth)

    /// An event capturing a change in the block height of the network associated with a
    /// WalletManager. Developers should listen for this event when making use of
    /// Transfer::confirmations, as that value is calculated based on the associated network's
    /// block height. Displays or caches of that confirmation count should be updated when this
    /// event occurs.
    case blockUpdated (height: UInt64)

    init (manager: WalletManager, core: WKWalletManagerEvent) {
        switch core.type {
        case WK_WALLET_MANAGER_EVENT_CREATED:
            self = .created

        case WK_WALLET_MANAGER_EVENT_CHANGED:
            self = .changed(oldState: WalletManagerState(core: core.u.state.old),
                            newState: WalletManagerState(core: core.u.state.new))

        case WK_WALLET_MANAGER_EVENT_DELETED:
            self = .deleted

        case WK_WALLET_MANAGER_EVENT_WALLET_ADDED:
            self = .walletAdded (wallet: Wallet (core: core.u.wallet,
                                                 manager: manager,
                                                 callbackCoordinator: manager.callbackCoordinator,
                                                 take: false))

        case WK_WALLET_MANAGER_EVENT_WALLET_CHANGED:
            self = .walletChanged (wallet: Wallet (core: core.u.wallet,
                                                   manager: manager,
                                                   callbackCoordinator: manager.callbackCoordinator,
                                                   take: false))

        case WK_WALLET_MANAGER_EVENT_WALLET_DELETED:
            self = .walletDeleted (wallet: Wallet (core: core.u.wallet,
                                                   manager: manager,
                                                   callbackCoordinator: manager.callbackCoordinator,
                                                   take: false))

        // wallet: added: ...
        case WK_WALLET_MANAGER_EVENT_SYNC_STARTED:
            self = .syncStarted
            
        case WK_WALLET_MANAGER_EVENT_SYNC_CONTINUES:
            let timestamp: Date? = (0 == core.u.syncContinues.timestamp // NO_WK_TIMESTAMP
                ? nil
                : Date (timeIntervalSince1970: TimeInterval(core.u.syncContinues.timestamp)))

            self = .syncProgress (timestamp: timestamp,
                                  percentComplete: core.u.syncContinues.percentComplete)

        case WK_WALLET_MANAGER_EVENT_SYNC_STOPPED:
            let reason = WalletManagerSyncStoppedReason(core: core.u.syncStopped.reason)
            self = .syncEnded(reason: reason)

        case WK_WALLET_MANAGER_EVENT_SYNC_RECOMMENDED:
            let depth = WalletManagerSyncDepth(core: core.u.syncRecommended.depth)
            self = .syncRecommended(depth: depth)

        case WK_WALLET_MANAGER_EVENT_BLOCK_HEIGHT_UPDATED:
            self = .blockUpdated(height: core.u.blockHeight)

        default:
            preconditionFailure()

        }
    }
}

///
/// Listener For WalletManagerEvent
///
public protocol WalletManagerListener: AnyObject {
    ///
    /// Handle a WalletManagerEvent.
    ///
    /// - Parameters:
    ///   - system: the system
    ///   - manager: the manager
    ///   - event: the event
    ///
    func handleManagerEvent (system: System,
                             manager: WalletManager,
                             event: WalletManagerEvent)
}

/// A Functional Interface for a Handler
public typealias WalletManagerEventHandler = (System, WalletManager, WalletManagerEvent) -> Void

public protocol WalletManagerFactory { }
