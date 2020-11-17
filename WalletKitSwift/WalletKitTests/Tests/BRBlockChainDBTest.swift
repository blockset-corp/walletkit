//
//  BRBlockChainDBTest.swift
//  WalletKitTests
//
//  Created by Ed Gamble on 4/15/19.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

import XCTest
@testable import WalletKit

class BRBlockChainDBTest: XCTestCase {
    var client: SystemClient! = nil
    var expectation: XCTestExpectation!

    override func setUp() {
        guard let testConfiguration = TestConfiguration.loadFrom (bundle: Bundle.module)
        else { preconditionFailure("No TestConfiguration") }

        client = BlocksetSystemClient.createForTest(blocksetAccess: testConfiguration.blocksetAccess)
    }

    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }

    func testBlockchains() {
        expectation = XCTestExpectation (description: "blockchain")

        let blockchainId = "bitcoin-testnet"
        client.getBlockchain (blockchainId: blockchainId) { (res: Result<SystemClient.Blockchain, SystemClientError>) in
            guard case let .success (blockchain) = res
                else { XCTAssert(false); return }

            XCTAssertEqual (blockchainId, blockchain.id)
            XCTAssertEqual (6, blockchain.confirmationsUntilFinal)

            self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)


        expectation = XCTestExpectation (description: "blockchains")

        client.getBlockchains (mainnet: false) { (res: Result<[SystemClient.Blockchain], SystemClientError>) in
            guard case let .success (blockchains) = res
                else { XCTAssert(false); return }

            XCTAssertFalse (blockchains.isEmpty)

            self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)
    }

    func testCurrencies() {
        expectation = XCTestExpectation (description: "currency")

        let currencyId = "bitcoin-testnet:__native__"
        client.getCurrency (currencyId: currencyId) { (res: Result<SystemClient.Currency, SystemClientError>) in
            guard case let .success (currency) = res
                else { XCTAssert(false); return }

            XCTAssertEqual (currencyId, currency.id)
            self.expectation.fulfill()
        }
        wait (for: [expectation], timeout: 60)

        expectation = XCTestExpectation (description: "currencies")

        client.getCurrencies (mainnet: true) { (res: Result<[SystemClient.Currency], SystemClientError>) in
            guard case let .success (currencies) = res
                else { XCTAssert(false); return }

            XCTAssertFalse (currencies.isEmpty)

            self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)
    }

    func testTransfers() {
        expectation = XCTestExpectation (description: "transfer")

        let transferId = "bitcoin-testnet:ea4ed7efc701b5fdcfc38d4901f75f90c1a5de3e13fa38590289b2244f8887cb:0"
        client.getTransfer (transferId: transferId) { (res: Result<SystemClient.Transfer, SystemClientError>) in
            guard case let .success (transfer) = res
                else { XCTAssert(false); return }
            
            XCTAssertEqual (transferId, transfer.id)
            self.expectation.fulfill()
        }
        wait (for: [expectation], timeout: 60)

        //
        //
        //
        let blockchainId = "bitcoin-testnet"

        #if false
        expectation = XCTestExpectation (description: "transfers")

        client.getTransfers (blockchainId: blockchainId,
                             addresses: [],
                             begBlockNumber: 0,
                             endBlockNumber: 1,
                             maxPageSize: nil) {
                                (res: Result<[SystemClient.Transfer], SystemClientError>) in
                                guard case let .success (transfers) = res
                                    else { XCTAssert(false); return }
                                
                                XCTAssertTrue (transfers.isEmpty)
                                self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)
        #endif

        ///
        ///
        ///
        expectation = XCTestExpectation (description: "transfers /w addresses nonsense")

        client.getTransfers (blockchainId: blockchainId,
                             addresses: ["abc", "def"],
                             begBlockNumber: 0,
                             endBlockNumber: 1500000,
                             maxPageSize: nil) {
                                (res: Result<[SystemClient.Transfer], SystemClientError>) in
                                guard case let .success (transfers) = res
                                    else { XCTAssert(false); return }
                                
                                XCTAssertTrue (transfers.isEmpty)
                                self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)

        ///
        ///
        ///
        expectation = XCTestExpectation (description: "transfers /w addresses")

        client.getTransfers (blockchainId: blockchainId,
                             addresses: ["2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT"],
                             begBlockNumber: 1446080,
                             endBlockNumber: 1446090,
                             maxPageSize: nil) {
                                (res: Result<[SystemClient.Transfer], SystemClientError>) in
                                guard case let .success (transfers) = res
                                    else { XCTAssert(false); return }
                                
                                XCTAssertEqual (2, transfers.count)
                                self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)

        ///
        ///
        ///
        expectation = XCTestExpectation (description: "transfers w/ addresses w/ one-per-page")

        client.getTransfers (blockchainId: blockchainId,
                             addresses: ["2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT"],
                             begBlockNumber: 1446080,
                             endBlockNumber: 1446090,
                             maxPageSize: 1) {
                                (res: Result<[SystemClient.Transfer], SystemClientError>) in
                                guard case let .success (transfers) = res
                                    else { XCTAssert(false); return }
                                
                                XCTAssertEqual (2, transfers.count)
                                self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)

        ///
        /// Avoid this - `addresses` must not be empty
        ///
        #if false
        expectation = XCTestExpectation (description: "transfers w/ [0,11000) w/ no address")

        client.getTransfers (blockchainId: blockchainId,
                             addresses: [],
                             begBlockNumber: 1446080,
                             endBlockNumber: 1446090,
                             maxPageSize: nil) {
                                (res: Result<[SystemClient.Transfer], SystemClientError>) in
                                // A 'status' 400
                                guard case let .success(transfers) = res
                                    else { XCTAssert(false); return }
                                
                                XCTAssert(transfers.isEmpty)
                                self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)
        #endif
    }

    func testTransactions () {
        expectation = XCTestExpectation (description: "transactions")

        let transactionId = "bitcoin-testnet:d9bdd96426747b769aab74e33e109e73f793a1e309c00bed7732824a2ac85438"
        client.getTransaction (transactionId: transactionId,
                               includeRaw: true,
                               includeProof: false) {
                                (res: Result<SystemClient.Transaction, SystemClientError>) in
                                guard case let .success (transaction) = res
                                    else { XCTAssert(false); return }

                                XCTAssertEqual (transactionId, transaction.id)
                                self.expectation.fulfill()
        }
        wait (for: [expectation], timeout: 60)

        ///
        ///
        ///
        let blockchainId = "bitcoin-testnet"

        #if false
        expectation = XCTestExpectation (description: "transactions")

        client.getTransactions (blockchainId: blockchainId,
                                addresses: [],
                                begBlockNumber: 0,
                                endBlockNumber: 1,
                                includeRaw: true,
                                includeTransfers: false) {
                                    (res: Result<[SystemClient.Transaction], SystemClientError>) in
                                    guard case let .success (transactions) = res
                                        else { XCTAssert(false); return }

                                    XCTAssertTrue (transactions.isEmpty)
                                    self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)
        #endif

        ///
        ///
        ///
        expectation = XCTestExpectation (description: "transactions /w addresses nonsense")

        client.getTransactions (blockchainId: blockchainId,
                                addresses: ["2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT"],
                                begBlockNumber: nil,
                                endBlockNumber: nil,
                                includeRaw: true,
                                includeTransfers: false) {
                                    (res: Result<[SystemClient.Transaction], SystemClientError>) in
                                    guard case let .success (transactions) = res
                                        else { XCTAssert(false); return }

                                    XCTAssertEqual (2, transactions.count)
                                    self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)

        ///
        ///
        ///
        expectation = XCTestExpectation (description: "transactions /w addresses")

        client.getTransactions (blockchainId: blockchainId,
                                addresses: ["2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT"],
                                begBlockNumber: 1446080,
                                endBlockNumber: 1446090,
                                includeRaw: false,
                                includeTransfers: false) {
                                    (res: Result<[SystemClient.Transaction], SystemClientError>) in
                                    guard case let .success (transactions) = res
                                        else { XCTAssert(false); return }

                                    XCTAssertEqual (2, transactions.count)
                                    self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)

        ///
        ///
        ///
        expectation = XCTestExpectation (description: "transactions w/ addresses w/ one-per-page")

        client.getTransactions (blockchainId: blockchainId,
                                addresses: ["2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT"],
                                begBlockNumber: 1446080,
                                endBlockNumber: 1446090,
                                includeRaw: false,
                                includeProof: false,
                                includeTransfers: false,
                                maxPageSize: 1) {
                                    (res: Result<[SystemClient.Transaction], SystemClientError>) in
                                    guard case let .success (transactions) = res
                                        else { XCTAssert(false); return }

                                    XCTAssertEqual (2, transactions.count)
                                    self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)

        ///
        ///
        ///
        #if false
        expectation = XCTestExpectation (description: "transactions w/ [0,11000) w/ no address")

        client.getTransactions (blockchainId: blockchainId,
                                addresses: [],
                                begBlockNumber: 1446080,
                                endBlockNumber: 1446090,
                                includeRaw: true) {
                                    (res: Result<[SystemClient.Transaction], SystemClientError>) in
                                    // A 'status' 400
                                    guard case let .success(transactions) = res
                                        else { XCTAssert(false); return }

                                    XCTAssert(transactions.isEmpty)
                                    self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)
        #endif
    }

    func testBlocks () {
        expectation = XCTestExpectation (description: "block")

        //        let blockId = "bitcoin-mainnet:0000000000000000001ed7770597decf0f98fe4f099111c6a0073ceabbd1e812"
        let blockId = "bitcoin-testnet:000000000000004deedbdb977277330aa156385bbc60ddc3e49938556b436330"
        client.getBlock (blockId: blockId, includeRaw: true) { (res: Result<SystemClient.Block, SystemClientError>) in
            guard case let .success (block) = res
                else { XCTAssert(false); return }

            XCTAssertEqual (blockId, block.id)
            self.expectation.fulfill()
        }
        wait (for: [expectation], timeout: 60)

        ///
        ///
        ///
        expectation = XCTestExpectation (description: "blocks")

        let blockchainId = "bitcoin-testnet"
        client.getBlocks (blockchainId: blockchainId,
                          begBlockNumber: 1446080,
                          endBlockNumber: 1446090,
                          includeRaw: true) { (res: Result<[SystemClient.Block], SystemClientError>) in
                            guard case let .success (blocks) = res
                                else { XCTAssert(false); return }

                            XCTAssertEqual ((1446090 - 1446080),  blocks.count)
                            self.expectation.fulfill()
        }

        wait (for: [expectation], timeout: 60)

        ///
        ///
        ///
        expectation = XCTestExpectation (description: "blocks")
        client.getBlocks (blockchainId: blockchainId,
                          begBlockNumber: 1446080,
                          endBlockNumber: 1446090,
                          includeRaw: true,
                          includeTx: false,
                          includeTxRaw: false,
                          includeTxProof: false,

                          maxPageSize: 1) { (res: Result<[SystemClient.Block], SystemClientError>) in
                            guard case let .success (blocks) = res
                                else { XCTAssert(false); return }

                            XCTAssertEqual ((1446090 - 1446080),  blocks.count)
                            self.expectation.fulfill()
        }
    }

    func testSubscription () {
        #if os(Linux) || os(macOS)
        let deviceId = "some-random-device-id"
        #else
        let deviceId = UIDevice.current.identifierForVendor!.uuidString
        #endif

        /// environment : { unknown, production, development }
        /// kind        : { unknown, apns, fcm, ... }
        /// value       : For apns/fcm this will be the registration token, apns should be hex-encoded
        let endpoint =  (environment: "development",
                         kind: "apns",
                         value: "apns registration token")

        let currencies: [SystemClient.SubscriptionCurrency] = [
            (addresses: [
                "2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT",
                "mvnSpXB1Vizfg3uodBx418APVK1jQXScvW"
                ],
             currencyId: "bitcoin-testnet:__native__",
             events: [
                (name: "confirmed", confirmations: [1])
            ])
        ]

        var subscription: SystemClient.Subscription =
            (id: "ignore",
             device: deviceId,
             endpoint: endpoint,
             currencies: currencies)

        var subscriptionCountObserved: Int  = 0;
        expectation = XCTestExpectation (description: "subscription get all")
        client.getSubscriptions { (res: Result<[SystemClient.Subscription], SystemClientError>) in
            guard case let .success (subs) = res
                else { XCTAssert (false); return }

            // Depending on the 'Bearer Authorization Token' we might have subscriptions.  So
            // don't be so picky on how many 'subs' we have.
            subscriptionCountObserved = subs.count;
            self.expectation.fulfill();
        }
        wait (for: [expectation], timeout: 10)

        expectation = XCTestExpectation (description: "subscription create")
        client.createSubscription(subscription) { (res: Result<SystemClient.Subscription, SystemClientError>) in
            guard case let .success (sub) = res
                else { XCTAssert (false); return }

            self.expectation.fulfill()
            subscription = sub
        }
        wait (for: [expectation], timeout: 10)

        expectation = XCTestExpectation (description: "subscription get")
        client.getSubscription(id: subscription.id) { (res: Result<SystemClient.Subscription, SystemClientError>) in
            guard case let .success (sub) = res
                else { XCTAssert(false); return }

            XCTAssertEqual(sub.id,  subscription.id)
            XCTAssertEqual(sub.device, subscription.device)
            // ...
            self.expectation.fulfill()
        }
        wait (for: [expectation], timeout: 10)

        expectation = XCTestExpectation (description: "subscription get all too")
        client.getSubscriptions { (res: Result<[SystemClient.Subscription], SystemClientError>) in
            guard case let .success (subs) = res
                else { XCTAssert (false); return }

            XCTAssertEqual(subs.count, 1 + subscriptionCountObserved)
            self.expectation.fulfill();
        }
        wait (for: [expectation], timeout: 10)

        expectation = XCTestExpectation (description: "subscription update")
        subscription.currencies = [
            (addresses: ["2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT"],
             currencyId: "bitcoin-testnet:__native__",
             events: [(name: "confirmed", confirmations: [1])])
        ]
        client.updateSubscription (subscription) { (res: Result<SystemClient.Subscription, SystemClientError>) in
            guard case let .success (sub) = res
                else { XCTAssert(false); return }
            XCTAssertTrue(sub.currencies.count == subscription.currencies.count)
            self.expectation.fulfill()
        }
        wait (for: [expectation], timeout: 10)

        expectation = XCTestExpectation (description: "subscription delete")
        client.deleteSubscription(id: subscription.id) { (res: Result<Void, SystemClientError>) in
            guard case .success = res
                else { XCTAssert(false); return }
            self.expectation.fulfill()
        }
        wait (for: [expectation], timeout: 10)

        expectation = XCTestExpectation (description: "subscription get all tre")
        client.getSubscriptions { (res: Result<[SystemClient.Subscription], SystemClientError>) in
            guard case let .success (subs) = res
                else { XCTAssert (false); return }

            XCTAssertEqual(subs.count, subscriptionCountObserved)
            self.expectation.fulfill();
        }
        wait (for: [expectation], timeout: 10)
    }

    static var allTests = [
        ("testBlockchains",  testBlockchains),
        ("testCurrencies",   testCurrencies),
        ("testTransfers",    testTransfers),
        ("testTransactions", testTransactions),
        ("testBlocks",       testBlocks),
        ("testSubscription", testSubscription),
    ]
}
