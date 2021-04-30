//
//  WKCurrency.swift
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
/// A currency is a medium for exchange.
///
public final class Currency: Hashable {

    internal let core: WKCurrency

    /// A 'Unique Identifier
    public var uids: String {
        return asUTF8String (wkCurrencyGetUids (core))
    }

    /// The code; e.g. BTC
    public var code: String {
        return asUTF8String (wkCurrencyGetCode (core))
    }

    /// The name; e.g. Bitcoin
    public var name: String {
        return asUTF8String (wkCurrencyGetName (core))
    }

    /// The type:
    public var type: String {
        return asUTF8String (wkCurrencyGetType (core))
    }

    /// The issuer, if present.  This is generally an ERC20 address.
    public var issuer: String? {
        return wkCurrencyGetIssuer (core).map { asUTF8String($0) }
    }

    internal init (core: WKCurrency, take: Bool) {
        self.core = take ? wkCurrencyTake (core) : core
    }

    internal convenience init (core: WKCurrency) {
        self.init (core: core, take: true)
    }

    internal convenience init (uids: String,
                               name: String,
                               code: String,
                               type: String,
                               issuer: String?) {
        self.init (core: wkCurrencyCreate(uids, name, code, type, issuer), take: false)
    }

    deinit {
        wkCurrencyGive (core)
    }

    public static func == (lhs: Currency, rhs: Currency) -> Bool {
        return lhs === rhs || WK_TRUE == wkCurrencyIsIdentical (lhs.core, rhs.core)
    }

    public func hash (into hasher: inout Hasher) {
        hasher.combine (uids)
    }
}
