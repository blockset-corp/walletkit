//
//  WKNetwork.swift
//  WalletKit
//
//  Created by Ed Gamble on 3/27/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
import WalletKitCore

///
/// A Blockchain Network.  Networks are created based from a cross-product of block chain and
/// network type.  Specifically {BTC, BCH, ETH, ...} x {Mainnet, Testnet, ...}.  Thus there will
/// be networks of [BTC-Mainnet, BTC-Testnet, ..., ETH-Mainnet, ETH-Testnet, ETH-Rinkeby, ...]
///
public final class Network: CustomStringConvertible {
    let core: WKNetwork

    /// A unique-identifer-string
    internal let uids: String

    /// The name
    public let name: String

    /// If 'mainnet' then true, otherwise false
    public let onMainnet: Bool

    /// The type of network
    public let type: NetworkType

    /// The current height of the blockChain network.  On a reorganization, this might go backwards.
    /// (No guarantee that this monotonically increases)
    public internal(set) var height: UInt64 {
        get { return wkNetworkGetHeight (core) }
        set { wkNetworkSetHeight (core, newValue) }
    }

    /// The network fees.  Expect the User to select their preferred fee, based on time-to-confirm,
    /// and then have their preferred fee held in WalletManager.defaultNetworkFee.
    public internal(set) var fees: [NetworkFee] {
        get {
            var count: WKCount = 0
            let ptr = wkNetworkGetNetworkFees(core, &count);
            defer { if let p = ptr { wkMemoryFree (p) } }

            let items = ptr?.withMemoryRebound(to: WKNetworkFee.self, capacity: count) {
                Array(UnsafeBufferPointer (start: $0, count: count))
            } ?? []

            return items.map { NetworkFee (core: $0, take: false) }
        }
        set {
            precondition(0 != newValue.count)

            let feeCores: [WKNetworkFee?] = newValue.map { $0.core }
            feeCores.withUnsafeBufferPointer { (buffer: UnsafeBufferPointer<WKNetworkFee?>) -> Void in
                wkNetworkSetNetworkFees(core, buffer.baseAddress!, feeCores.count)
            }
        }
    }

    /// Return the minimum fee which should be the fee with the largest confirmation time
    public var minimumFee: NetworkFee {
        return fees.min { $0.timeIntervalInMilliseconds > $1.timeIntervalInMilliseconds }!
    }

    public var confirmationsUntilFinal: UInt32 {
        return wkNetworkGetConfirmationsUntilFinal (core);
    }

    ///
    /// Create a Network Peer for use in P2P modes when a WalletManager connects.
    ///
    /// - Parameters:
    ///   - address: An numeric-dot-notation IP address
    ///   - port: A port number
    ///   - publicKey: An optional public key
    ///
    /// - Returns: A NetworkPeer if the address correctly parses; otherwise `nil`
    ///
    public func createPeer (address: String, port: UInt16, publicKey: String?) -> NetworkPeer? {
        return NetworkPeer (network: core, address: address, port: port, publicKey: publicKey)
    }

    // Address Schemes

    ///
    /// Return the default AddressScheme for `network`
    ///
    /// - Parameter network: the network
    ///
    /// - Returns: The default AddressScheme
    ///
    public lazy var defaultAddressScheme: AddressScheme = {
        return AddressScheme (core: wkNetworkGetDefaultAddressScheme (core))
    }()

    ///
    /// Return the AddressSchemes support for `network`
    ///
    /// - Parameter network: the network
    ///
    /// - Returns: An array of AddressScheme
    ///
    public lazy var supportedAddressSchemes: [AddressScheme] = {
        var schemesCount: WKCount = 0;
        return wkNetworkGetSupportedAddressSchemes (core, &schemesCount)
            .withMemoryRebound (to: WKAddressScheme.self, capacity: schemesCount) {
                Array (UnsafeBufferPointer (start: $0, count: schemesCount))
        }
        .map { AddressScheme (core: $0) }
    }()

    ///
    /// Check if `network` supports `scheme`
    ///
    /// - Parameters:
    ///   - network: the network
    ///   - scheme: the scheme
    ///
    /// - Returns: If supported `true`; otherwise `false`.
    ///
    public func supportsAddressScheme (_ scheme: AddressScheme) -> Bool {
        return WK_TRUE == wkNetworkSupportsAddressScheme (core, scheme.core)
    }

    // Sync Modes

    ///
    /// Return the default WalletManagerMode for `network`
    ///
    /// - Parameter network: the network
    ///
    /// - Returns: the default mode
    ///
    public lazy var defaultMode: WalletManagerMode = {
        return WalletManagerMode (core: wkNetworkGetDefaultSyncMode(core))
    }()

    ///
    /// Return the WalletManagerModes supported by `network`
    ///
    /// - Parameter network: the network
    ///
    /// - Returns: an aray of WalletManagerMode
    ///
    public lazy var supportedModes : [WalletManagerMode] = {
        var modesCount: WKCount = 0;
        return wkNetworkGetSupportedSyncModes (core, &modesCount)
            .withMemoryRebound(to: WKSyncMode.self, capacity: modesCount) {
                Array (UnsafeBufferPointer (start: $0, count: modesCount))
        }
        .map { WalletManagerMode (core: $0) }
    }()

    ///
    /// Check if `network` supports `mode`
    ///
    /// - Parameters:
    ///   - network: the network
    ///   - mode: the mode
    ///
    /// - Returns: If supported `true`; otherwise `false`
    ///
    public func supportsMode (_ mode: WalletManagerMode) -> Bool {
        return WK_TRUE == wkNetworkSupportsSyncMode (core, mode.core)
    }

    /// The native currency.
    public let currency: Currency

    /// All currencies - at least those we are handling/interested-in.
    public var currencies: Set<Currency> {
        Set ((0..<wkNetworkGetCurrencyCount(core))
            .map { wkNetworkGetCurrencyAt (core, $0) }
            .map { Currency (core: $0, take: false)})
    }

    public func currencyBy (code: String) -> Currency? {
        return wkNetworkGetCurrencyForCode (core, code)
            .map { Currency (core: $0, take: false) }
    }

    public func currencyBy (issuer: String) -> Currency? {
        return wkNetworkGetCurrencyForIssuer (core, issuer)
            .map { Currency (core: $0, take: false) }
    }

    public func hasCurrency(_ currency: Currency) -> Bool {
        return WK_TRUE == wkNetworkHasCurrency (core, currency.core)
    }

    public func baseUnitFor (currency: Currency) -> Unit? {
        guard hasCurrency(currency) else { return nil }
        return wkNetworkGetUnitAsBase (core, currency.core)
            .map { Unit (core: $0, take: false) }
    }

    public func defaultUnitFor (currency: Currency) -> Unit? {
        guard hasCurrency (currency) else { return nil }
        return wkNetworkGetUnitAsDefault (core, currency.core)
            .map { Unit (core: $0, take: false) }
    }

    public func unitsFor (currency: Currency) -> Set<Unit>? {
        guard hasCurrency (currency) else { return nil }
        return Set ((0..<wkNetworkGetUnitCount (core, currency.core))
            .map { wkNetworkGetUnitAt (core, currency.core, $0) }
            .map { Unit (core: $0, take: false) }
        )
    }

    public func hasUnitFor (currency: Currency, unit: Unit) -> Bool? {
        return unitsFor (currency: currency)?.contains(unit)
    }

    internal func addCurrency (_ currency: Currency, baseUnit: Unit, defaultUnit: Unit) {
        precondition (baseUnit.hasCurrency(currency))
        precondition (defaultUnit.hasCurrency(currency))
        if !hasCurrency(currency) {
            wkNetworkAddCurrency (core, currency.core, baseUnit.core, defaultUnit.core)
        }
    }
    
    internal func addUnitFor (currency: Currency, unit: Unit) {
        precondition (hasCurrency(currency))
        precondition (unit.hasCurrency(currency))
        if !hasUnitFor(currency: currency, unit: unit)! { // 'bang' is safe here
            wkNetworkAddCurrencyUnit (core, currency.core, unit.core)
        }
    }

    internal init (core: WKNetwork, take: Bool) {
        self.core = take ? wkNetworkTake(core) : core
        self.uids = asUTF8String (wkNetworkGetUids (core))
        self.name = asUTF8String (wkNetworkGetName (core))
        self.onMainnet  = (WK_TRUE == wkNetworkIsMainnet (core))
        self.type       = NetworkType (core: wkNetworkGetType(core))
        self.currency   = Currency (core: wkNetworkGetCurrency(core), take: false)
    }

    internal static func findBuiltin (uids: String) -> Network? {
        return wkNetworkFindBuiltin(uids, uids.hasSuffix("mainnet"))
            .map { Network (core: $0, take: false) }
    }

//    internal static func installBuiltins (isMainnet: Bool = true) -> [Network] {
//        var builtinCoresCount: Int = 0
//        let builtinCores = wkNetworkInstallBuiltins (&builtinCoresCount, isMainnet)!
//        defer { wkMemoryFree (builtinCores) }
//
//        return builtinCores
//            .withMemoryRebound (to: WKNetwork.self, capacity: builtinCoresCount) {
//                Array (UnsafeBufferPointer (start: $0, count: builtinCoresCount))
//        }
//        .map { Network (core: $0, take: false) }
//    }

    public var description: String {
        return name
    }

    public static func getTypeFromName (name: String) -> NetworkType? {
        var isMainnet: WKBoolean = WK_FALSE
        let core = wkNetworkGetTypeFromName (name, &isMainnet)
        return (core == WKNetworkType (WK_NETWORK_TYPE_UNKNOWN)
            ? nil
            : NetworkType (core: core))
    }

    deinit {
        wkNetworkGive (core)
    }

    @available(*, deprecated, message: "Replace with Address.create(string:network:)")
    public func addressFor (_ string: String) -> Address? {
        return Address.create (string: string, network: self)
    }
}

extension Network: Hashable {
    public static func == (lhs: Network, rhs: Network) -> Bool {
        return lhs.uids == rhs.uids
    }

    public func hash(into hasher: inout Hasher) {
        hasher.combine(uids)
    }
}

///
/// Try as we might, certain functionality outside of WalletKit will require knowing the
/// canonical network type as: BTC, BCH, ETH, etc.  The NetworkType provides this information.
/// Previously we had exposed Currency.code_as_{btc,bch,eth,...} and expected string comparisons
/// to distinguish the network type; not the best approach.  Now we are explicit.
///
/// WalletKit only supports a well-defined set of network types - this because the network
/// must be built in to handle the specfics of accounts, transactions, balances, etc.  This
/// enumeration is extends as new network types are added.
///
public enum NetworkType: CustomStringConvertible {
    case btc
    case bch
    case bsv
    case ltc
    case eth
    case xrp
    case hbar
    case xtz
//    case xlm

    internal init (core: WKNetworkType) {
        switch core {
        case WK_NETWORK_TYPE_BTC:  self = .btc
        case WK_NETWORK_TYPE_BCH:  self = .bch
        case WK_NETWORK_TYPE_BSV:  self = .bsv
        case WK_NETWORK_TYPE_LTC:  self = .ltc
        case WK_NETWORK_TYPE_ETH:  self = .eth
        case WK_NETWORK_TYPE_XRP:  self = .xrp
        case WK_NETWORK_TYPE_HBAR: self = .hbar
        case WK_NETWORK_TYPE_XTZ:  self = .xtz
//        case WK_NETWORK_TYPE_XLM:  self = .xlm
        default: preconditionFailure()
        }
    }

    internal var core: WKNetworkType {
        switch self {
        case .btc: return WK_NETWORK_TYPE_BTC
        case .bch: return WK_NETWORK_TYPE_BCH
        case .bsv: return WK_NETWORK_TYPE_BSV
        case .ltc: return WK_NETWORK_TYPE_LTC
        case .eth: return WK_NETWORK_TYPE_ETH
        case .xrp: return WK_NETWORK_TYPE_XRP
        case .hbar: return WK_NETWORK_TYPE_HBAR
        case .xtz: return WK_NETWORK_TYPE_XTZ
//        case .xml: return WK_NETWORK_TYPE_XLM
        }
    }

    public var description: String {
        return "WK_NETWORK_TYPE_" + asUTF8String (wkBlockChainTypeGetCurrencyCode (core)).uppercased()
    }
}

public enum NetworkEvent {
    /// The network was created.
    case created

    /// The network had its currencies, fees and/or blockHeight updated.
    case updated

    // The netwok had its fees updated.
    case feesUpdated

    // The network had its currencies updated.
    case currenciesUpdated

    case deleted

    init (core: WKNetworkEvent) {
        switch core.type {
        case WK_NETWORK_EVENT_CREATED:
            self = .created

        case WK_NETWORK_EVENT_FEES_UPDATED:
            self = .feesUpdated

        case WK_NETWORK_EVENT_CURRENCIES_UPDATED:
            self = .currenciesUpdated

        case WK_NETWORK_EVENT_DELETED:
            self = .deleted

        default:
            preconditionFailure()
        }
    }
}

///
/// Listener for NetworkEvent
///
public protocol NetworkListener: AnyObject {
    ///
    /// Handle a NetworkEvent
    ///
    /// - Parameters:
    ///   - system: the system
    ///   - network: the network
    ///   - event: the event
    ///
    func handleNetworkEvent (system: System,
                             network: Network,
                             event: NetworkEvent)
}

/// A Functional Interface for a Handler
public typealias NetworkEventHandler = (System, Network, NetworkEvent) -> Void

///
/// A Network Fee represents the 'amount per cost factor' paid to mine a transfer. For BTC this
/// amount is 'SAT/BYTE'; for ETH this amount is 'gasPrice'.  The actual fee for the transfer
/// depends on properties of the transfer; for BTC, the cost factor is 'size in kB'; for ETH, the
/// cost factor is 'gas'.
///
/// A Network supports a variety of fees.  Essentially the higher the fee the more enticing the
/// transfer is to a miner and thus the more quickly the transfer gets into the block chain.
///
/// A NetworkFee is Equatable on the underlying Core representation.  It is natural to compare
/// NetworkFee based on timeIntervalInMilliseconds
///
public final class NetworkFee: Equatable {
    // The Core representation
    internal var core: WKNetworkFee

    /// The estimated time internal for a transaction confirmation.
    public let timeIntervalInMilliseconds: UInt64

    /// The ammount, as a rate on 'cost factor', to pay in network fees for the desired
    /// time internal to confirmation.  The 'cost factor' is blockchain specific - for BTC it is
    /// 'transaction size in kB'; for ETH it is 'gas'.
    internal let pricePerCostFactor: Amount

    /// Initialize from the Core representation
    internal init (core: WKNetworkFee, take: Bool) {
        self.core = (take ? wkNetworkFeeTake(core) : core)
        self.timeIntervalInMilliseconds = wkNetworkFeeGetConfirmationTimeInMilliseconds(core)
        self.pricePerCostFactor = Amount (core: wkNetworkFeeGetPricePerCostFactor (core),
                                          take: false)
    }

    /// Initialize based on the timeInternal and pricePerCostFactor.  Used by BlockchainDB when
    /// parsing a NetworkFee from BlockchainDB.Model.BlockchainFee
    internal convenience init (timeIntervalInMilliseconds: UInt64,
                               pricePerCostFactor: Amount) {
        self.init (core: wkNetworkFeeCreate (timeIntervalInMilliseconds,
                                                 pricePerCostFactor.core,
                                                 pricePerCostFactor.unit.core),
                   take: false)
    }

    deinit {
        wkNetworkFeeGive (core)
    }

    // Equatable using the Core representation
    public static func == (lhs: NetworkFee, rhs: NetworkFee) -> Bool {
        return WK_TRUE == wkNetworkFeeEqual (lhs.core, rhs.core)
    }
}

///
/// A NetworkPeer is a Peer on a Network.  This is optionally used in Peer-to-Peer modes to
/// specfify one or more peers to connect to for network synchronization.  Normally the P2P
/// protocoly dynamically discovers peers and thus NetworkPeer is not commonly used.
///
public final class NetworkPeer: Equatable {
    // The Core representation
    internal let core: WKPeer

    /// The network
    public let network: Network

    /// The address
    public let address: String

    /// The port
    public let port: UInt16

    /// The public key
    public let publicKey: String?

    internal init (core: WKPeer, take: Bool) {
        self.core = (take ? wkPeerTake (core) : core)
        self.network = Network (core: wkPeerGetNetwork (core), take: false)
        self.address = asUTF8String (wkPeerGetAddress (core))
        self.port    = wkPeerGetPort (core)
        self.publicKey = wkPeerGetPublicKey (core)
            .map { asUTF8String ($0) }
    }

    internal convenience init? (network: WKNetwork, address: String, port: UInt16, publicKey: String?) {
        guard let core = wkPeerCreate (network, address, port, publicKey)
            else { return nil }

        self.init (core: core, take: false)
    }

    deinit {
        wkPeerGive (core)
    }

    public static func == (lhs: NetworkPeer, rhs: NetworkPeer) -> Bool {
        return WK_TRUE == wkPeerIsIdentical (lhs.core, rhs.core)
    }
}
