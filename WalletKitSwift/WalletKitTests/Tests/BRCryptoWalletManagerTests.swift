//
//  BRCryptoWalletManagerTests.swift
//  WalletKitTests
//
//  Created by Ed Gamble on 1/11/19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

import XCTest
@testable import WalletKit

class BRCryptoWalletManagerTests: BRCryptoSystemBaseTests {
    
    override func setUp() {
        super.setUp()
    }

    override func tearDown() {
    }

    func testWalletManagerMode () {
        let modes = [WalletManagerMode.api_only,
                     WalletManagerMode.api_with_p2p_submit,
                     WalletManagerMode.p2p_with_api_sync,
                     WalletManagerMode.p2p_only]

        for mode in modes {
            XCTAssertEqual (mode, WalletManagerMode (serialization: mode.serialization))
            XCTAssertEqual (mode, WalletManagerMode (core: mode.core))
        }

        for syncModeInteger in 0..<4 {
            XCTAssertNil (WalletManagerMode (serialization: UInt8 (syncModeInteger)))
        }
    }

    func runWalletManagerBTCTest (networkType: NetworkType, currencyCode: String) {
        let walletManagerDisconnectExpectation = XCTestExpectation (description: "Wallet Manager Disconnect")
        listener.managerHandlers += [
            { (system: System, manager:WalletManager, event: WalletManagerEvent) in
                if case let .changed(_, newState) = event, case .disconnected = newState {
                    walletManagerDisconnectExpectation.fulfill()
                }
            }]

        let network: Network! = system.networks.first {
            networkType == $0.type
                && currencyCode == $0.currency.code
                && isMainnet == $0.onMainnet
        }
        XCTAssertNotNil (network)

        let manager: WalletManager! = system.managers.first { $0.network == network }
        let wallet = manager.primaryWallet

        XCTAssertNotNil (manager)
        XCTAssertTrue  (system  === manager.system)
        XCTAssertEqual (network, manager.network)

        XCTAssertEqual (WalletManagerState.created, manager.state)
        XCTAssertTrue  (manager.height > 0)
        XCTAssertEqual (manager.primaryWallet.manager, manager)
        XCTAssertEqual (1, manager.wallets.count)
        XCTAssertTrue  (manager.wallets.contains(manager.primaryWallet))
        XCTAssertTrue  (network.fees.contains(manager.defaultNetworkFee))

        XCTAssertTrue  (network.supportedModes.contains(manager.mode))
        XCTAssertEqual (network.defaultAddressScheme, manager.addressScheme)

        if let otherAddressScheme = network.supportedAddressSchemes.first (where: { $0 != manager.addressScheme }) {
            manager.addressScheme = otherAddressScheme
            XCTAssertEqual (otherAddressScheme, manager.addressScheme)
        }
        manager.addressScheme = network.defaultAddressScheme
        XCTAssertEqual (network.defaultAddressScheme, manager.addressScheme)

        XCTAssertNotNil (manager.baseUnit)
        XCTAssertNotNil (manager.defaultUnit)
        XCTAssertFalse (manager.isActive)
        XCTAssertEqual (manager, manager)
        
        XCTAssertEqual(currencyCode, manager.description)

        XCTAssertFalse (system.wallets.isEmpty)

        // Events

        XCTAssertTrue (listener.checkSystemEventsCommonlyWith (network: network,
                                                               manager: manager))

        XCTAssertTrue (listener.checkManagerEvents(
            [WalletManagerEvent.created,
             WalletManagerEvent.walletAdded(wallet: wallet)],
            strict: true))

        XCTAssertTrue (listener.checkWalletEvents(
            [WalletEvent.created],
            strict: true))

        XCTAssertTrue (listener.checkTransferEvents(
            [],
            strict: true))

        // Connect
        listener.transferCount = (.bsv == networkType ? 2 : 5)
        manager.connect()
        wait (for: [self.listener.transferExpectation], timeout: 15)

        manager.disconnect()
        wait (for: [walletManagerDisconnectExpectation], timeout: 5)

        XCTAssertTrue (listener.checkManagerEventsCommonlyWith (mode: manager.mode,
                                                               wallet: wallet))
    }

    func testWalletManagerBTC() {
        isMainnet = false
        prepareAccount()

        currencyCodesToMode = ["btc":WalletManagerMode.api_only]
        prepareSystem()

        runWalletManagerBTCTest(networkType: .btc, currencyCode: "btc")
    }

    func testWalletManagerBCH() {
        isMainnet = false
        prepareAccount()

        currencyCodesToMode = ["bch":WalletManagerMode.api_only]
        prepareSystem()

        runWalletManagerBTCTest(networkType: .bch, currencyCode: "bch")
    }

    func testWalletManagerBSV() {
        isMainnet = true
        prepareAccount (identifier: "loan(C)")

        currencyCodesToMode = ["bsv":WalletManagerMode.api_only]
        prepareSystem()

        runWalletManagerBTCTest(networkType: .bsv, currencyCode: "bsv")
    }

    func testWalletManagerETH () {
        isMainnet = true
        prepareAccount (identifier: "loan(C)")

        registerCurrencyCodes = ["brd"]
        currencyCodesToMode = ["eth":WalletManagerMode.api_only]

        let listener = CryptoTestSystemListener (networkCurrencyCodesToMode: currencyCodesToMode,
                                                 registerCurrencyCodes: registerCurrencyCodes,
                                                 isMainnet: isMainnet)

        // Listen for a non-primary wallet - specifically the BRD wallet
        var walletBRD: Wallet! = nil
        let walletBRDExpectation = XCTestExpectation (description: "BRD Wallet")
        listener.managerHandlers += [
            { (system: System, manager:WalletManager, event: WalletManagerEvent) in
                if case let .walletAdded(wallet) = event, "brd" == wallet.name {
                    walletBRD = wallet
                    walletBRDExpectation.fulfill()
                }
            }]

        let walletManagerDisconnectExpectation = XCTestExpectation (description: "Wallet Manager Disconnect")
        listener.managerHandlers += [
            { (system: System, manager:WalletManager, event: WalletManagerEvent) in
                if case let .changed(_, newState) = event, case .disconnected = newState {
                    walletManagerDisconnectExpectation.fulfill()
                }
            }]

        prepareSystem (listener: listener)

        let network: Network! = system.networks.first { "eth" == $0.currency.code && isMainnet == $0.onMainnet }
        XCTAssertNotNil (network)

        let manager: WalletManager! = system.managers.first { $0.network == network }
        XCTAssertNotNil (manager)

        let walletETH = manager.primaryWallet
        wait (for: [walletBRDExpectation], timeout: 30)

        // Events

        XCTAssertTrue (listener.checkSystemEventsCommonlyWith (network: network,
                                                               manager: manager))

        XCTAssertTrue (listener.checkManagerEvents(
            [EventMatcher (event: WalletManagerEvent.created),
             EventMatcher (event: WalletManagerEvent.walletAdded(wallet: walletETH), strict: true, scan: false),
             EventMatcher (event: WalletManagerEvent.walletAdded(wallet: walletBRD), strict: true, scan: true)
            ]))

        XCTAssertTrue (listener.checkWalletEvents(
            [WalletEvent.created,
             WalletEvent.created],
            strict: true))

        XCTAssertTrue (listener.checkTransferEvents(
            [],
            strict: true))

        // Connect
        listener.transferCount = 2
        manager.connect()
        wait (for: [self.listener.transferExpectation], timeout: 60)

        sleep (30) // allow some 'ongoing' syncs to occur; don't want to see events for these.
        manager.disconnect()
        wait (for: [walletManagerDisconnectExpectation], timeout: 5)

        // TODO: We have an 'extra' syncStarted in here; the disconnect reason is 'unknown'?
        XCTAssertTrue (listener.checkManagerEvents (
            [EventMatcher (event: WalletManagerEvent.created),
             EventMatcher (event: WalletManagerEvent.walletAdded(wallet: walletETH)),
             EventMatcher (event: WalletManagerEvent.walletAdded(wallet: walletBRD), strict: true, scan: true),
             EventMatcher (event: WalletManagerEvent.changed(oldState: WalletManagerState.created,   newState: WalletManagerState.connected),
                           strict: true, scan: true),
             // wallet changed?
             EventMatcher (event: WalletManagerEvent.syncStarted, strict: true, scan: true),
             EventMatcher (event: WalletManagerEvent.changed(oldState: WalletManagerState.connected, newState: WalletManagerState.syncing)),
             // We might not see `syncProgress`
             // EventMatcher (event: WalletManagerEvent.syncProgress(timestamp: nil, percentComplete: 0), strict: false),

             EventMatcher (event: WalletManagerEvent.syncEnded(reason: WalletManagerSyncStoppedReason.complete), strict: false, scan: true),
             EventMatcher (event: WalletManagerEvent.changed(oldState: WalletManagerState.syncing, newState: WalletManagerState.connected)),

             // Can have another sync started here... so scan
             EventMatcher (event: WalletManagerEvent.changed(oldState: WalletManagerState.connected,
                                                             newState: WalletManagerState.disconnected (reason: WalletManagerDisconnectReason.unknown)),
                           strict: true, scan: true),
            ]))
    }

    func testWalletManagerXRP() {
        isMainnet = true
        prepareAccount (identifier: "loan(C)")

        currencyCodesToMode = ["xrp":WalletManagerMode.api_only]
        prepareSystem()

        let walletManagerDisconnectExpectation = XCTestExpectation (description: "Wallet Manager Disconnect")
        let walletManagerConnectExpectation    = XCTestExpectation (description: "Wallet Manager Connect")
        let walletManagerSyncDoneExpectation   = XCTestExpectation (description: "Wallet Manager Sync Done")
        listener.managerHandlers += [
            { (system: System, manager:WalletManager, event: WalletManagerEvent) in
                if case let .changed(_, newState) = event, case .disconnected = newState {
                    walletManagerDisconnectExpectation.fulfill()
                }
                else if case let .changed(oldState, newState) = event, case .syncing = oldState, case .connected = newState {
                    walletManagerSyncDoneExpectation.fulfill()
                }
                else if case let .changed(_, newState) = event, case .connected = newState {
                    walletManagerConnectExpectation.fulfill()
                }
            }]

        let network: Network! = system.networks.first { "xrp" == $0.currency.code && isMainnet == $0.onMainnet }
        XCTAssertNotNil (network)

        let manager: WalletManager! = system.managers.first { $0.network == network }
        let wallet = manager.primaryWallet

        XCTAssertNotNil (manager)
        XCTAssertTrue  (system  === manager.system)
        XCTAssertEqual (network, manager.network)

        XCTAssertEqual (WalletManagerState.created, manager.state)
        XCTAssertTrue  (manager.height > 0)
        XCTAssertEqual (manager.primaryWallet.manager, manager)
        XCTAssertEqual (1, manager.wallets.count)
        XCTAssertTrue  (manager.wallets.contains(manager.primaryWallet))
        XCTAssertTrue  (network.fees.contains(manager.defaultNetworkFee))

        XCTAssertTrue  (network.supportedModes.contains(manager.mode))
        XCTAssertEqual (network.defaultAddressScheme, manager.addressScheme)

        XCTAssertNotNil (manager.baseUnit)
        XCTAssertNotNil (manager.defaultUnit)
        XCTAssertFalse (manager.isActive)
        XCTAssertEqual (manager, manager)

        XCTAssertEqual("xrp", manager.description)

        XCTAssertFalse (system.wallets.isEmpty)

        // Events

        XCTAssertTrue (listener.checkSystemEventsCommonlyWith (network: network,
                                                               manager: manager))

        XCTAssertTrue (listener.checkManagerEvents(
            [WalletManagerEvent.created,
             WalletManagerEvent.walletAdded(wallet: wallet)],
            strict: true))

        XCTAssertTrue (listener.checkWalletEvents(
            [WalletEvent.created],
            strict: true))

        XCTAssertTrue (listener.checkTransferEvents(
            [],
            strict: true))

        // Connect
        manager.connect()
        wait (for: [walletManagerConnectExpectation], timeout: 15)

        // Wait for sync complete...
        wait (for: [walletManagerSyncDoneExpectation], timeout: 30)

        manager.disconnect()
        wait (for: [walletManagerDisconnectExpectation], timeout: 15)

        XCTAssertTrue (listener.checkManagerEventsCommonlyWith (mode: manager.mode,
                                                               wallet: wallet))
    }

    static var allTests = [
        ("testWalletManagerMode",       testWalletManagerMode),
        ("testWalletManagerBTC",        testWalletManagerBTC),
        ("testWalletManagerBCH",        testWalletManagerBCH),
        ("testWalletManagerBSV",        testWalletManagerBSV),
        ("testWalletManagerETH",        testWalletManagerETH),
    ]
}
