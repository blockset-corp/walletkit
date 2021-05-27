//
//  BRCryptoAccount.swift
//  WalletKit
//
//  Created by Ed Gamble on 3/27/19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//
import Foundation // Data
import WalletKitCore

///
///
///
public final class Account {
    let core: BRCryptoAccount

    // A 'globally unique' ID String for account.  For BlockchainDB this will be the 'walletId'
    public var uids: String {
        return asUTF8String (cryptoAccountGetUids (core))
    }

    public var timestamp: Date {
        return Date.init(timeIntervalSince1970: TimeInterval (cryptoAccountGetTimestamp (core)))
    }

    ///
    /// Serialize an account.  The serialization is *always* in the current, default format
    ///
    public var serialize: Data {
        var bytesCount: Int = 0
        let bytes = cryptoAccountSerialize (core, &bytesCount)
        defer { cryptoMemoryFree(bytes) }
        return Data (bytes: bytes!, count: bytesCount)
    }

    ///
    /// Validate that `serialization` is for this account.
    ///
    /// - Parameter serialization: the serialization data
    ///
    public func validate (serialization: Data) -> Bool {
        var bytes = [UInt8](serialization)
        return CRYPTO_TRUE == cryptoAccountValidateSerialization (core, &bytes, bytes.count)
    }

    internal init (core: BRCryptoAccount, take: Bool) {
        self.core = take ? cryptoAccountTake(core) : core
    }

    internal var fileSystemIdentifier: String {
        return asUTF8String (cryptoAccountGetFileSystemIdentifier(core), true);
    }

    deinit {
        cryptoAccountGive (core)
    }

    ///
    /// Recover an account from a BIP-39 'paper key'
    ///
    /// - Parameter paperKey: the 12 word paper key
    /// - Parameter timestamp:
    ///
    /// - Returns: the paperKey's corresponding account, or NIL if the paperKey is invalid.
    ///
    public static func createFrom (phrase: String, timestamp: Date, uids: String) -> Account? {
        let timestampAsInt = UInt64 (timestamp.timeIntervalSince1970)
        return cryptoAccountCreate (phrase, timestampAsInt, uids)
            .map { Account (core: $0, take: false) }
    }

    ///
    /// Create an account based on an account serialization
    ///
    /// - Parameter serialization: The result of a prior call to account.serialize.
    ///
    /// - Returns: The serialization's corresponding account or NIL if the serialization is invalid.
    ///    If the serialization is invalid then the account *must be recreated* from the `phrase`
    ///    (aka 'Paper Key').  A serialization will be invald when the serialization format changes
    ///    which will *always occur* when a new blockchain is added.  For example, when XRP is added
    ///    the XRP public key must be serialized; the old serialization w/o the XRP public key will
    ///    be invalid and the `phrase` is *required* in order to produce the XRP public key.
    ///
    public static func createFrom (serialization: Data, uids: String) -> Account? {
        var bytes = [UInt8](serialization)
        return cryptoAccountCreateFromSerialization (&bytes, bytes.count, uids)
            .map { Account (core: $0, take: false) }
    }

    ///
    /// Generate a BIP-39 'paper Key'.  Use Account.createFrom(paperKey:) to get the account.  The
    /// wordList is the locale-specifc BIP-39-defined array of BIP39_WORDLIST_COUNT words.  This
    /// function has a precondition on the size of the wordList.
    ///
    /// - Parameter words: A local-specific BIP-39-defined array of BIP39_WORDLIST_COUNT words.
    ///
    /// - Returns: A 12 word 'paper key'
    ///
    public static func generatePhrase (words: [String]) -> (String,Date)? {
        precondition (CRYPTO_TRUE == cryptoAccountValidateWordsList (words.count))

        var words = words.map { UnsafePointer<Int8> (strdup($0)) }
        defer { words.forEach { cryptoMemoryFree (UnsafeMutablePointer (mutating: $0)) } }

        return (asUTF8String (cryptoAccountGeneratePaperKey (&words)), Date())
    }

    ///
    /// Validate a phrase as a BIP-39 'paper key'; returns true if validated, false otherwise
    ///
    /// - Parameters:
    ///   - phrase: the candidate paper key
    ///   - words: A locale-specific BIP-39-defined array of BIP39_WORDLIST_COUNT words.
    ///
    /// - Returns: true is a valid paper key; false otherwise
    ///
    public static func validatePhrase (_ phrase: String, words: [String]) -> Bool {
        precondition (CRYPTO_TRUE == cryptoAccountValidateWordsList (words.count))

        var words = words.map { UnsafePointer<Int8> (strdup($0)) }
        defer { words.forEach { cryptoMemoryFree (UnsafeMutablePointer (mutating: $0)) } }

        return CRYPTO_TRUE == cryptoAccountValidatePaperKey (phrase, &words)
    }

    ///
    /// Check if `account` is initialized for `network`.  Some networks require that accounts
    /// be initialized before they can be used; Hedera is one such network.
    ///
    /// - Parameters:
    ///   - network: the network
    ///
    /// - Returns: `true` if initialized; `false` otherwise
    ///
    internal func isInitialized (onNetwork network: Network) -> Bool {
        return CRYPTO_TRUE == cryptoNetworkIsAccountInitialized(network.core, core)
    }

    ///
    /// Initialize `account` on `network` using `data`.  The provided data is network specific and
    /// thus an opaque sequence of bytes.
    ///
    /// - Parameters:
    ///   - network: the network
    ///   - data: the data
    ///
    /// - Returns: The account serialization or `nil` if the account was already initialized.  This
    ///            serialization must be saved otherwise the initialization will be lost upon the
    ///            next System start.
    ///
    internal func initialize (onNetwork network: Network, using data: Data) -> Data? {
        guard !isInitialized (onNetwork: network)
            else { return nil }

        return data.withUnsafeBytes { (dataBytes: UnsafeRawBufferPointer) -> Data in
            let dataAddr  = dataBytes.baseAddress?.assumingMemoryBound(to: UInt8.self)
            let dataCount = dataBytes.count

            cryptoNetworkInitializeAccount (network.core, core, dataAddr, dataCount);

            return serialize
        }
    }

    ///
    /// Get the data needed to initialize `account` on `network`.  This data is network specfic and
    /// thus an opaqe sequence of bytes.  The bytes are provided to some 'initialization provider'
    /// in a network specific manner; the provider's result is passed back using the
    /// `accountInitialize` function.
    ///
    /// - Parameters:
    ///   - network: the network
    ///
    /// - Returns: Opaque data to be provided to the 'initialization provider'
    ///
    internal func getInitializationdData (onNetwork network: Network) -> Data? {
        var bytesCount: BRCryptoCount = 0
        return cryptoNetworkGetAccountInitializationData (network.core, core, &bytesCount)
            .map {
                let bytes = $0
                defer { cryptoMemoryFree (bytes) }
                return Data (bytes: bytes, count: bytesCount)
        }
    }
}
