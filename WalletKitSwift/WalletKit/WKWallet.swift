//
//  WKWallet.swift
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
/// A Wallet holds the transfers and a balance for a single currency.
///
public final class Wallet: Equatable {

    /// The Core representation
    internal let core: WKWallet

    /// The owning manager
    public let manager: WalletManager

    /// The owning system
    public var system: System {
        return manager.system
    }

    internal var callbackCoordinator: SystemCallbackCoordinator
    
    /// The unit for display of the wallet's balance
    public let unit: Unit

    /// The currency held in wallet (as balance).
    public var currency: Currency {
        return unit.currency
    }

    public let unitForFee: Unit

    /// The (default) name derived from the currency.  For example: BTC, ETH, or BRD.
    public var name: String {
        return unit.currency.code
    }

    /// The current balance for currency
    public var balance: Amount {
        return Amount (core: wkWalletGetBalance (core), take: false)
    }

    /// The maximum balance
    public var balanceMaximum: Amount? {
        return wkWalletGetBalanceMaximum (core)
            .map { Amount (core: $0, take: false) }
    }

    /// The minimum balance
    public var balanceMinimum: Amount? {
        return wkWalletGetBalanceMinimum (core)
            .map { Amount (core: $0, take: false) }
    }

    /// The current state.
    public var state: WalletState {
        return WalletState (core: wkWalletGetState(core))
    }

    /// The default TransferFactory for creating transfers.
    ///    var transferFactory: TransferFactory { get set }

    /// An address suitable for a transfer target (receiving).  Uses the default Address Scheme
    public var target: Address {
        return targetForScheme (manager.addressScheme)
    }

    public func targetForScheme (_ scheme: AddressScheme) -> Address {
        return Address (core: wkWalletGetAddress (core, scheme.core), take: false)
    }

    /// TODO: `var {targets,sources}: [Address]` - for query needs?

    ///
    /// Check if `address` is in `wallet`.  The address is considered in wallet if: a) it
    /// has been used in a transaction or b) is the target address
    ///
    /// - Parameter address: the address to check
    ///
    public func hasAddress (_ address: Address) -> Bool {
        return wkWalletHasAddress (core, address.core);
    }

    ///
    /// The Set of TransferAttributes applicable to Transfers created for this Wallet.  Every
    /// attribute in the returned Set has a `nil` value.  Pass a subset of these to the
    /// `createTransfer()` function.  Transfer creation and attribute validation will fail if
    /// any of the _required_ attributes have a `nil` value or if any `value` is not valid itself.
    ///
    public lazy private(set) var transferAttributes: Set<TransferAttribute> = {
        return transferAttributesFor(target: nil)
    }()

    public func transferAttributesFor (target: Address?) -> Set<TransferAttribute> {
        let coreAttributes = (0..<wkWalletGetTransferAttributeCount(core, target?.core))
            .map { wkWalletGetTransferAttributeAt (core, target?.core, $0)! }
        defer { coreAttributes.forEach (wkTransferAttributeGive) }

        return Set (coreAttributes.map { TransferAttribute (core: wkTransferAttributeCopy($0), take: false) })
    }
    ///
    /// Validate a TransferAttribute.  This returns `true` if the attributes value is valid and,
    /// if the attribute's value is required, if is it not `nil`.
    ///
    /// - Parameter attribute: The attribute to validate
    ///
    public func validateTransferAttribute (_ attribute: TransferAttribute) -> TransferAttributeValidationError? {
        let coreAttribute = attribute.core

        var validates: WKBoolean = WK_TRUE
        let error = wkWalletValidateTransferAttribute (core, coreAttribute, &validates)
        return WK_TRUE == validates ? nil : TransferAttributeValidationError (core: error)
    }

    ///
    /// Validate a Set of TransferAttributes.  This should be called prior to `createTransfer`
    /// (otherwise `createTransfer` will fail).  This checks the Set as a whole given that their
    /// might be relationships between the attributes
    ///
    /// - Note: Relationships between attributes are not explicitly provided in the interface
    ///
    /// - Parameter attributes: the set of attributes to validate
    ///
    public func validateTransferAttributes (_ attributes: Set<TransferAttribute>) -> TransferAttributeValidationError? {
        let coreAttributesCount = attributes.count
        var coreAttributes: [WKTransferAttribute?] = attributes.map { $0.core }

        var validates: WKBoolean = WK_TRUE
        let error = wkWalletValidateTransferAttributes (core,
                                                            coreAttributesCount,
                                                            &coreAttributes,
                                                            &validates)
        return WK_TRUE == validates ? nil : TransferAttributeValidationError (core: error)
    }


    /// The transfers of currency yielding `balance`
    public var transfers: [Transfer] {
        var transfersCount: WKCount = 0
        let transfersPtr = wkWalletGetTransfers(core, &transfersCount);
        defer { if let ptr = transfersPtr { wkMemoryFree (ptr) } }
        
        let transfers: [WKTransfer] = transfersPtr?
            .withMemoryRebound(to: WKTransfer.self, capacity: transfersCount) {
                Array(UnsafeBufferPointer (start: $0, count: transfersCount))
            } ?? []
        
        return transfers
            .map { Transfer (core: $0,
                             wallet: self,
                             take: false) }
    }

    /// Use a hash to lookup a transfer
    public func transferBy (hash: TransferHash) -> Transfer? {
        return transfers
            .first { $0.hash.map { $0 == hash } ?? false }
    }

    internal func transferBy (core: WKTransfer) -> Transfer? {
        return (WK_FALSE == wkWalletHasTransfer (self.core, core)
            ? nil
            : Transfer (core: core,
                        wallet: self,
                        take: true))
    }

    internal func transferByCoreOrCreate (_ core: WKTransfer,
                                          create: Bool = false) -> Transfer? {
        return transferBy (core: core) ??
            (!create
                ? nil
                : Transfer (core: core,
                            wallet: self,
                            take: true))
    }

    // address scheme

    ///
    /// Create a transfer for wallet.  If attributes are provided and they don't validate, then
    /// `nil` is returned.  Creation will fail if the amount exceeds the wallet's balance.
    ///
    /// Generates events: TransferEvent.created and WalletEvent.transferAdded(transfer).
    ///
    /// - Parameters:
    ///   - target: The target receives 'amount
    ///   - amount: The amount
    ///   - estimatedFeeBasis: The basis for 'fee'
    ///   - attributes: Optional transfer attributes.
    ///
    /// - Returns: A new transfer
    ///
    public func createTransfer (target: Address,
                                amount: Amount,
                                estimatedFeeBasis: TransferFeeBasis,
                                attributes: Set<TransferAttribute>? = nil) -> Transfer? {
        if nil != attributes && nil != self.validateTransferAttributes(attributes!) {
            return nil
        }

        let coreAttributesCount = attributes?.count ?? 0
        var coreAttributes: [WKTransferAttribute?] = attributes?.map { $0.core } ?? []
        
        return wkWalletCreateTransfer (core,
                                           target.core,
                                           amount.core,
                                           estimatedFeeBasis.core,
                                           coreAttributesCount,
                                           &coreAttributes)
            .map { Transfer (core: $0,
                             wallet: self,
                             take: false)
            }
    }

    public func createTransfer (outputs: [TransferOutput],
                                estimatedFeeBasis: TransferFeeBasis) -> Transfer? {
        let coreOutputsCount = outputs.count

        switch coreOutputsCount {
        case 0: return nil
        case 1: return createTransfer (target: outputs[0].target,
                                       amount: outputs[0].amount,
                                       estimatedFeeBasis: estimatedFeeBasis)
        default:
            var coreOutputs = outputs.map { $0.core }
            
            return wkWalletCreateTransferMultiple (core,
                                                       coreOutputsCount,
                                                       &coreOutputs,
                                                       estimatedFeeBasis.core)
                .map { Transfer (core: $0,
                                 wallet: self,
                                 take: false)
                }
        }
    }

    internal func createTransfer(sweeper: WalletSweeper,
                                 estimatedFeeBasis: TransferFeeBasis) -> Transfer? {
        return wkWalletSweeperCreateTransferForWalletSweep(sweeper.core, manager.core, self.core, estimatedFeeBasis.core)
            .map { Transfer (core: $0,
                             wallet: self,
                             take: false)
        }
    }

    internal func createTransfer(request: PaymentProtocolRequest,
                                 estimatedFeeBasis: TransferFeeBasis) -> Transfer? {
        return wkWalletCreateTransferForPaymentProtocolRequest(self.core, request.core, estimatedFeeBasis.core)
            .map { Transfer (core: $0,
                             wallet: self,
                             take: false)
        }
    }
    
    /// MARK: Estimate Limit

    func hackTheAmountIfTezos (amount: Amount) -> Amount {
        let network = self.manager.network
        switch network.type {
        case .xtz:
            // See BRTezosOperation.c
            let TEZOS_FEE_DEFAULT: Int64 = 0

            let unitBase   = network.baseUnitFor(currency: amount.currency)!
            let amountSlop = Amount.create (integer: 1 + TEZOS_FEE_DEFAULT, unit: unitBase)

            //
            // A Tezos fee estimation for an amount such that:
            //     `(balance - TEZOS_FEE_DEFAULT) <= amount <= balance`
            // will return "balance_too_low" but you can actually send a tranaction with roughly
            //     `amount < (balance - 424)`
            // where 424 is the fee for 1mutez (424 is typical)
            //
            // So, if asked to perform a fee estimate for an amount within TEZOS_FEE_DEFAULT of
            // balance we'll instead use an amount of (balance - TEZOS_FEE_DEFAULT - 1).  Note: if
            // balance < TEZOS_FEE_DEFAULT, we'll use an amout of 1.
            //
            return (self.balance > (amount + amountSlop)!
                        ? amount
                        : (self.balance > amountSlop
                                ? (self.balance - amountSlop)!
                                : Amount.create (integer: 1, unit: unitBase)))

        default:
            return amount
        }
    }

    ///
    /// A `Wallet.EstimateLimitHandler` is a function th handle the result of `Wallet.estimateLimit`
    /// with return type of `Amount`.
    ///
    public typealias EstimateLimitHandler = (Result<Amount,LimitEstimationError>) -> Void

    ///
    /// Estimate the maximum amount that can be transfered from Wallet.  This value does not
    /// include the fee, however, a fee estimate has been performed and the maximum has been
    /// adjusted to be (nearly) balance = amount + fee.  That is, the maximum amount is what you
    /// can safe transfer to 'zero out' the wallet
    ///
    /// In cases where `balance < fee` then .insufficientFunds is returned.  This can occur for
    /// an ERC20 transfer where the ETH wallet's balance is not enough to pay the fee.  That is,
    /// the .insufficientFunds check respects the wallet from which fees are extracted.  Both
    /// BTC and ETH transfer might have an insufficient balance to pay a fee.
    ///
    /// This is an synchronous function that returns immediately but will call `completion` once
    /// the maximum has been determined.
    ///
    /// The returned Amount is always in the wallet's currencyh.
    ///
    /// - Parameters:
    ///   - target: the target address
    ///   - fee: the network fees
    ///   - completion: the handler for the results
    ///
    public func estimateLimitMaximum (target: Address,
                                      fee: NetworkFee,
                                      completion: @escaping Wallet.EstimateLimitHandler) {
        estimateLimit (asMaximum: true, target: target, fee: fee, attributes: nil, completion: completion)
    }

    ///
    /// Estimate the minimum amount that can be transfered from Wallet.  This value does not
    /// include the fee, however, a fee estimate has been performed.  Generally the minimum
    /// amount in zero; however, some currencies have minimum values, below which miners will
    /// reject.  In those casaes the minimum amount is above zero.
    ///
    /// In cases where `balance < amount + fee` then .insufficientFunds is returned.  The
    /// .insufficientFunds check respects the wallet from which fees are extracted.
    ///
    /// This is an synchronous function that returns immediately but will call `completion` once
    /// the maximum has been determined.
    ///
    /// The returned Amount is always in the wallet's currencyh.
    ///
    /// - Parameters:
    ///   - target: the target address
    ///   - fee: the network fees
    ///   - completion: the handler for the results
    ///
    public func estimateLimitMinimum (target: Address,
                                      fee: NetworkFee,
                                      completion: @escaping Wallet.EstimateLimitHandler) {
        estimateLimit (asMaximum: false, target: target, fee: fee, attributes: nil, completion: completion)
    }

    ///
    /// Internal function to handle limit estimation
    ///
    internal func estimateLimit (asMaximum: Bool,
                                 target: Address,
                                 fee: NetworkFee,
                                 attributes: Set<TransferAttribute>?,
                                 completion: @escaping Wallet.EstimateLimitHandler) {
        var needFeeEstimate: WKBoolean = WK_TRUE
        var isZeroIfInsuffientFunds: WKBoolean = WK_FALSE;

        // This `amount` is in the `unit` of `wallet`
        guard let amount = wkWalletManagerEstimateLimit (self.manager.core,
                                                             self.core,
                                                             (asMaximum ? WK_TRUE : WK_FALSE),
                                                             target.core,
                                                             fee.core,
                                                             &needFeeEstimate,
                                                             &isZeroIfInsuffientFunds)
            .map ({ Amount (core: $0, take: false)})
            else {
                // This is extraneous as `wkWalletEstimateLimit()` always returns an amount
                estimateLimitCompleteInQueue (completion,
                                              Result.failure (LimitEstimationError.insufficientFunds))
                return;
        }

        // If we don't need an estimate, then we invoke `completion` and skip out immediately.  But
        // include a check on a zero amount - which indicates insufficient funds.
        if WK_FALSE == needFeeEstimate {
            estimateLimitCompleteInQueue (completion,
                                          (WK_TRUE == isZeroIfInsuffientFunds && amount.isZero
                                            ? Result.failure (LimitEstimationError.insufficientFunds)
                                            : Result.success (amount)))
            return
        }

        // We need an estimate of the fees.

        // The currency for the fee
        let currencyForFee = fee.pricePerCostFactor.currency

        guard let walletForFee = self.manager.wallets
            .first (where: { $0.currency == currencyForFee })
            else {
                estimateLimitCompleteInQueue(completion, Result.failure (LimitEstimationError.serviceError))
                return

        }

        // Skip out immediately if we've no balance.
        if walletForFee.balance.isZero {
            estimateLimitCompleteInQueue (completion, Result.failure (Wallet.LimitEstimationError.insufficientFunds))
            return
        }

        //
        // If the `walletForFee` differs from `wallet` then we just need to estimate the fee
        // once.  Get the fee estimate and just ensure that walletForFee has sufficient balance
        // to pay the fee.
        //
        if self != walletForFee {
            // This `amount` will not unusually be zero.
            // TODO: Does ETH fee estimation work if the ERC20 amount is zero?
            self.estimateFee (target: target, amount: amount, fee: fee, attributes: attributes) {
                (res: Result<TransferFeeBasis, Wallet.FeeEstimationError>) in
                switch res {
                case .success (let feeBasis):
                    completion (walletForFee.balance >= feeBasis.fee
                        ? Result.success(amount)
                        : Result.failure(LimitEstimationError.insufficientFunds))

                case .failure (let error):
                    completion (Result.failure (LimitEstimationError.fromFeeEstimationError(error)))
                }
            }
            return
        }

        // The `fee` is in the same unit as the `wallet`

        //
        // If we are estimating the minimum, then get the fee and ensure that the wallet's
        // balance is enough to cover the (minimum) amount plus the fee
        //
        if !asMaximum {
            self.estimateFee (target: target, amount: amount, fee: fee, attributes: attributes) {
                (res: Result<TransferFeeBasis, Wallet.FeeEstimationError>) in
                switch res {
                case .success (let feeBasis):
                    guard let transactionAmount = amount + feeBasis.fee
                        else { preconditionFailure() }

                    completion (self.balance >= transactionAmount
                        ? Result.success (amount)
                        : Result.failure (LimitEstimationError.insufficientFunds))

                case .failure (let error):
                    completion (Result.failure (LimitEstimationError.fromFeeEstimationError(error)))
                }
            }
            return
        }

        // The base unit for `currency`
        let transferBaseUnit = self.manager.network.baseUnitFor(currency: self.currency)!

        // The minimum non-zero transfer amount
        let transferMin  = Amount.create (integer: 1, unit: transferBaseUnit)
        let transferZero = Amount.create (integer: 0, unit: transferBaseUnit)

        //
        // We are forced to deal with XTZ.  Not by our choosing.  The value returned by the above
        // `wkWalletManagerEstimateLimit()` is something well below `self.balance` for XTZ - becuase
        // we are desperate to get a non-error response from the XTZ node.  And, if we provide the
        // balance for the estimate, we get a `balance_too_low` error.  This then forces us into
        // a binary search until 'not balance_too_low' which for a range of {0, 1 xtz} is ~25
        // queries of Blockset and the XTZ Node.  Insane.  We will unfortunately sacrifice our
        // User's funds until XTZ matures.
        //
        if (.xtz == manager.network.type) {
            // Make a request with the lowest possible amount; hopefully we get a result.
            estimateFee (target: target,
                         amount: transferMin,
                         fee: fee,
                         attributes: attributes) { (res: Result<TransferFeeBasis, FeeEstimationError>) in
                switch res {
                case .success (let feeBasis):
                    let amountEstimated = (self.balance - feeBasis.fee) ?? transferZero
                    completion (Result.success (amountEstimated < amount
                                                        ? amountEstimated
                                                        : amount))

                case .failure (_):
                    //
                    // The request failed but we don't know why (limits in the current interface).
                    // Could be a network failure; could be something with the XTZ wallet; could
                    // be a protocol change for XTZ - no matter, we'll return the maximum amount
                    // as the balance.  If the user attempts to send the 'balance' it will fail
                    // as there won't be enough for the fee.
                    //
                    completion (Result.success(amount))
                }
            }

            return
        }

        // If the `walletForFee` and `wallet` are identical, then we need to iteratively estimate
        // the fee and adjust the amount until the fee stabilizes.
        var transferFee = transferZero

        // We'll limit the number of iterations
        let estimationCompleterRecurseLimit = 3
        var estimationCompleterRecurseCount = 0

        // This function will be recursively defined
        func estimationCompleter (res: Result<TransferFeeBasis, Wallet.FeeEstimationError>) {
            // Another estimation completed
            estimationCompleterRecurseCount += 1

            // Check the result
            switch res {
            case .success (let feeBasis):
                // The estimated transfer fee
                let newTransferFee = feeBasis.fee

                // The estimated transfer amount, updated with the transferFee
                guard let newTransferAmount = amount.sub (newTransferFee)
                    else { preconditionFailure() }

                // If the two transfer fees match, then we have converged
                if transferFee == newTransferFee {
                    guard let transactionAmount = newTransferAmount + newTransferFee
                        else { preconditionFailure() }
                    
                    completion (self.balance >= transactionAmount && newTransferAmount >= transferZero
                        ? Result.success (newTransferAmount)
                        : Result.failure (Wallet.LimitEstimationError.insufficientFunds))

                }

                else if estimationCompleterRecurseCount < estimationCompleterRecurseLimit {
                    // but is they haven't converged try again with the new amount
                    transferFee = newTransferFee
                    self.estimateFee (target: target, amount: newTransferAmount, fee: fee, attributes: attributes, completion: estimationCompleter)
                }

                else {
                    // We've tried too many times w/o convergence; abort
                    completion (Result.failure (Wallet.LimitEstimationError.serviceError))
                }

            case .failure (let error):
                completion (Result.failure (LimitEstimationError.fromFeeEstimationError(error)))
            }
        }

        estimateFee (target: target,
                     amount: (manager.network.type == .xtz ? transferMin : amount),
                     fee: fee,
                     attributes: attributes,
                     completion: estimationCompleter)
    }

    private func estimateLimitCompleteInQueue (_ completion: @escaping Wallet.EstimateLimitHandler,
                                               _ result: Result<Amount, Wallet.LimitEstimationError>) {
        system.queue.async {
            completion (result)
        }
    }

    public enum LimitEstimationError: Error {
        case serviceUnavailable
        case serviceError
        case insufficientFunds

        static func fromStatus (_ status: WKStatus) -> LimitEstimationError {
            switch status {
            case WK_ERROR_FAILED: return .serviceError
            default: return .serviceError // preconditionFailure ("Unknown FeeEstimateError")
            }
        }

        static func fromFeeEstimationError (_ error: FeeEstimationError) -> LimitEstimationError{
            switch error {
            case .ServiceUnavailable: return .serviceUnavailable
            case .ServiceError:       return .serviceError
            case .InsufficientFunds:  return .insufficientFunds
            }
        }
    }


    /// MARK: Estimate Fee

    /// A `Wallet.EstimateFeeHandler` is a function to handle the result of a Wallet.estimateFee.
    public typealias EstimateFeeHandler = (Result<TransferFeeBasis,FeeEstimationError>) -> Void

    ///
    /// Estimate the `TransferFeeBasis` for a transfer with `amount` from `wallet`.  The result
    /// will have margin applied if appropriate for the `target`.  Specifically, some targets for
    /// some blockchains involve 'Smart Contracts' which may have some uncertainty in their 'cost
    /// units'.  On a blockchain-specific basis a blockchain-specific amount of margin is added
    /// to the 'cost units' - thereby ensuring high reliabitly in transfer submission.
    ///
    /// - Parameters:
    ///   - target: the transfer's target address
    ///   - amount: the transfer amount MUST BE GREATER THAN 0
    ///   - fee: the network fee (aka priority)
    ///   - attributes: arbitrary, generally Network-specific, attributes
    ///   - completion: handler function
    ///
    public func estimateFee (target: Address,
                             amount: Amount,
                             fee: NetworkFee,
                             attributes: Set<TransferAttribute>?,
                             completion: @escaping Wallet.EstimateFeeHandler) {
        if nil != attributes && nil != self.validateTransferAttributes(attributes!) {
            assertionFailure()
        }

        let coreAttributesCount = attributes?.count ?? 0
        var coreAttributes: [WKTransferAttribute?] = attributes?.map { $0.core } ?? []

        let amountHackedIfXTZ = hackTheAmountIfTezos(amount: amount)

        // 'Redirect' up to the 'manager'
        wkWalletManagerEstimateFeeBasis (self.manager.core,
                                             self.core,
                                             callbackCoordinator.addWalletFeeEstimateHandler(completion),
                                             target.core,
                                             amountHackedIfXTZ.core,
                                             fee.core,
                                             coreAttributesCount,
                                             &coreAttributes)
    }

    internal func estimateFee (sweeper: WalletSweeper,
                               fee: NetworkFee,
                               completion: @escaping EstimateFeeHandler) {
        wkWalletManagerEstimateFeeBasisForWalletSweep (sweeper.core,
                                                           self.manager.core,
                                                           self.core,
                                                           callbackCoordinator.addWalletFeeEstimateHandler(completion),
                                                           fee.core)
    }
    
    internal func estimateFee (request: PaymentProtocolRequest,
                               fee: NetworkFee,
                               completion: @escaping EstimateFeeHandler) {
        wkWalletManagerEstimateFeeBasisForPaymentProtocolRequest (self.manager.core,
                                                                      self.core,
                                                                      callbackCoordinator.addWalletFeeEstimateHandler(completion),
                                                                      request.core,
                                                                      fee.core)
    }

    public enum FeeEstimationError: Error {
        case ServiceUnavailable
        case ServiceError
        case InsufficientFunds

        static func fromStatus (_ status: WKStatus) -> FeeEstimationError {
            switch status {
            case WK_ERROR_FAILED: return .ServiceError
            default: return .ServiceError // preconditionFailure ("Unknown FeeEstimateError")
            }
        }
    }
    
    internal func defaultFeeBasis () -> TransferFeeBasis? {
        return wkWalletGetDefaultFeeBasis (core)
            .map { TransferFeeBasis (core: $0, take: false) }
    }
    
    ///
    /// Create a wallet
    ///
    /// - Parameters:
    ///   - core: the WKWallet basis
    ///   - listener: an optional listener
    ///   - manager: the manager
    ///   - take: a boolean to indicate if `core` needs to be taken (for reference counting)
    ///
    internal init (core: WKWallet,
                   manager: WalletManager,
                   callbackCoordinator: SystemCallbackCoordinator,
                   take: Bool) {
        self.core = take ? wkWalletTake (core) : core
        self.manager = manager
        self.callbackCoordinator = callbackCoordinator
        self.unit = Unit (core: wkWalletGetUnit(core), take: false)
        self.unitForFee = Unit (core: wkWalletGetUnitForFee(core), take: false)
    }

    deinit {
        wkWalletGive (core)
    }

    // Equatable
    public static func == (lhs: Wallet, rhs: Wallet) -> Bool {
        return lhs === rhs || lhs.core == rhs.core
    }
}

extension Wallet {
    // Default implementation, using `transferFactory`
    //    public func createTransfer (listener: TransferListener,
    //                                target: Address,
    //                                amount: Amount,
    //                                feeBasis: TransferFeeBasis) -> Transfer? {
    //        return transferFactory.createTransfer (listener: listener,
    //                                               wallet: self,
    //                                               target: target,
    //                                               amount: amount,
    //                                               feeBasis: feeBasis)
    //    }

    ///
    /// Create a transfer for wallet using the `defaultFeeBasis`.  Invokes the wallet's
    /// `transferFactory` to create a transfer.  Generates events: TransferEvent.created and
    /// WalletEvent.transferAdded(transfer).
    ///
    /// - Parameters:
    ///   - source: The source spends 'amount + fee'
    ///   - target: The target receives 'amount'
    ///   - amount: The amouunt
    ///
    /// - Returns: A new transfer
    ///
//    public func createTransfer (target: Address,
//                                amount: Amount) -> Transfer? {
//        return createTransfer (target: target,
//                               amount: amount,
//                               feeBasis: defaultFeeBasis)
//    }

}

///
/// The Wallet state
///
/// - created: The wallet was created (and remains in existence).
/// - deleted: The wallet was deleted.
///
public enum WalletState: Equatable {
    case created
    case deleted

    internal init (core: WKWalletState) {
        switch core {
        case WK_WALLET_STATE_CREATED: self = .created
        case WK_WALLET_STATE_DELETED: self = .deleted
        default: self = .created; preconditionFailure()
        }
    }
}

///
/// A WalletEvent represents a asynchronous announcment of a wallet's state change.
///
public enum WalletEvent {
    case created
    case changed (oldState: WalletState, newState: WalletState)
    case deleted

    case transferAdded     (transfer: Transfer)
    case transferChanged   (transfer: Transfer)
    case transferSubmitted (transfer: Transfer, success: Bool)
    case transferDeleted   (transfer: Transfer)

    case balanceUpdated    (amount: Amount)
    case feeBasisUpdated   (feeBasis: TransferFeeBasis)
    case feeBasisEstimated (feeBasis: TransferFeeBasis)

    init? (wallet: Wallet, core: WKWalletEvent) {
        switch wkWalletEventGetType(core) {
        case WK_WALLET_EVENT_CREATED:
            self = .created
            
        case WK_WALLET_EVENT_CHANGED:
            var oldState: WKWalletState!
            var newState: WKWalletState!

            wkWalletEventExtractState(core, &oldState, &newState)
            self = .changed (oldState: WalletState (core: oldState),
                             newState: WalletState (core: newState))
            
        case WK_WALLET_EVENT_DELETED:
            self = .deleted
            
        case WK_WALLET_EVENT_TRANSFER_ADDED:
            var transfer: WKTransfer!

            wkWalletEventExtractTransfer (core, &transfer);
            self = .transferAdded (transfer: Transfer (core: transfer,
                                                       wallet: wallet,
                                                       take: false))
            
        case WK_WALLET_EVENT_TRANSFER_CHANGED:
            var transfer: WKTransfer!

            wkWalletEventExtractTransfer (core, &transfer);
            self = .transferChanged (transfer: Transfer (core: transfer,
                                                         wallet: wallet,
                                                         take: false))
            
        case WK_WALLET_EVENT_TRANSFER_SUBMITTED:
            var transfer: WKTransfer!
            wkWalletEventExtractTransferSubmit (core, &transfer);

            self = .transferSubmitted (transfer: Transfer (core: transfer,
                                                           wallet: wallet,
                                                           take: false),
                                       success: true);
            
        case WK_WALLET_EVENT_TRANSFER_DELETED:
            var transfer: WKTransfer!

            wkWalletEventExtractTransfer (core, &transfer);
            self = .transferDeleted (transfer: Transfer (core: transfer,
                                                         wallet: wallet,
                                                         take: false))
            
        case WK_WALLET_EVENT_BALANCE_UPDATED:
            var balance: WKAmount!

            wkWalletEventExtractBalanceUpdate (core, &balance);
            self = .balanceUpdated (amount: Amount (core: balance, take: false))
            
        case WK_WALLET_EVENT_FEE_BASIS_UPDATED:
            var feeBasis: WKFeeBasis!

            wkWalletEventExtractFeeBasisUpdate (core, &feeBasis);

            guard nil != feeBasis else { return nil }
            self = .feeBasisUpdated (feeBasis: TransferFeeBasis (core: feeBasis, take: false))

        case WK_WALLET_EVENT_FEE_BASIS_ESTIMATED:
            var feeBasis: WKFeeBasis!

            wkWalletEventExtractFeeBasisEstimate (core, nil, nil, &feeBasis);
            
            guard nil != feeBasis else { return nil }
            self = .feeBasisEstimated (feeBasis: TransferFeeBasis (core: feeBasis, take: false))
            
        default:
            preconditionFailure()
        }
    }
}

extension WalletEvent: CustomStringConvertible {
    public var description: String {
        switch self {
        case .created:           return "Created"
        case .changed:           return "StateChanged"
        case .deleted:           return "Deleted"
        case .transferAdded:     return "TransferAdded"
        case .transferChanged:   return "TransferChanged"
        case .transferDeleted:   return "TransferDeleted"
        case .transferSubmitted: return "TransferSubmitted"
        case .balanceUpdated:    return "BalanceUpdated"
        case .feeBasisUpdated:   return "FeeBasisUpdated"
        case .feeBasisEstimated: return "FeeBasisEstimated"
        }
    }
}

///
/// Listener for WalletEvent
///
public protocol WalletListener: AnyObject {
    ///
    /// Handle a WalletEvent
    ///
    /// - Parameters:
    ///   - system: the system
    ///   - manager: the manager
    ///   - wallet: the wallet
    ///   - event: the wallet event.
    ///
    func handleWalletEvent (system: System,
                            manager: WalletManager,
                            wallet: Wallet,
                            event: WalletEvent)
}
/// A Functional Interface for a Handler
public typealias WalletEventHandler = (System, WalletManager, Wallet, WalletEvent) -> Void


///
/// A WalletFactory is a customization point for Wallet creation.
/// TODO: ?? AND HOW DOES THIS FIT WITH CoreWallet w/ REQUIRED INTERFACE TO Core ??
///
public protocol WalletFactory {
    ///
    /// Create a Wallet managed by `manager` and holding `currency`.  The wallet is initialized
    /// with no balance, no transfers and some default feeBasis (appropriate for the `currency`).
    /// Generates events: WalletEvent.created (and maybe others).
    ///
    /// - Parameters:
    ///   - manager: the Wallet's manager
    ///   - currency: The currency held
    ///
    /// - Returns: A new wallet
    ///
    //    func createWallet (manager: WalletManager,
    //                       currency: Currency) -> Wallet
}

