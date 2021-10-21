//
//  BRSystemClient.swift
//  WalletKit
//
//  Created by Ed Gamble on 8/19/20.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
import Foundation  // Data, Date

///
/// An Error is a result of 'submission' to the System Client.  The System Client successfully
/// processed a request and produced a response (processed a 'submisison') but the result of the
/// submission is an error.
///
public enum SystemClientSubmissionError: Error {
    /// The process handling the submit could not be accessed
    case access

    /// Transaction errors, generally
    case account
    case signature
    case insufficientBalance
    case insufficientNetworkFee
    case insufficientNetworkCostUnit
    case insufficientFee
    case nonceTooLow
    case nonceInvalid
    case transactionDuplicate
    case transactionExpired
    case transaction

    /// An unknown, unclassifiable error - see details.
    case unknown
}

///
/// An Error from SystemClient
///
public enum SystemClientError: Error {
    /// The request itself was flawed.  For example, the URL could not be built.
    case badRequest (String)

    /// The request was rejected w/ invalid permission.
    case permission

    /// The request was rejected having exceeded a resource (rateLimit, dataLimit, etc)
    case resource

    /// The response was flawed.  For example response data could not be parsed or was expected but
    /// was not provided.
    case badResponse (String)

    /// The request and response succeeded, but the submission ultimately failed.  For example,
    /// the Client submitted a Transaction to the Ethereum network but the submission failed with
    /// 'gas_too_low'
    case submission (error: SystemClientSubmissionError, details: String)

    /// The client is unavailable.
    case unavailable

    /// The client cannot be reached as network connectivity has been lost
    case lostConnectivity
}

public protocol SystemClient {

    // pause, resume, cancel, ...
    func cancelAll ()

    
    // Blockchain
    
    typealias BlockchainFee = (amount: String, tier: String, confirmationTimeInMilliseconds: UInt64) // currency?
    typealias Blockchain = (
        id: String,
        name: String,
        network: String,
        isMainnet: Bool,
        currency: String,
        blockHeight: UInt64?,
        verifiedBlockHash: String?,
        feeEstimates: [BlockchainFee],
        confirmationsUntilFinal: UInt32)
    
    func getBlockchains (mainnet: Bool?,
                         completion: @escaping (Result<[Blockchain],SystemClientError>) -> Void)
    
    func getBlockchain (blockchainId: String,
                        completion: @escaping (Result<Blockchain,SystemClientError>) -> Void)
    
    
    // Currency
    
    typealias CurrencyDenomination = (name: String, code: String, decimals: UInt8, symbol: String /* extra */)
    typealias Currency = (
        id: String,
        name: String,
        code: String,
        type: String,
        blockchainID: String,
        address: String?,
        verified: Bool,
        demoninations: [CurrencyDenomination])

    func getCurrencies (blockchainId: String?,
                        mainnet: Bool,
                        completion: @escaping (Result<[Currency],SystemClientError>) -> Void)
    
    func getCurrency (currencyId: String,
                      completion: @escaping (Result<Currency,SystemClientError>) -> Void)
    
    // Amount
    
    typealias Amount = (currency: String, value: String)
    
    // Transfer
    
    typealias Transfer = (
        id: String,
        source: String?,
        target: String?,
        amount: Amount,
        acknowledgements: UInt64,
        index: UInt64,
        transactionId: String?,
        blockchainId: String,
        metaData: Dictionary<String,String>?
    )
    
    func getTransfers (blockchainId: String,
                       addresses: [String],
                       begBlockNumber: UInt64,
                       endBlockNumber: UInt64,
                       maxPageSize: Int?,
                       completion: @escaping (Result<[Transfer], SystemClientError>) -> Void)
    
    
    func getTransfer (transferId: String,
                      completion: @escaping (Result<Transfer, SystemClientError>) -> Void)
    
    
    // Transaction
    
    typealias Transaction = (
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
        acknowledgements: UInt64,
        metaData: Dictionary<String,String>?
    )

    typealias TransactionIdentifier = (
        id: String,
        blockchainId: String,
        hash: String?,
        identifier: String
    )

    func getTransactions (blockchainId: String,
                          addresses: [String],
                          begBlockNumber: UInt64?,
                          endBlockNumber: UInt64?,
                          includeRaw: Bool,
                          includeProof: Bool,
                          includeTransfers: Bool,
                          maxPageSize: Int?,
                          completion: @escaping (Result<[Transaction], SystemClientError>) -> Void)
    
    func getTransaction (transactionId: String,
                         includeRaw: Bool,
                         includeProof: Bool,
                         completion: @escaping (Result<Transaction, SystemClientError>) -> Void)
    
    func createTransaction (blockchainId: String,
                            transaction: Data,
                            identifier: String?,
                            completion: @escaping (Result<TransactionIdentifier, SystemClientError>) -> Void)

    // Transaction Fee
    
    typealias TransactionFee = (
        // This is the best estimate of the costUnits needed to include the transaction in the
        // blockchain.  It is does not include margin; it might be an upper limit.
        costUnits: UInt64,
        properties: Dictionary<String,String>?
    )

    func estimateTransactionFee (blockchainId: String,
                                 transaction: Data,
                                 completion: @escaping (Result<TransactionFee, SystemClientError>) -> Void)
    
    // Block
    
    typealias Block = (
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
    
    func getBlocks (blockchainId: String,
                    begBlockNumber: UInt64,
                    endBlockNumber: UInt64,
                    includeRaw: Bool,
                    includeTx: Bool,
                    includeTxRaw: Bool,
                    includeTxProof: Bool,
                    maxPageSize: Int?,
                    completion: @escaping (Result<[Block], SystemClientError>) -> Void)
    
    func getBlock (blockId: String,
                   includeRaw: Bool,
                   includeTx: Bool,
                   includeTxRaw: Bool,
                   includeTxProof: Bool,
                   completion: @escaping (Result<Block, SystemClientError>) -> Void)
    
    
    // Subscription
    
    typealias SubscriptionEndpoint = (environment: String, kind: String, value: String)
    typealias SubscriptionEvent = (name: String, confirmations: [UInt32]) // More?
    typealias SubscriptionCurrency = (addresses: [String], currencyId: String, events: [SubscriptionEvent])
    typealias Subscription = (
        id: String,     // subscriptionId
        device: String, //  devcieId
        endpoint: SubscriptionEndpoint,
        currencies: [SubscriptionCurrency]
    )
    
    func getSubscriptions (completion: @escaping (Result<[SystemClient.Subscription], SystemClientError>) -> Void)
    
    func getSubscription (id: String,
                          completion: @escaping (Result<SystemClient.Subscription, SystemClientError>) -> Void)
    
    func getOrCreateSubscription (_ subscription: SystemClient.Subscription,
                                  completion: @escaping (Result<SystemClient.Subscription, SystemClientError>) -> Void)
    
    func createSubscription (_ subscription: SystemClient.Subscription, // TODO: Hackily
        completion: @escaping (Result<SystemClient.Subscription, SystemClientError>) -> Void)
    
    func updateSubscription (_ subscription: SystemClient.Subscription,
                             completion: @escaping (Result<SystemClient.Subscription, SystemClientError>) -> Void)
    
    func deleteSubscription (id: String,
                             completion: @escaping (Result<Void, SystemClientError>) -> Void)
    
    // TODO: Temporary?
    func subscribe (walletId: String, subscription: Subscription)
    
    // Address
    
    typealias Address = (
        blockchainID: String,
        address: String,
        nonce: UInt64?,
        timestamp: UInt64,
        metaData: Dictionary<String,String>?,
        balances: [Amount]
    )
    
    func getAddresses (blockchainId: String, publicKey: String,
                       completion: @escaping (Result<[Address],SystemClientError>) -> Void)
    
    func getAddress (blockchainId: String, address: String, timestamp: UInt64?,
                     completion: @escaping (Result<Address,SystemClientError>) -> Void)
    
    func createAddress (blockchainId: String, data: Data,
                        completion: @escaping (Result<Address, SystemClientError>) -> Void)
    
    typealias HederaAccount = (
        id: String,
        balance: UInt64?,
        deleted: Bool
    )
    
    
    func getHederaAccount (blockchainId: String,
                           publicKey: String,
                           completion: @escaping (Result<[HederaAccount], SystemClientError>) -> Void)
    
    func createHederaAccount (blockchainId: String,
                              publicKey: String,
                              completion: @escaping (Result<[HederaAccount], SystemClientError>) -> Void)
}

extension SystemClient {
    public func getCurrencies (mainnet: Bool, completion: @escaping (Result<[Currency],SystemClientError>) -> Void) {
        getCurrencies(blockchainId: nil, mainnet: mainnet, completion: completion)
    }
    
    public func getTransactions (blockchainId: String,
                                 addresses: [String],
                                 begBlockNumber: UInt64?,
                                 endBlockNumber: UInt64?,
                                 includeRaw: Bool,
                                 includeTransfers: Bool,
                                 completion: @escaping (Result<[Transaction], SystemClientError>) -> Void) {
        
        getTransactions(blockchainId: blockchainId,
                        addresses: addresses,
                        begBlockNumber: begBlockNumber,
                        endBlockNumber: endBlockNumber,
                        includeRaw: includeRaw,
                        includeProof: false,
                        includeTransfers: includeTransfers,
                        maxPageSize: nil,
                        completion: completion)
    }
    
    public func getBlocks (blockchainId: String,
                           begBlockNumber: UInt64 = 0,
                           endBlockNumber: UInt64 = 0,
                           includeRaw: Bool = false,
                           completion: @escaping (Result<[SystemClient.Block], SystemClientError>) -> Void) {
        getBlocks (blockchainId: blockchainId,
                   begBlockNumber: begBlockNumber,
                   endBlockNumber: endBlockNumber,
                   includeRaw: includeRaw,
                   includeTx: false,
                   includeTxRaw: false,
                   includeTxProof: false,
                   maxPageSize: nil,
                   completion: completion)
    }
    
    public func getBlock (blockId: String,
                          includeRaw: Bool = false,
                          completion: @escaping (Result<SystemClient.Block, SystemClientError>) -> Void) {
        getBlock (blockId: blockId,
                  includeRaw: includeRaw,
                  includeTx: false,
                  includeTxRaw: false,
                  includeTxProof: false,
                  completion: completion)
    }
}
