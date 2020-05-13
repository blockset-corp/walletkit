//
//  BRBlockChainDB.swift
//  WalletKit
//
//  Created by Ed Gamble on 3/27/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
import Foundation // DispatchQueue

#if os(Linux)
import FoundationNetworking
#endif

private struct BlockChainDBCapabilities: OptionSet, CustomStringConvertible {
    let rawValue: Int

    static let transferStatusRevert = BlockChainDBCapabilities (rawValue: 1 << 0)
    static let transferStatusReject = BlockChainDBCapabilities (rawValue: 1 << 1)

    var description: String {
        switch rawValue {
        case 1 << 0: return "revert"
        case 1 << 1: return "reject"
        default:     return options.map { $0.description }.joined (separator: ", ")
        }
    }

    var options: [BlockChainDBCapabilities] {
        (0..<2).compactMap {
            switch rawValue & (1 << $0) {
            case 1 << 0: return .transferStatusRevert
            case 1 << 1: return .transferStatusReject
            default: return nil
            }
        }
    }

    static let v2020_03_21: BlockChainDBCapabilities = [
        .transferStatusRevert,
        .transferStatusReject
    ]

    var versionDescription: String {
        switch self {
        case BlockChainDBCapabilities.v2020_03_21: return "application/vnd.blockset.V_2020-03-21+json"
        default: return "application/json"
        }
    }

    static let current = v2020_03_21
}

public class BlockChainDB {
    static fileprivate let capabilities =  BlockChainDBCapabilities.current

    /// Base URL (String) for the BRD BlockChain DB
    let bdbBaseURL: String

    /// Base URL (String) for BRD API Services
    let apiBaseURL: String

    // The seesion to use for DataTaskFunc as in `session.dataTask (with: request, ...)`
    let session: URLSession

    /// A DispatchQueue Used for certain queries that can't be accomplished in the session's data
    /// task.  Such as when multiple request are needed in getTransactions().
    let queue = DispatchQueue.init(label: "BlockChainDB")

    /// A function type that decorates a `request`, handles 'challenges', performs decrypting and/or
    /// uncompression of response data, redirects if requires and creates a `URLSessionDataTask`
    /// for the provided `session`.
    public typealias DataTaskFunc = (URLSession, URLRequest, @escaping (Data?, URLResponse?, Error?) -> Void) -> URLSessionDataTask

    /// A DataTaskFunc for submission to the BRD API
    internal let apiDataTaskFunc: DataTaskFunc

    /// A DataTaskFunc for submission to the BRD BlockChain DB
    internal let bdbDataTaskFunc: DataTaskFunc

    /// A default DataTaskFunc that simply invokes `session.dataTask (with: request, ...)`
    static let defaultDataTaskFunc: DataTaskFunc = {
        (_ session: URLSession,
        _ request: URLRequest,
        _ completionHandler: @escaping (Data?, URLResponse?, Error?) -> Void) -> URLSessionDataTask in
        session.dataTask (with: request, completionHandler: completionHandler)
    }

    ///
    /// A Subscription allows for BlockchainDB 'Asynchronous Notifications'.
    ///
    public struct Subscription {
        
        /// A unique identifier for the subscription
        public let subscriptionId: String

        /// A unique identifier for the device.
        public let deviceId: String

        ///
        /// An endpoint definition allowing the BlockchainDB to 'target' this App.  Allows
        /// for APNS, FCM and other notification systems.  This is an optional value; when set to
        /// .none, any existing notification will be disabled
        ///
        /// environment : { unknown, production, development }
        /// kind        : { unknown, apns, fcm, ... }
        /// value       : For apns/fcm this will be the registration token, apns should be hex-encoded
        public let endpoint: (environment: String, kind: String, value: String)?

        public init (id: String, deviceId: String? = nil, endpoint: (environment: String, kind: String, value: String)?) {
            self.subscriptionId = id
            self.deviceId = deviceId ?? id
            self.endpoint = endpoint
        }
    }

    /// A User-specific identifier - a string representation of a UUIDv4
    internal var walletId: String? = nil

    /// A Subscription specificiation
    internal var subscription: Subscription? = nil

    #if false
    private var modelSubscription: Model.Subscription? {
        guard let _ = walletId,  let subscription = subscription, let endpoint = subscription.endpoint
            else { return nil }

        return (id: subscription.subscriptionId,
                device: subscription.deviceId,
                endpoint: (environment: endpoint.environment, kind: endpoint.kind, value: endpoint.value))
    }
    #endif

    // A BlockChainDB wallet requires at least one 'currency : [<addr>+]' entry.  Create a minimal,
    // default one using a compromised ETH address.  This will get replaced as soon as we have
    // a real ETH account.
    internal static let minimalCurrencies = ["eth" : ["A9De3DBd7D561e67527BC1ECB025C59D53B9F7EF"]]

    internal func subscribe (walletId: String, subscription: Subscription) {
        self.walletId = walletId
        self.subscription = subscription

        // TODO: Update caller System.subscribe
        #if false
        // Subscribing requires a wallet on the BlockChainDD, so start by create the BlockChainDB
        // Model.Wallet and then get or create one on the DB.

        let wallet = (id: walletId, currencies: BlockChainDB.minimalCurrencies)

        getOrCreateWallet (wallet) { (walletRes: Result<Model.Wallet, QueryError>) in
            guard case .success = walletRes
                else { print ("SYS: BDB:Wallet: Missed"); return }

            if let model = self.modelSubscription {
                // If the subscription included an endpoint, then put the subscription on the
                // blockchainDB - via POST or PUT.
                self.getSubscription (id: model.id) { (subRes: Result<Model.Subscription, QueryError>) in
                    switch subRes {
                    case let .success (subscription):
                        self.updateSubscription (subscription) {
                            (resSub: Result<BlockChainDB.Model.Subscription, BlockChainDB.QueryError>) in
                        }

                    case .failure:
                        self.createSubscription (model) {
                            (resSub: Result<BlockChainDB.Model.Subscription, BlockChainDB.QueryError>) in
                        }
                    }
                }
            }

            else {
                // Otherwise delete it.
                self.deleteSubscription(id: subscription.subscriptionId) {
                    (subRes: Result<BlockChainDB.Model.Subscription, BlockChainDB.QueryError>) in
                }
            }
        }
        #endif
    }

    ///
    /// Initialize a BlockChainDB
    ///
    /// - Parameters:
    ///   - session: the URLSession to use.  Defaults to `URLSession (configuration: .default)`
    ///   - bdbBaseURL: the baseURL for the BRD BlockChain DB.  Defaults to "http://blockchain-db.us-east-1.elasticbeanstalk.com"
    ///   - bdbDataTaskFunc: an optional DataTaskFunc for BRD BlockChain DB.  This defaults to
    ///       `session.dataTask (with: request, ...)`
    ///   - apiBaseURL: the baseRUL for the BRD API Server.  Defaults to "https://api.breadwallet.com".
    ///       if this is a DEBUG build then "https://stage2.breadwallet.com" will be used instead.
    ///   - apiDataTaskFunc: an optional DataTaskFunc for BRD API services.  For a non-DEBUG build,
    ///       this function would need to properly authenticate with BRD.  This means 'decorating
    ///       the request' header, perhaps responding to a 'challenge', perhaps decripting and/or
    ///       uncompressing response data.  This defaults to `session.dataTask (with: request, ...)`
    ///       which suffices for DEBUG builds.
    ///
    public init (session: URLSession = URLSession (configuration: .default),
                 bdbBaseURL: String = "https://api.blockset.com",
                 bdbDataTaskFunc: DataTaskFunc? = nil,
                 apiBaseURL: String = "https://api.breadwallet.com",
                 apiDataTaskFunc: DataTaskFunc? = nil) {

        self.session = session

        self.bdbBaseURL = bdbBaseURL
        self.apiBaseURL = apiBaseURL

        self.bdbDataTaskFunc = bdbDataTaskFunc ?? BlockChainDB.defaultDataTaskFunc
        self.apiDataTaskFunc = apiDataTaskFunc ?? BlockChainDB.defaultDataTaskFunc
    }

    // this token has no expiration - testing only.
    public static let createForTestBDBBaseURL = "https://api.blockset.com"
    public static let createForTestBDBToken   = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiI1YjQ1M2VhOC1iOGMxLTQwNTEtODk1MC1jMzE5YmQzMjNiMzQiLCJpYXQiOjE1ODUzNDczMzAsImV4cCI6MTkwMDkzMjAzMCwiYnJkOmN0IjoidXNyIiwiYnJkOmNsaSI6IjY1MTNkOGVjLWM2NDUtNGNkNi1iNDZlLTM3MzM4NGYxMTczMCJ9.PEDGBTSOYaqylQ6Kf6wIdwrNvswneziLO61XTS1AXagjFNkGA_OANGYqw0E-ztOFQAyey4DsOhmUlTQLX5Y3yg"

//    public static let createForTestBDBBaseURL = "http://localhost:8079"
//    public static let createForTestBDBToken   = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzUxMiJ9.eyJzdWIiOiI0YjJkOWNjOS04YjlhLTQ0ZTItYjg4NS0zZDliMDM2MzJjOWUiLCJicmQ6Y3QiOiJjbGkiLCJleHAiOjkyMjMzNzIwMzY4NTQ3NzUsImlhdCI6MTU4Njk2OTIxN30.c_5dl5evlQHqeAzSBrX-a22m99TXDUcTb-yc-i2pan63ndEl1bF6gYxDHXtkrp06GetlORCAGr_pU8nKdxSEjA="

    ///
    /// Create a BlockChainDB using a specified Authorization token.  This is declared 'public'
    /// so that the Crypto Demo can use it.
    ///
    public static func createForTest (bdbBaseURL: String = BlockChainDB.createForTestBDBBaseURL,
                                      bdbToken:   String = BlockChainDB.createForTestBDBToken) -> BlockChainDB {
        return BlockChainDB (bdbBaseURL: bdbBaseURL,
                             bdbDataTaskFunc: { (session, request, completion) -> URLSessionDataTask in
                                var decoratedReq = request
                                decoratedReq.setValue ("Bearer \(bdbToken)", forHTTPHeaderField: "Authorization")
                                return session.dataTask (with: decoratedReq, completionHandler: completion)
        })
    }

    ///
    /// A QueryError subtype of Error
    ///
    /// - url:
    /// - submission:
    /// - noData:
    /// - jsonParse:
    /// - model:
    /// - noEntity:
    ///
    public enum QueryError: Error {
        // HTTP URL build failed
        case url (String)

        // HTTP submission error
        case submission (Error)

        // HTTP response unexpected (typically not 200/OK)
        case response (Int) // includes the status code

        // HTTP submission didn't error but returned no data
        case noData

        // JSON parse failed, generically
        case jsonParse (Error?)

        // Could not convert JSON -> T
        case model (String)

        // JSON entity expected but not provided - e.g. requested a 'transferId' that doesn't exist.
        case noEntity (id: String?)
    }

    ///
    /// The BlockChainDB Model (aka Schema-ish)
    ///
    public struct Model {

        /// Blockchain

        public typealias BlockchainFee = (amount: String, tier: String, confirmationTimeInMilliseconds: UInt64) // currency?
        public typealias Blockchain = (
            id: String,
            name: String,
            network: String,
            isMainnet: Bool,
            currency: String,
            blockHeight: UInt64?,
            feeEstimates: [BlockchainFee],
            confirmationsUntilFinal: UInt32)

        static internal func asBlockchainFee (json: JSON) -> Model.BlockchainFee? {
            guard let confirmationTime = json.asUInt64(name: "estimated_confirmation_in"),
                let amountValue = json.asDict(name: "fee")?["amount"] as? String,
                let _ = json.asDict(name: "fee")?["currency_id"] as? String,
                let tier = json.asString(name: "tier")
                else { return nil }

            return (amount: amountValue, tier: tier, confirmationTimeInMilliseconds: confirmationTime)
        }

        static internal func asBlockchain (json: JSON) -> Model.Blockchain? {
            guard let id = json.asString (name: "id"),
                let name = json.asString (name: "name"),
                let network = json.asString (name: "network"),
                let isMainnet = json.asBool (name: "is_mainnet"),
                let currency = json.asString (name: "native_currency_id"),
                let blockHeight = json.asInt64 (name: "block_height"),
                let confirmationsUntilFinal = json.asUInt32(name: "confirmations_until_final")
                else { return nil }

            guard let feeEstimates = json.asArray(name: "fee_estimates")?
                .map ({ JSON (dict: $0) })
                .map ({ asBlockchainFee (json: $0) }) as? [Model.BlockchainFee]
            else { return nil }

            return (id: id, name: name, network: network, isMainnet: isMainnet, currency: currency,
                    blockHeight: (-1 == blockHeight ? nil : UInt64 (blockHeight)),
                    feeEstimates: feeEstimates,
                    confirmationsUntilFinal: confirmationsUntilFinal)
        }

        /// Currency & CurrencyDenomination

        public typealias CurrencyDenomination = (name: String, code: String, decimals: UInt8, symbol: String /* extra */)
        public typealias Currency = (
            id: String,
            name: String,
            code: String,
            type: String,
            blockchainID: String,
            address: String?,
            verified: Bool,
            demoninations: [CurrencyDenomination])

       static internal func asCurrencyDenomination (json: JSON) -> Model.CurrencyDenomination? {
            guard let name = json.asString (name: "name"),
                let code = json.asString (name: "short_name"),
                let decimals = json.asUInt8 (name: "decimals")
                // let symbol = json.asString (name: "symbol")
                else { return nil }

            let symbol = lookupSymbol (code)

            return (name: name, code: code, decimals: decimals, symbol: symbol)
        }

        static internal let currencySymbols = ["btc":"₿", "eth":"Ξ"]
        static internal func lookupSymbol (_ code: String) -> String {
            return currencySymbols[code] ?? code.uppercased()
        }

        static private let currencyInternalAddress = "__native__"

        static internal func asCurrency (json: JSON) -> Model.Currency? {
            guard let id = json.asString (name: "currency_id"),
                let name = json.asString (name: "name"),
                let code = json.asString (name: "code"),
                let type = json.asString (name: "type"),
                let bid  = json.asString (name: "blockchain_id"),
                let verified = json.asBool(name: "verified")
                else { return nil }

            // Address is optional
            let address = json.asString(name: "address")

            // All denomincations must parse
            guard let demoninations = json.asArray (name: "denominations")?
                .map ({ JSON (dict: $0 )})
                .map ({ asCurrencyDenomination(json: $0)}) as? [Model.CurrencyDenomination]
                else { return nil }
            
            return (id: id, name: name, code: code, type: type,
                    blockchainID: bid,
                    address: (address == currencyInternalAddress ? nil : address),
                    verified: verified,
                    demoninations: demoninations)
        }

        static internal let addressBRDTestnet = "0x7108ca7c4718efa810457f228305c9c71390931a" // testnet
        static internal let addressBRDMainnet = "0x558ec3152e2eb2174905cd19aea4e34a23de9ad6" // mainnet

        /// Amount

        public typealias Amount = (currency: String, value: String)

        static internal func asAmount (json: JSON) -> Model.Amount? {
            guard let currency = json.asString (name: "currency_id"),
                let value = json.asString (name: "amount")
                else { return nil }
            return (currency: currency, value: value)
        }

        /// Transfer

        public typealias Transfer = (
            id: String,
            source: String?,
            target: String?,
            amount: Amount,
            acknowledgements: UInt64,
            index: UInt64,
            transactionId: String?,
            blockchainId: String,
            metaData: Dictionary<String,String>?)

        static internal func asTransfer (json: JSON) -> Model.Transfer? {
            guard let id   = json.asString (name: "transfer_id"),
                let bid    = json.asString (name: "blockchain_id"),
                let index  = json.asUInt64 (name: "index"),
                let amount = json.asDict (name: "amount")
                    .map ({ JSON (dict: $0) })
                    .map ({ asAmount(json: $0) }) as? Amount
                else { return nil }

            // TODO: Resolve if optional or not
            let acks   = json.asUInt64 (name: "acknowledgements") ?? 0
            let source = json.asString (name: "from_address")
            let target = json.asString (name: "to_address")
            let tid    = json.asString (name: "transaction_id")
            let meta   = json.asDict(name: "meta")?.mapValues { return $0 as! String }

            return (id: id, source: source, target: target, amount: amount,
                    acknowledgements: acks, index: index,
                    transactionId: tid, blockchainId: bid,
                    metaData: meta)
        }

        /// Transaction

        public typealias Transaction = (
            id: String,
            blockchainId: String,
            hash: String,
            identifier: String,
            blockHash: String?,
            blockHeight: UInt64?,
            index: UInt64?,
            confirmations: UInt64?,
            status: String,
            size: UInt64,
            timestamp: Date?,
            firstSeen: Date?,
            raw: Data?,
            fee: Amount,
            transfers: [Transfer],
            acknowledgements: UInt64
        )

        static internal func asTransactionValidateStatus (_ status: String) -> Bool {
            switch status {
            case "confirmed",
                 "submitted",
                 "failed":
                return true
            case "reverted":
                return BlockChainDB.capabilities.contains(.transferStatusRevert)
            case "rejected":
                return BlockChainDB.capabilities.contains(.transferStatusReject)
            default:
                return false
            }
        }

        static internal func asTransaction (json: JSON) -> Model.Transaction? {
            guard let id = json.asString(name: "transaction_id"),
                let bid        = json.asString (name: "blockchain_id"),
                let hash       = json.asString (name: "hash"),
                let identifier = json.asString (name: "identifier"),
                let status     = json.asString (name: "status"),
                let size       = json.asUInt64 (name: "size"),
                let fee        = json.asDict (name: "fee")
                    .map ({ JSON (dict: $0) })
                    .map ({ asAmount(json: $0)}) as? Amount,
                asTransactionValidateStatus(status)
                else { return nil }

            // TODO: Resolve if optional or not
            let acks       = json.asUInt64 (name: "acknowledgements") ?? 0
            // TODO: Resolve if optional or not
            let firstSeen     = json.asDate   (name: "first_seen")
            let blockHash     = json.asString (name: "block_hash")
            let blockHeight   = json.asUInt64 (name: "block_height")
            let index         = json.asUInt64 (name: "index")
            let confirmations = json.asUInt64 (name: "confirmations")
            let timestamp     = json.asDate   (name: "timestamp")

            let raw = json.asData (name: "raw")

            // Require "_embedded" : "transfers" as [JSON.Dict]
            guard let transfersJSON = json.asDict (name: "_embedded")?["transfers"] as? [JSON.Dict]
                else { return nil }

            // Require asTransfer is not .none
            guard let transfers = transfersJSON
                .map ({ JSON (dict: $0) })
                .map ({ asTransfer (json: $0) }) as? [Model.Transfer]
                else { return nil }

            return (id: id, blockchainId: bid,
                     hash: hash, identifier: identifier,
                     blockHash: blockHash, blockHeight: blockHeight, index: index, confirmations: confirmations, status: status,
                     size: size, timestamp: timestamp, firstSeen: firstSeen,
                     raw: raw,
                     fee: fee,
                     transfers: transfers,
                     acknowledgements: acks)
        }

        /// Transaction Fee

        public typealias TransactionFee = (
            costUnits: UInt64,
            foo: String
        )

        static internal func asTransactionFee (json: JSON) -> Model.TransactionFee? {
            guard let costUnits = json.asUInt64(name: "cost_units")
                else { return nil }

            return (costUnits: costUnits, foo: "ignore")
        }

        /// Block

        public typealias Block = (
            id: String,
            blockchainId: String,
            hash: String,
            height: UInt64,
            header: String?,
            raw: Data?,
            mined: Date,
            size: UInt64,
            prevHash: String?,
            nextHash: String?, // fees
            transactions: [Transaction]?,
            acknowledgements: UInt64
        )

        static internal func asBlock (json: JSON) -> Model.Block? {
            guard let id = json.asString(name: "block_id"),
                let bid      = json.asString(name: "blockchain_id"),
                let hash     = json.asString (name: "hash"),
                let height   = json.asUInt64 (name: "height"),
                let mined    = json.asDate   (name: "mined"),
                let size     = json.asUInt64 (name: "size")
                else { return nil }

            let acks     = json.asUInt64 (name: "acknowledgements") ?? 0
            let header   = json.asString (name: "header")
            let raw      = json.asData   (name: "raw")
            let prevHash = json.asString (name: "prev_hash")
            let nextHash = json.asString (name: "next_hash")

            let transactions = json.asArray (name: "transactions")?
                .map ({ JSON (dict: $0 )})
                .map ({ asTransaction (json: $0)}) as? [Model.Transaction]  // not quite

            return (id: id, blockchainId: bid,
                    hash: hash, height: height, header: header, raw: raw, mined: mined, size: size,
                    prevHash: prevHash, nextHash: nextHash,
                    transactions: transactions,
                    acknowledgements: acks)
        }

        /// Subscription Endpoint

        public typealias SubscriptionEndpoint = (environment: String, kind: String, value: String)

        static internal func asSubscriptionEndpoint (json: JSON) -> SubscriptionEndpoint? {
            guard let environment = json.asString (name: "environment"),
                let kind = json.asString(name: "kind"),
                let value = json.asString(name: "value")
                else { return nil }

            return (environment: environment, kind: kind, value: value)
        }

        static internal func asJSON (subscriptionEndpoint: SubscriptionEndpoint) -> JSON.Dict {
            return [
                "environment"   : subscriptionEndpoint.environment,
                "kind"          : subscriptionEndpoint.kind,
                "value"         : subscriptionEndpoint.value
            ]
        }


        /// Subscription Event

        public typealias SubscriptionEvent = (name: String, confirmations: [UInt32]) // More?

        static internal func asSubscriptionEvent (json: JSON) -> SubscriptionEvent? {
            guard let name = json.asString(name: "name")
                else { return nil }
            return (name: name, confirmations: [])
        }

        static internal func asJSON (subscriptionEvent: SubscriptionEvent) -> JSON.Dict {
            switch subscriptionEvent.name {
            case "submitted":
                return [
                    "name" : subscriptionEvent.name
                ]
            case "confirmed":
                return [
                    "name"          : subscriptionEvent.name,
                    "confirmations" : subscriptionEvent.confirmations
                ]
            default:
                preconditionFailure()
            }
        }

        /// Subscription Currency

        public typealias SubscriptionCurrency = (addresses: [String], currencyId: String, events: [SubscriptionEvent])

        static internal func asSubscriptionCurrency (json: JSON) -> SubscriptionCurrency? {
            guard let addresses = json.asStringArray (name: "addresses"),
                let currencyId = json.asString (name: "currency_id"),
                let events = json.asArray(name: "events")?
                    .map ({ JSON (dict: $0) })
                    .map ({ asSubscriptionEvent(json: $0) }) as? [SubscriptionEvent] // not quite
                else { return nil }

            return (addresses: addresses, currencyId: currencyId, events: events)
        }

        static internal func asJSON (subscriptionCurrency: SubscriptionCurrency) -> JSON.Dict {
            return [
                "addresses"   : subscriptionCurrency.addresses,
                "currency_id" : subscriptionCurrency.currencyId,
                "events"       : subscriptionCurrency.events.map { asJSON(subscriptionEvent: $0) }
            ]
        }

        /// Subscription

        // TODO: Apparently `currences` can not be empty.
        public typealias Subscription = (
            id: String,     // subscriptionId
            device: String, //  devcieId
            endpoint: SubscriptionEndpoint,
            currencies: [SubscriptionCurrency]
        )

       static internal func asSubscription (json: JSON) -> Subscription? {
            guard let id = json.asString (name: "subscription_id"),
                let device = json.asString (name: "device_id"),
                let endpoint = json.asDict(name: "endpoint")
                    .flatMap ({ asSubscriptionEndpoint (json: JSON (dict: $0)) }),
                let currencies = json.asArray(name: "currencies")?
                    .map ({ JSON (dict: $0) })
                    .map ({ asSubscriptionCurrency (json: $0) }) as? [SubscriptionCurrency]
                else { return nil }

            return (id: id,
                    device: device,
                    endpoint: endpoint,
                    currencies: currencies)
        }

        static internal func asJSON (subscription: Subscription) -> JSON.Dict {
            return [
                "subscription_id" : subscription.id,
                "device_id"       : subscription.device,
                "endpoint"        : asJSON (subscriptionEndpoint: subscription.endpoint),
                "currencies"      : subscription.currencies.map { asJSON (subscriptionCurrency: $0) }
            ]
        }

        /// Address

        public typealias Address = (
            blockchainID: String,
            address: String,
            nonce: UInt64?,
            timestamp: UInt64,
            metaData: Dictionary<String,String>?,
            balances: [Amount]
        )

        static internal func asAddress (json: JSON) -> Address? {
            guard let bid     = json.asString (name: "blockchain_id"),
                let address   = json.asString (name: "address"),
                let timestamp = json.asUInt64 (name: "timestamp"),
                let balances = json.asArray (name: "balances")?
                    .map ({ JSON (dict: $0) })
                    .map ({ asAmount(json: $0)}) as? [Amount]
                else { return nil }

            let nonce = json.asUInt64 (name: "nonce")
            let meta  = json.asDict(name: "meta")?.mapValues { return $0 as! String }

            return (blockchainID: bid, address: address,
                    nonce: nonce, timestamp: timestamp,
                    metaData: meta,
                    balances: balances)
        }

        /// Hedera Account

        public typealias HederaAccount = (
            id: String,
            balance: UInt64?,
            deleted: Bool
        )

        static internal func asHederaAccount (json: JSON) -> HederaAccount? {
            guard let id      = json.asString (name: "account_id"),
                let status    = json.asString (name: "account_status")
                else { return nil }

            let balance   = json.asUInt64 (name: "hbar_balance")

            return (id: id,
                    balance: balance,
                    deleted: "active" != status)
        }

    } // End of Model

    public func getBlockchains (mainnet: Bool? = nil, completion: @escaping (Result<[Model.Blockchain],QueryError>) -> Void) {
        bdbMakeRequest (path: "blockchains", query: mainnet.map { zip (["testnet"], [($0 ? "false" : "true")]) }) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getManyExpected(data: $0, transform: Model.asBlockchain)
            })
        }
    }

    public func getBlockchain (blockchainId: String, completion: @escaping (Result<Model.Blockchain,QueryError>) -> Void) {
        bdbMakeRequest(path: "blockchains/\(blockchainId)", query: nil, embedded: false) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getOneExpected (id: blockchainId, data: $0, transform: Model.asBlockchain)
            })
        }
    }

    public func getCurrencies (blockchainId: String? = nil, verified: Bool = true, completion: @escaping (Result<[Model.Currency],QueryError>) -> Void) {
        let queryKeysBase = [
            blockchainId.map { (_) in "blockchain_id" },
            "verified"]
            .compactMap { $0 } // Remove `nil` from blockchainId

        let queryValsBase: [String] = [
            blockchainId,
            verified.description]
            .compactMap { $0 }  // Remove `nil` from blockchainId

        bdbMakeRequest (path: "currencies", query: zip (queryKeysBase, queryValsBase)) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getManyExpected(data: $0, transform: Model.asCurrency)
            })
        }
    }

    public func getCurrency (currencyId: String, completion: @escaping (Result<Model.Currency,QueryError>) -> Void) {
        bdbMakeRequest (path: "currencies/\(currencyId)", query: nil, embedded: false) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getOneExpected(id: currencyId, data: $0, transform: Model.asCurrency)
            })
        }
    }

    /// Subscription

    internal func makeSubscriptionRequest (path: String, data: JSON.Dict?, httpMethod: String,
                                           completion: @escaping (Result<Model.Subscription, QueryError>) -> Void) {
        makeRequest (bdbDataTaskFunc, bdbBaseURL,
                     path: path,
                     query: nil,
                     data: data,
                     httpMethod: httpMethod) {
                        (res: Result<JSON.Dict, QueryError>) in
                        completion (res.flatMap {
                            Model.asSubscription(json: JSON(dict: $0))
                                .map { Result.success ($0) }
                                ?? Result.failure(QueryError.model("Missed Subscription"))
                        })
        }
    }

    public func getSubscriptions (completion: @escaping (Result<[Model.Subscription], QueryError>) -> Void) {
        bdbMakeRequest (path: "subscriptions", query: nil, embedded: true) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            completion (res.flatMap {
                BlockChainDB.getManyExpected(data: $0, transform: Model.asSubscription)
            })
        }
    }

    public func getSubscription (id: String, completion: @escaping (Result<Model.Subscription, QueryError>) -> Void) {
        bdbMakeRequest (path: "subscriptions/\(id)", query: nil, embedded: false) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getOneExpected (id: id, data: $0, transform: Model.asSubscription)
            })
        }
    }

    public func getOrCreateSubscription (_ subscription: Model.Subscription,
                                         completion: @escaping (Result<Model.Subscription, QueryError>) -> Void) {
        getSubscription(id: subscription.id) {
            (res: Result<BlockChainDB.Model.Subscription, BlockChainDB.QueryError>) in
            if case .success = res { completion (res) }
            else {
                self.createSubscription (subscription, completion: completion)
            }
        }
    }

    public func createSubscription (_ subscription: Model.Subscription, // TODO: Hackily
                                    completion: @escaping (Result<Model.Subscription, QueryError>) -> Void) {
        makeSubscriptionRequest (
            path: "subscriptions",
            data: [
                // We can not use asJSON(Subscription) because that will include the 'id'
                "device_id"       : subscription.device,
                "endpoint"        : BlockChainDB.Model.asJSON (subscriptionEndpoint: subscription.endpoint),
                "currencies"      : subscription.currencies.map { BlockChainDB.Model.asJSON (subscriptionCurrency: $0) }],
            httpMethod: "POST",
            completion: completion)
    }

    public func updateSubscription (_ subscription: Model.Subscription, completion: @escaping (Result<Model.Subscription, QueryError>) -> Void) {
        makeSubscriptionRequest (
            path: "subscriptions/\(subscription.id )",
            data: Model.asJSON (subscription: subscription),
            httpMethod: "PUT",
            completion: completion)
    }

    public func deleteSubscription (id: String, completion: @escaping (Result<Void, QueryError>) -> Void) {
        let path = "subscriptions/\(id)"
        makeRequest (bdbDataTaskFunc, bdbBaseURL,
                     path: path,
                     query: nil,
                     data: nil,
                     httpMethod: "DELETE",
                     deserializer: { (data: Data?) in
                        return (nil == data || 0 == data!.count 
                            ? Result.success (())
                            : Result.failure (QueryError.model ("Unexpected Data on DELETE"))) },
                     completion: completion)
    }

    // Transfers

    static let ADDRESS_COUNT = 50
    static let DEFAULT_MAX_PAGE_SIZE = 20

    public func getTransfers (blockchainId: String,
                              addresses: [String],
                              begBlockNumber: UInt64,
                              endBlockNumber: UInt64,
                              maxPageSize: Int? = nil,
                              completion: @escaping (Result<[Model.Transfer], QueryError>) -> Void) {
        self.queue.async {
            var error: QueryError? = nil
            var results = [Model.Transfer]()

            let maxPageSize = maxPageSize ?? BlockChainDB.DEFAULT_MAX_PAGE_SIZE

            for addresses in addresses.chunked(into: BlockChainDB.ADDRESS_COUNT) {
                if nil != error { break }

                let queryKeys = ["blockchain_id", "start_height", "end_height", "max_page_size"] + Array (repeating: "address", count: addresses.count)
                let queryVals = [blockchainId, begBlockNumber.description, endBlockNumber.description, maxPageSize.description] + addresses

                let semaphore = DispatchSemaphore (value: 0)

                var nextURL: URL? = nil

                self.bdbMakeRequest (path: "transfers", query: zip (queryKeys, queryVals)) {
                    (more: URL?, res: Result<[JSON], QueryError>) in

                    // Append `blocks` with the resulting blocks.
                    results += try! res
                        .flatMap { BlockChainDB.getManyExpected(data: $0, transform: Model.asTransfer) }
                        .recover { error = $0; return [] }.get()

                    nextURL = more

                    semaphore.signal()
                }

                // Wait for the first request
                semaphore.wait()

                // Loop until all 'nextURL' values are queried
                while let url = nextURL, nil == error {
                    self.bdbMakeRequest (url: url, embedded: true, embeddedPath: "transfers") {
                        (more: URL?, res: Result<[JSON], QueryError>) in

                        // Append `transactions` with the resulting transactions.
                        results += try! res
                            .flatMap { BlockChainDB.getManyExpected(data: $0, transform: Model.asTransfer) }
                            .recover { error = $0; return [] }.get()

                        nextURL = more

                        semaphore.signal()
                    }

                    semaphore.wait()
                }
            }

            completion (nil == error
                ? Result.success (results)
                : Result.failure (error!))
        }
    }

    public func getTransfer (transferId: String, completion: @escaping (Result<Model.Transfer, QueryError>) -> Void) {
        bdbMakeRequest (path: "transfers/\(transferId)", query: nil, embedded: false) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getOneExpected (id: transferId, data: $0, transform: Model.asTransfer)
            })
        }
    }

    // Transactions

    public func getTransactions (blockchainId: String,
                                 addresses: [String],
                                 begBlockNumber: UInt64? = nil,
                                 endBlockNumber: UInt64? = nil,
                                 includeRaw: Bool = false,
                                 includeProof: Bool = false,
                                 maxPageSize: Int? = nil,
                                 completion: @escaping (Result<[Model.Transaction], QueryError>) -> Void) {
        // This query could overrun the endpoint's page size (typically 5,000).  If so, we'll need
        // to repeat the request for the next batch.
        self.queue.async {
            var error: QueryError? = nil
            var results = [Model.Transaction]()

            let maxPageSize = maxPageSize ?? BlockChainDB.DEFAULT_MAX_PAGE_SIZE

            let queryKeysBase = [
                "blockchain_id",
                begBlockNumber.map { (_) in "start_height" },
                endBlockNumber.map { (_) in "end_height" },
                "include_proof",
                "include_raw",
                "max_page_size"]
                .compactMap { $0 } // Remove `nil` from {beg,end}BlockNumber

            let queryValsBase: [String] = [
                blockchainId,
                begBlockNumber.map { $0.description },
                endBlockNumber.map { $0.description },
                includeProof.description,
                includeRaw.description,
                maxPageSize.description]
                .compactMap { $0 }  // Remove `nil` from {beg,end}BlockNumber

            let semaphore = DispatchSemaphore (value: 0)
            var nextURL: URL? = nil

            func handleResult (more: URL?, res: Result<[JSON], QueryError>) {
                // Append `transactions` with the resulting transactions.
                results += res
                    .flatMap { BlockChainDB.getManyExpected(data: $0, transform: Model.asTransaction) }
                    .getWithRecovery { error = $0; return [] }

                // Record if more exist
                nextURL = more

                // signal completion
                semaphore.signal()
            }

            for addresses in addresses.chunked(into: BlockChainDB.ADDRESS_COUNT) {
                if nil != error { break }

                let queryKeys = queryKeysBase + Array (repeating: "address", count: addresses.count)
                let queryVals = queryValsBase + addresses

                // Ensure a 'clean' start for this set of addresses
                nextURL = nil
                
                // Make the first request.  Ideally we'll get all the transactions in one gulp
                self.bdbMakeRequest (path: "transactions",
                                     query: zip (queryKeys, queryVals),
                                     completion: handleResult)

                // Wait for the first request
                semaphore.wait()

                // Loop until all 'nextURL' values are queried
                while let url = nextURL, nil == error {
                    self.bdbMakeRequest (url: url,
                                         embedded: true,
                                         embeddedPath: "transactions",
                                         completion: handleResult)

                    // Wait for each subsequent result
                    semaphore.wait()
                }
            }

            completion (nil == error
                ? Result.success (results)
                : Result.failure (error!))
        }
    }

    public func getTransaction (transactionId: String,
                                includeRaw: Bool = false,
                                includeProof: Bool = false,
                                completion: @escaping (Result<Model.Transaction, QueryError>) -> Void) {
        let queryKeys = ["include_proof", "include_raw"]
        let queryVals = [includeProof.description, includeRaw.description]

        bdbMakeRequest (path: "transactions/\(transactionId)", query: zip (queryKeys, queryVals), embedded: false) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getOneExpected (id: transactionId, data: $0, transform: Model.asTransaction)
            })
        }
    }

    public func createTransaction (blockchainId: String,
                                   hashAsHex: String,
                                   transaction: Data,
                                   completion: @escaping (Result<Void, QueryError>) -> Void) {
        let json: JSON.Dict = [
            "blockchain_id": blockchainId,
            "transaction_id": hashAsHex,
            "data" : transaction.base64EncodedString()
        ]

        makeRequest (bdbDataTaskFunc, bdbBaseURL,
                     path: "/transactions",
                     data: json,
                     httpMethod: "POST",
                     deserializer: { (_) in Result.success(()) },
                     completion: completion)
    }

    public func estimateTransactionFee (blockchainId: String,
                                        hashAsHex: String,
                                        transaction: Data,
                                        completion: @escaping (Result<Model.TransactionFee, QueryError>) -> Void) {
        let json: JSON.Dict = [
            "blockchain_id": blockchainId,
            "transaction_id": hashAsHex,
            "data" : transaction.base64EncodedString()
        ]

        makeRequest (bdbDataTaskFunc, bdbBaseURL,
                     path: "/transactions",
                     query: zip(["estimate_fee"], ["true"]),
                     data: json,
                     httpMethod: "POST",
                     // deserializer: { (_) in Result.success(()) },
                     completion: completion)
    }

    // Blocks

    public func getBlocks (blockchainId: String,
                           begBlockNumber: UInt64 = 0,
                           endBlockNumber: UInt64 = 0,
                           includeRaw: Bool = false,
                           includeTx: Bool = false,
                           includeTxRaw: Bool = false,
                           includeTxProof: Bool = false,
                           maxPageSize: Int? = nil,
                           completion: @escaping (Result<[Model.Block], QueryError>) -> Void) {
        self.queue.async {
            var error: QueryError? = nil
            var results = [Model.Block]()

            var queryKeys = ["blockchain_id", "start_height", "end_height",  "include_raw",
                             "include_tx", "include_tx_raw", "include_tx_proof"]

            var queryVals = [blockchainId, begBlockNumber.description, endBlockNumber.description, includeRaw.description,
                             includeTx.description, includeTxRaw.description, includeTxProof.description]

            if let maxPageSize = maxPageSize {
                queryKeys += ["max_page_size"]
                queryVals += [String(maxPageSize)]
            }

            let semaphore = DispatchSemaphore (value: 0)

            var nextURL: URL? = nil

            self.bdbMakeRequest (path: "blocks", query: zip (queryKeys, queryVals)) {
                (more: URL?, res: Result<[JSON], QueryError>) in

                // Append `blocks` with the resulting blocks.
                results += try! res
                    .flatMap { BlockChainDB.getManyExpected(data: $0, transform: Model.asBlock) }
                    .recover { error = $0; return [] }.get()

                nextURL = more

                semaphore.signal()
            }

            // Wait for the first request
            semaphore.wait()

            // Loop until all 'nextURL' values are queried
            while let url = nextURL, nil == error {
                self.bdbMakeRequest (url: url, embedded: true, embeddedPath: "blocks") {
                    (more: URL?, res: Result<[JSON], QueryError>) in

                    // Append `transactions` with the resulting transactions.
                    results += try! res
                        .flatMap { BlockChainDB.getManyExpected(data: $0, transform: Model.asBlock) }
                        .recover { error = $0; return [] }.get()

                    nextURL = more

                    semaphore.signal()
                }

                semaphore.wait()
            }

            completion (nil == error
                ? Result.success (results)
                : Result.failure (error!))
        }
    }

    public func getBlock (blockId: String,
                          includeRaw: Bool = false,
                          includeTx: Bool = false,
                          includeTxRaw: Bool = false,
                          includeTxProof: Bool = false,
                          completion: @escaping (Result<Model.Block, QueryError>) -> Void) {
        let queryKeys = ["include_raw", "include_tx", "include_tx_raw", "include_tx_proof"]

        let queryVals = [includeRaw.description, includeTx.description, includeTxRaw.description, includeTxProof.description]

        bdbMakeRequest (path: "blocks/\(blockId)", query: zip (queryKeys, queryVals), embedded: false) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getOneExpected (id: blockId, data: $0, transform: Model.asBlock)
            })
        }
    }

    // Address

    public func getAddresses (blockchainId: String, publicKey: String,
                              completion: @escaping (Result<[Model.Address],QueryError>) -> Void) {
        let queryKeys = ["blockchain_id", "public_key"]
        let queryVals = [ blockchainId,    publicKey]

        bdbMakeRequest (path: "addresses", query: zip (queryKeys, queryVals)) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getManyExpected(data: $0, transform: Model.asAddress)
            })
        }
    }

    public func getAddress (blockchainId: String, address: String, timestamp: UInt64? = nil,
                            completion: @escaping (Result<Model.Address,QueryError>) -> Void) {
        let queryKeys = ["blockchain_id", timestamp.map { (ignore) in "timestamp" }].compactMap { $0 }
        let queryVals = [ blockchainId,   timestamp?.description].compactMap { $0 }

        bdbMakeRequest (path: "addresses/\(address)", query: zip (queryKeys, queryVals), embedded: false) {
            (more: URL?, res: Result<[JSON], QueryError>) in
            precondition (nil == more)
            completion (res.flatMap {
                BlockChainDB.getOneExpected(id: address, data: $0, transform: Model.asAddress)
            })
        }
    }

    public func createAddress (blockchainId: String, data: Data,
                               completion: @escaping (Result<Model.Address, QueryError>) -> Void) {
        let json: JSON.Dict = [
            "blockchain_id": blockchainId,
            "data" : data.base64EncodedString()
        ]

        makeRequest (bdbDataTaskFunc, bdbBaseURL,
                     path: "/addresses",
                     data: json,
                     httpMethod: "POST") {
                        self.bdbHandleResult ($0, embedded: false, embeddedPath: "") {
                            (more: URL?, res: Result<[JSON], QueryError>) in
                            precondition (nil == more)
                            completion (res.flatMap {
                                BlockChainDB.getOneExpected (id: "POST /addresses",
                                                             data: $0,
                                                             transform: Model.asAddress)
                            })
                        }
        }
    }

    // Experimental - Hedera Account Creation

    private func canonicalizePublicKey (_ publicKey: String) -> String {
        return publicKey.starts(with: "0x") || publicKey.starts(with: "0X")
            ? String (publicKey.dropFirst (2))
            : publicKey
    }

    private func getHederaAccount (blockchainId: String,
                                   publicKey: String,
                                   transactionId: String,
                                   completion: @escaping (Result<[Model.HederaAccount],QueryError>) -> Void) {
        // We don't actually use the `transactionID` through the `GET .../account_transactions`
        // endpoint.  It is more direct to just repeatedly "GET .../accounts"
        // let path = "/_experimental/hedera/account_transactions/\(blockchainId):\(transactionId)"
        let noDataFailure = Result<[Model.HederaAccount],QueryError>.failure(BlockChainDB.QueryError.noData)

        let initialDelayInSeconds  = 2
        let retryPeriodInSeconds   = 5
        let retryDurationInSeconds = 4 * 60
        var retriesRemaining = (retryDurationInSeconds / retryPeriodInSeconds) - 1

        func handleResult (res: Result<[Model.HederaAccount], QueryError>) {
            // On a Result with a QueryError just assume there is no account... and try again.
            let accounts = res.getWithRecovery { (_) in return [] }

            if accounts.count > 0 { completion (Result.success (accounts)) }
            else {
                guard retriesRemaining > 0
                    else { completion (noDataFailure); return }

                retriesRemaining -= 1
                let deadline = DispatchTime (uptimeNanoseconds: DispatchTime.now().uptimeNanoseconds + UInt64(1_000_000_000 * retryPeriodInSeconds))
                self.queue.asyncAfter (deadline: deadline) {
                    self.getHederaAccount(blockchainId: blockchainId,
                                          publicKey: publicKey,
                                          completion: handleResult)
                }
            }
        }

        let deadline = DispatchTime (uptimeNanoseconds: DispatchTime.now().uptimeNanoseconds + UInt64(1_000_000_000 * initialDelayInSeconds))
        self.queue.asyncAfter (deadline: deadline) {
            self.getHederaAccount(blockchainId: blockchainId,
                             publicKey: publicKey,
                             completion: handleResult)
        }
    }

    public func getHederaAccount (blockchainId: String,
                                  publicKey: String,
                                  completion: @escaping (Result<[Model.HederaAccount], QueryError>) -> Void) {
        let queryKeys = ["blockchain_id", "pub_key"  ]
        let queryVals = [ blockchainId,    canonicalizePublicKey (publicKey) ]

        makeRequest (bdbDataTaskFunc, bdbBaseURL,
                     path: "/_experimental/hedera/accounts",
                     query: zip (queryKeys, queryVals),
                     data: nil,
                     httpMethod: "GET") {
                        (res: Result<JSON.Dict, QueryError>) in
                        self.bdbHandleResult (res, embeddedPath: "accounts") {
                            (ignore, res: Result<[BlockChainDB.JSON], BlockChainDB.QueryError>) in
                            completion (res.flatMap {
                                BlockChainDB.getManyExpected(data: $0, transform: Model.asHederaAccount)
                            })
                        }
        }
    }

    public func createHederaAccount (blockchainId: String,
                                     publicKey: String,
                                     completion: @escaping (Result<[Model.HederaAccount], QueryError>) -> Void) {
        let noDataFailure = Result<[Model.HederaAccount],QueryError>.failure(BlockChainDB.QueryError.noData)

        let publicKey = canonicalizePublicKey (publicKey)

        let postData: JSON.Dict = [
            "blockchain_id": blockchainId,
            "pub_key": publicKey
        ]

        // Make a POST request to `/_experimental/hedera/accounts`.  On success a "transaction_id"
        // will be returned, in the JSON reponse data, that can be repeatedly queried for the
        // created AccountID.  On failure, if the POST produced a '422' status, then the AccountID
        // already exists and can be queried.
        makeRequest (bdbDataTaskFunc, bdbBaseURL,
                     path: "/_experimental/hedera/accounts",
                     data: postData,
                     httpMethod: "POST") {
                        (res: Result<JSON.Dict, QueryError>) in
                        switch res {
                        case .failure (let error):
                            // If a reponse error with HTTP status of 422, the Hedera accont
                            // already exists.  Just get it.
                            if case let .response (code) = error, code == 422 {
                                self.getHederaAccount (blockchainId: blockchainId,
                                                       publicKey: publicKey,
                                                       completion: completion)
                            }
                            else {
                                completion (Result.failure(error))
                            }
                        case .success (let dict ):
                            let json = JSON (dict: dict)

                            guard let transactionId = json.asString(name: "transaction_id")
                                else { completion (noDataFailure); return }

                            self.getHederaAccount (blockchainId: blockchainId,
                                                   publicKey: publicKey,
                                                   transactionId: transactionId,
                                                   completion: completion)
                        }
        }
    }

    /// BTC - nothing

    /// ETH

    /// The ETH JSON_RPC request identifier.
    var rid: UInt32 = 0

    /// Return the current request identifier and then increment it.
    var ridIncr: UInt32 {
        let rid = self.rid
        self.rid += 1
        return rid
    }

     static internal let dateFormatter: DateFormatter = {
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyy-MM-dd'T'HH:mm:ss.SSSZ"
        return formatter
    }()

    internal struct JSON {
        typealias Dict = [String:Any]

        let dict: Dict

        init (dict: Dict) {
            self.dict = dict
        }

        internal func asString (name: String) -> String? {
            return dict[name] as? String
        }

        internal func asBool (name: String) -> Bool? {
            return dict[name] as? Bool
        }

        internal func asInt64 (name: String) -> Int64? {
            return (dict[name] as? NSNumber)
                .flatMap { Int64 (exactly: $0)}
        }

        internal func asUInt64 (name: String) -> UInt64? {
            return (dict[name] as? NSNumber)
                .flatMap { UInt64 (exactly: $0)}
        }

        internal func asUInt32 (name: String) -> UInt32? {
            return (dict[name] as? NSNumber)
                .flatMap { UInt32 (exactly: $0)}
        }

        internal func asUInt8 (name: String) -> UInt8? {
            return (dict[name] as? NSNumber)
                .flatMap { UInt8 (exactly: $0)}
        }

        internal func asDate (name: String) -> Date? {
            return (dict[name] as? String)
                .flatMap { dateFormatter.date (from: $0) }
        }

        internal func asData (name: String) -> Data? {
            return (dict[name] as? String)
                .flatMap { Data (base64Encoded: $0)! }
        }

        internal func asArray (name: String) -> [Dict]? {
            return dict[name] as? [Dict]
        }

        internal func asDict (name: String) -> Dict? {
            return dict[name] as? Dict
        }

        internal func asStringArray (name: String) -> [String]? {
            return dict[name] as? [String]
        }

        internal func asJSON (name: String) -> JSON? {
            return asDict(name: name).map { JSON (dict: $0) }
        }
    }

    private static func deserializeAsJSON<T> (_ data: Data?) -> Result<T, QueryError> {
        guard let data = data else {
            return Result.failure (QueryError.noData);
        }

        do {
            guard let json = try JSONSerialization.jsonObject(with: data, options: []) as? T
                else {
                    print ("SYS: BDB:API: ERROR: JSON.Dict: '\(data.map { String(format: "%c", $0) }.joined())'")
                    return Result.failure(QueryError.jsonParse(nil)) }

            return Result.success (json)
        }
        catch let jsonError as NSError {
            print ("SYS: BDB:API: ERROR: JSON.Error: '\(data.map { String(format: "%c", $0) }.joined())'")
            return Result.failure (QueryError.jsonParse (jsonError))
        }
    }

    private func sendRequest<T> (_ request: URLRequest,
                                 _ dataTaskFunc: DataTaskFunc,
                                 _ responseSuccess: [Int],
                                 deserializer: @escaping (_ data: Data?) -> Result<T, QueryError>,
                                 completion: @escaping (Result<T, QueryError>) -> Void) {
        dataTaskFunc (session, request) { (data, res, error) in
            guard nil == error else {
                completion (Result.failure(QueryError.submission (error!))) // NSURLErrorDomain
                return
            }

            guard let res = res as? HTTPURLResponse else {
                completion (Result.failure (QueryError.url ("No Response")))
                return
            }

            guard responseSuccess.contains(res.statusCode) else {
                completion (Result.failure (QueryError.response(res.statusCode)))
                return
            }

            completion (deserializer (data))
            }.resume()
    }

    /// Update `request` with 'application/json' headers and the httpMethod
    internal func decorateRequest (_ request: inout URLRequest, httpMethod: String) {
        request.addValue (BlockChainDB.capabilities.versionDescription, forHTTPHeaderField: "Accept")
        request.addValue ("application/json", forHTTPHeaderField: "Content-Type")
        request.httpMethod = httpMethod
    }

    /// https://tools.ietf.org/html/rfc7231#page-24
    internal func responseSuccess (_ httpMethod: String) -> [Int] {
        switch httpMethod {
        case "GET":
            //            The 200 (OK) status code indicates that the request has succeeded.
            return [200]

        case "POST":
            //            If one or more resources has been created on the origin server as a
            //            result of successfully processing a POST request, the origin server
            //            SHOULD send a 201 (Created) response containing a Location header
            //            field that provides an identifier for the primary resource created
            //            (Section 7.1.2) and a representation that describes the status of the
            //            request while referring to the new resource(s).
            //
            //            Responses to POST requests are only cacheable when they include
            //            explicit freshness information (see Section 4.2.1 of [RFC7234]).
            //            However, POST caching is not widely implemented.  For cases where an
            //            origin server wishes the client to be able to cache the result of a
            //            POST in a way that can be reused by a later GET, the origin server
            //            MAY send a 200 (OK) response containing the result and a
            //            Content-Location header field that has the same value as the POST's
            //            effective request URI (Section 3.1.4.2).
            return [200, 201]

        case "DELETE":
            //            If a DELETE method is successfully applied, the origin server SHOULD
            //            send a 202 (Accepted) status code if the action will likely succeed
            //            but has not yet been enacted, a 204 (No Content) status code if the
            //            action has been enacted and no further information is to be supplied,
            //            or a 200 (OK) status code if the action has been enacted and the
            //            response message includes a representation describing the status.
            return [200, 202, 204]

        case "PUT":
            //            If the target resource does not have a current representation and the
            //            PUT successfully creates one, then the origin server MUST inform the
            //            user agent by sending a 201 (Created) response.  If the target
            //            resource does have a current representation and that representation
            //            is successfully modified in accordance with the state of the enclosed
            //            representation, then the origin server MUST send either a 200 (OK) or
            //            a 204 (No Content) response to indicate successful completion of the
            //            request.
            return [200, 201, 204]

        default:
            return [200]
        }
    }

    /// Make a reqeust but w/o the need to create a URL.  Just create a URLRequest, decorate it,
    /// and then send it off.
    internal func makeRequest<T> (_ dataTaskFunc: DataTaskFunc,
                                  url: URL,
                                  httpMethod: String = "POST",
                                  deserializer: @escaping (_ data: Data?) -> Result<T, QueryError> = deserializeAsJSON,
                                  completion: @escaping (Result<T, QueryError>) -> Void) {
        var request = URLRequest (url: url)
        decorateRequest(&request, httpMethod: httpMethod)
        sendRequest (request, dataTaskFunc, responseSuccess (httpMethod), deserializer: deserializer, completion: completion)
    }

    /// Make a request by building a URL request from baseURL, path, query and data.  Once we have
    /// a request, decorate it and then send it off.
    internal func makeRequest<T> (_ dataTaskFunc: DataTaskFunc,
                                  _ baseURL: String,
                                  path: String,
                                  query: Zip2Sequence<[String],[String]>? = nil,
                                  data: JSON.Dict? = nil,
                                  httpMethod: String = "POST",
                                  deserializer: @escaping (_ data: Data?) -> Result<T, QueryError> = deserializeAsJSON,
                                  completion: @escaping (Result<T, QueryError>) -> Void) {
        guard var urlBuilder = URLComponents (string: baseURL)
            else { completion (Result.failure(QueryError.url("URLComponents"))); return }

        urlBuilder.path = path.starts(with: "/") ? path : "/\(path)"
        if let query = query {
            urlBuilder.queryItems = query.map { URLQueryItem (name: $0, value: $1) }
        }

        guard let url = urlBuilder.url
            else { completion (Result.failure (QueryError.url("URLComponents.url"))); return }

        print ("SYS: BDB: Request: \(url.absoluteString): Method: \(httpMethod): Data: \(data?.description ?? "[]")")

        var request = URLRequest (url: url)
        decorateRequest(&request, httpMethod: httpMethod)

        // If we have data as a JSON.Dict, then add it as the httpBody to the request.
        if let data = data {
            do { request.httpBody = try JSONSerialization.data (withJSONObject: data, options: []) }
            catch let jsonError as NSError {
                completion (Result.failure (QueryError.jsonParse(jsonError)))
            }
        }

        sendRequest (request, dataTaskFunc, responseSuccess (httpMethod), deserializer: deserializer, completion: completion)
    }

    /// We have two flavors of bdbMakeRequest but they both handle their result identically.
    /// Provide this helper function to process the JSON result to extract the content and then
    /// to call the completion handler.
    internal func bdbHandleResult (_ res: Result<JSON.Dict, QueryError>,
                                   embedded: Bool = true,
                                   embeddedPath path: String,
                                   completion: @escaping (URL?, Result<[JSON], QueryError>) -> Void) {
        let res = res.map { JSON (dict: $0) }

        // Determine is there are more results for this query.  The BlockChainDB
        // will provide a "_links" JSON dictionary with a "next" field that provides
        // a URL to use for the remaining values.  The "_links" dictionary looks
        // like
        // "_links":{ "next": { "href": <url> },
        //            "self": { "href": <url> }}

        let moreURL = try? res
            .map {  $0.asJSON (name: "_links") }
            .map { $0?.asJSON (name: "next")   }
            .map { $0?.asString(name: "href")  }      // -> Result<String?, ...>
            .map { $0.flatMap { URL (string: $0) } }  // -> Result<URL?,    ...>
            .recover { (ignore) in return nil }       // -> ...
            .get ()
        // moreURL will be `nil` if `res` was not .success

        // Invoke the callback with `moreURL` and Result with [JSON]
        completion (moreURL,
                    res.flatMap { (json: JSON) -> Result<[JSON], QueryError> in
                        let json = (embedded
                            ? (json.asDict(name: "_embedded")?[path] ?? [])
                            : [json.dict])

                        guard let data = json as? [JSON.Dict]
                            else { return Result.failure(QueryError.model ("[JSON.Dict] expected")) }

                        return Result.success (data.map { JSON (dict: $0) })
        })
    }

    /// In the case where a BDB request has 'paged' (with more results than and be returned in
    /// one query, the BDB will give us a URL to use for the next page.  Thus this function
    /// is identical to the following bdbMakeReqeust(path:query:embedded:completion) except that
    /// instead of building a URL, we've got a URL.  In this function, we need to pass in
    /// the 'embeddedPath' so that the JSON parser can find the data.
    internal func bdbMakeRequest (url: URL,
                                  embedded: Bool = true,
                                  embeddedPath: String,
                                  completion: @escaping (URL?, Result<[JSON], QueryError>) -> Void) {
        makeRequest(bdbDataTaskFunc, url: url, httpMethod: "GET") {
            self.bdbHandleResult ($0, embedded: embedded, embeddedPath: embeddedPath, completion: completion)
        }
    }

    internal func bdbMakeRequest (path: String,
                                  query: Zip2Sequence<[String],[String]>?,
                                  embedded: Bool = true,
                                  completion: @escaping (URL?, Result<[JSON], QueryError>) -> Void) {
        makeRequest (bdbDataTaskFunc, bdbBaseURL,
                     path: path,
                     query: query,
                     data: nil,
                     httpMethod: "GET") {
                        self.bdbHandleResult ($0, embedded: embedded, embeddedPath: path, completion: completion)
        }
    }

    ///
    /// Convert an array of JSON into a single value using a specified transform
    ///
    /// - Parameters:
    ///   - id: If no value exists, report QueryError.NoEntity (id: id)
    ///   - data: The array of JSON
    ///   - transform: Function to tranfrom JSON -> T?
    ///
    /// - Returns: A `Result` with success of `T`
    ///
    private static func getOneExpected<T> (id: String, data: [JSON], transform: (JSON) -> T?) -> Result<T, QueryError> {
        switch data.count {
        case  0:
            return Result.failure (QueryError.noEntity(id: id))
        case  1:
            guard let transfer = transform (data[0])
                else { return Result.failure (QueryError.model ("(JSON) -> T transform error (one)"))}
            return Result.success (transfer)
        default:
            return Result.failure (QueryError.model ("(JSON) -> T expected one only"))
        }
    }

    ///
    /// Convert an array of JSON into an array of `T` using a specified transform.  If any
    /// individual JSON cannot be converted, then a QueryError is return for `Result`
    ///
    /// - Parameters:
    ///   - data: Array of JSON
    ///   - transform: Function to transform JSON -> T?
    ///
    /// - Returns: A `Result` with success of `[T]`
    ///
    private static func getManyExpected<T> (data: [JSON], transform: (JSON) -> T?) -> Result<[T], QueryError> {
        let results = data.map (transform)
        return results.contains(where: { $0 == nil })
            ? Result.failure(QueryError.model ("(JSON) -> T transform error (many)"))
            : Result.success(results as! [T])
    }

    ///
    /// Given JSON extract a value and then apply a completion
    ///
    /// - Parameters:
    ///   - extract: A function that extracts the "result" field from JSON to return T?
    ///   - completion: A function to process a Result on T
    ///
    /// - Returns: A function to process a Result on JSON
    ///
    private static func getOneResult<T> (_ extract: @escaping (JSON) -> (String) ->T?,
                                         _ completion: @escaping (Result<T,QueryError>) -> Void) -> ((Result<JSON,QueryError>) -> Void) {
        return { (res: Result<JSON,QueryError>) in
            completion (res.flatMap {
                extract ($0)("result").map { Result.success ($0) } // extract()() returns an optional
                    ?? Result<T,QueryError>.failure(QueryError.noData) })
        }
    }


    /// Given JSON extract a value with JSON.asString (returning String?) and then apply a completion
    ///
    /// - Parameter completion: A function to process a Result on String
    ///
    /// - Returns: A function to process a Result on JSON
    ///
    private static func getOneResultString (_ completion: @escaping (Result<String,QueryError>) -> Void) -> ((Result<JSON,QueryError>) -> Void) {
        return getOneResult (JSON.asString, completion)
    }
}
