//
//  BRCryptoBaseTests.swift
//  WalletKitTests
//
//  Created by Ed Gamble on 3/28/19.
//  Copyright © 2019 Breadwallet AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

import XCTest
@testable import WalletKit

class BRCryptoBaseTests: XCTestCase {
    var accountSpecifications: [AccountSpecification] = []
    var accountSpecification: AccountSpecification! {
        return accountSpecifications.count > 0
            ? accountSpecifications[0]
            : nil
    }

    var isMainnet = true

    // let configPath = Bundle(for: BRCryptoBaseTests.self).path(forResource: "CoreTestsConfig", ofType: "json") ?? "foo"

    var coreDataDir: String!

    func coreDirClear () {
        do {
            if FileManager.default.fileExists(atPath: coreDataDir) {
                try FileManager.default.removeItem(atPath: coreDataDir)
            }
        }
        catch {
            print ("Error: \(error)")
            XCTAssert(false)
        }
    }

    func coreDirCreate () {
        do {
            try FileManager.default.createDirectory (atPath: coreDataDir,
                                                     withIntermediateDirectories: true,
                                                     attributes: nil)
        }
        catch {
            XCTAssert(false)
        }
    }

    var account: Account!

    func prepareAccount (_ spec: AccountSpecification? = nil, identifier: String? = nil) {

        // If provided with CommandLine arguments, the first is locate of .brdCoreTestsConfig.json
        let configPath: String? = (CommandLine.argc > 1
            ? NSString (string: CommandLine.arguments[1]).expandingTildeInPath
            : nil)

        // If there is no `configPath` file, use this as the default AccountSpecification.  This
        // default must have "network" match `isMainnet`
        let defaultSpecification = AccountSpecification (dict: (isMainnet
            ? [
                "identifier": "loan(C)",
                "paperKey":   "loan father fancy category panel render dinosaur mixture spy neck grocery habit",
                "timestamp":  "2018-07-01",
                "network":    "mainnet",
                "content":    "compromised"
                ]
            : [
                "identifier": "ginger",
                "paperKey":   "ginger settle marine tissue robot crane night number ramp coast roast critic",
                "timestamp":  "2018-01-01",
                "network":    "testnet",
                "content":    ""
            ]))

        // If `spec` was provided explicitly, then use it
        self.accountSpecifications = spec.map { [$0] }
            ?? AccountSpecification.loadFrom (configPath: configPath, defaultSpecification: defaultSpecification)
                .filter { $0.network == (isMainnet ? "mainnet" : "testnet") }

        // If there is an identifier, filter
        if let id = identifier {
            self.accountSpecifications.removeAll { $0.identifier != id }
        }

        /// Create the account
        let walletId = UUID (uuidString: "5766b9fa-e9aa-4b6d-9b77-b5f1136e5e96")?.uuidString ?? "empty-wallet-id"
        account = Account.createFrom (phrase: accountSpecification.paperKey,
                                      timestamp: accountSpecification.timestamp,
                                      uids: walletId)


    }
    
    override func setUp() {
        super.setUp()

        /// Create the 'storagePath'
        coreDataDir = FileManager.default
            .urls(for: .documentDirectory, in: .userDomainMask)[0]
            .appendingPathComponent("Core").path

        coreDirCreate()
        coreDirClear()
        XCTAssert (nil != coreDataDir)

        print ("TST: StoragePath: \(coreDataDir ?? "<none>")");
   }

    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
    }
}

///
/// Listeners
///

class TestWalletManagerListener: WalletManagerListener {
    let handler: WalletManagerEventHandler

    init (_ handler: @escaping WalletManagerEventHandler) {
        self.handler = handler
    }

    func handleManagerEvent(system: System, manager: WalletManager, event: WalletManagerEvent) {
        handler (system, manager, event)
    }
}

class TestWalletListener: WalletListener {
    let handler: WalletEventHandler

    init (_ handler: @escaping WalletEventHandler) {
        self.handler = handler
    }

    func handleWalletEvent(system: System, manager: WalletManager, wallet: Wallet, event: WalletEvent) {
        handler (system, manager, wallet, event)
    }
}

class TestTransferListener: TransferListener {
    let handler: TransferEventHandler

    init (_ handler: @escaping TransferEventHandler) {
        self.handler = handler
    }

    func handleTransferEvent(system: System, manager: WalletManager, wallet: Wallet, transfer: Transfer, event: TransferEvent) {
        handler (system, manager, wallet, transfer, event)
    }
}

class TestNetworkListener: NetworkListener {
    let handler: NetworkEventHandler

    init (_ handler: @escaping NetworkEventHandler) {
        self.handler = handler
    }
    func handleNetworkEvent(system: System, network: Network, event: NetworkEvent) {
        handler (system, network, event)
    }
}

class CryptoTestSystemListener: SystemListener {
    private let queue = DispatchQueue (label: "Crypto System")

    private let isMainnet: Bool
    private let networkCurrencyCodesToMode: [String:WalletManagerMode]
    private let registerCurrencyCodes: [String]

    public init (networkCurrencyCodesToMode: [String:WalletManagerMode],
                 registerCurrencyCodes: [String],
                 isMainnet: Bool) {
        self.networkCurrencyCodesToMode = networkCurrencyCodesToMode
        self.registerCurrencyCodes = registerCurrencyCodes;
        self.isMainnet = isMainnet
    }

    var systemHandlers: [SystemEventHandler] = []
    var systemEvents: [SystemEvent] = []

    func handleSystemEvent(system: System, event: SystemEvent) {
        print ("TST: System Event: \(event)")
        queue.async {
            
            self.systemEvents.append (event)
            switch event {
            case .networkAdded (let network):
                if self.isMainnet == network.onMainnet,
                    network.currencies.contains(where: { nil != self.networkCurrencyCodesToMode[$0.code] }),
                    let currencyMode = self.networkCurrencyCodesToMode [network.currency.code] {
                    // Get a valid mode, ideally from `currencyMode`
                    
                    let mode = network.supportsMode (currencyMode)
                        ? currencyMode
                        : network.defaultMode
                    
                    let scheme = network.defaultAddressScheme
                    
                    let currencies = network.currencies
                        .filter { (c) in self.registerCurrencyCodes.contains { c.code == $0 } }
                    
                    let success = system.createWalletManager (network: network,
                                                              mode: mode,
                                                              addressScheme: scheme,
                                                              currencies: currencies)
                    XCTAssertTrue(success)
                }
                
            case .discoveredNetworks:
                self.networkExpectation.fulfill()
                
            case .managerAdded:
                self.managerExpectation.fulfill()
                
            default: break
            }
            
            self.systemHandlers.forEach { $0 (system, event) }
        }
    }

    func checkSystemEvents (_ expected: [SystemEvent], strict: Bool = false) -> Bool {
        return checkSystemEvents (expected.map { EventMatcher (event: $0, strict: strict, scan: false) })
    }

    func checkSystemEvents (_ matchers: [EventMatcher<SystemEvent>]) -> Bool {
        return queue.sync { systemEvents.match(matchers) }
    }

    // MARK: - Wallet Manager Handler

    var managerHandlers: [WalletManagerEventHandler] = []
    var managerEvents: [WalletManagerEvent] = []
    var managerExpectation = XCTestExpectation (description: "ManagerExpectation")

    func handleManagerEvent(system: System, manager: WalletManager, event: WalletManagerEvent) {
        print ("TST: Manager Event: \(event)")
        queue.async {
            self.managerEvents.append(event)
            if case .walletAdded = event {
                if self.walletExpectationNeeded {
                    self.walletExpectationNeeded = false
                    self.walletExpectation.fulfill()   // Might fulfill on ETH before BRD?
                }
            }
            self.managerHandlers.forEach { $0 (system, manager, event) }
        }
    }

    func checkManagerEvents (_ expected: [WalletManagerEvent], strict: Bool = false) -> Bool {
        return checkManagerEvents (expected.map { EventMatcher (event: $0, strict: strict, scan: false) })
    }

    func checkManagerEvents (_ matchers: [EventMatcher<WalletManagerEvent>]) -> Bool {
        return managerEvents.match(matchers)
    }

    // MARK: - Wallet Handler

    var walletHandlers: [WalletEventHandler] = []
    var walletEvents: [WalletEvent] = []
    var walletExpectation  = XCTestExpectation (description: "WalletExpectation")
    var walletExpectationNeeded = true

    func handleWalletEvent(system: System, manager: WalletManager, wallet: Wallet, event: WalletEvent) {
        print ("TST: Wallet (\(wallet.name)) Event: \(event)")
        queue.async {
            self.walletEvents.append(event)
            self.walletHandlers.forEach { $0 (system, manager, wallet, event) }
        }
    }

    func checkWalletEvents (_ expected: [WalletEvent], strict: Bool = false) -> Bool {
        return checkWalletEvents (expected.map { EventMatcher (event: $0, strict: strict, scan: false) })
    }

    func checkWalletEvents (_ matchers: [EventMatcher<WalletEvent>]) -> Bool {
        return queue.sync { walletEvents.match(matchers) }
    }

    // MARK: - Transfer Handler

    var transferIncluded: Bool = false
    var transferCount: Int = 0;
    var transferHandlers: [TransferEventHandler] = []
    var transferEvents: [TransferEvent] = []
    var transferExpectation = XCTestExpectation (description: "TransferExpectation")
    var transferWallet: Wallet? = nil

    func handleTransferEvent(system: System, manager: WalletManager, wallet: Wallet, transfer: Transfer, event: TransferEvent) {
        guard transferWallet.map ({ $0 == wallet }) ?? true
            else { return }

        print ("TST: Transfer Event: \(event)")
        queue.async {
            self.transferEvents.append (event)
            if self.transferIncluded, case .included = transfer.state {
                if 1 <= self.transferCount { self.transferCount -= 1 }
                if 0 == self.transferCount { self.transferExpectation.fulfill()}
            }
            else if !self.transferIncluded, case .created = transfer.state {
                if 1 <= self.transferCount { self.transferCount -= 1 }
                if 0 == self.transferCount { self.transferExpectation.fulfill()}
            }
            self.transferHandlers.forEach { $0 (system, manager, wallet, transfer, event) }
        }
    }

   func checkTransferEvents (_ expected: [TransferEvent], strict: Bool = false) -> Bool {
        return checkTransferEvents (expected.map { EventMatcher (event: $0, strict: strict, scan: false) })
    }

    func checkTransferEvents (_ matchers: [EventMatcher<TransferEvent>]) -> Bool {
        return queue.sync { transferEvents.match(matchers) }
    }

    // MARK: - Network Handler

    var networkHandlers: [NetworkEventHandler] = []
    var networkEvents: [NetworkEvent] = []
    var networkExpectation = XCTestExpectation (description: "NetworkExpectation")

    func handleNetworkEvent(system: System, network: Network, event: NetworkEvent) {
        print ("TST: Network Event: \(event)")
        queue.async {
            self.networkEvents.append (event)
            self.networkHandlers.forEach { $0 (system, network, event) }
        }
    }

    func checkNetworkEvents (_ expected: [NetworkEvent], strict: Bool = false) -> Bool {
        return checkNetworkEvents (expected.map { EventMatcher (event: $0, strict: strict, scan: false) })
    }

    func checkNetworkEvents (_ matchers: [EventMatcher<NetworkEvent>]) -> Bool {
        return queue.sync { self.networkEvents.match(matchers) }
    }

    //
    // Common Sequences
    //

    func checkSystemEventsCommonlyWith (network: Network, manager: WalletManager) -> Bool {
        return checkSystemEvents(
            [EventMatcher (event: SystemEvent.created),
             EventMatcher (event: SystemEvent.networkAdded(network: network), strict: true, scan: true),
             EventMatcher (event: SystemEvent.managerAdded(manager: manager), strict: true, scan: true)
        ])
    }

    func checkManagerEventsCommonlyWith (mode: WalletManagerMode,
                                         wallet: Wallet,
                                         lenientDisconnect: Bool = false) -> Bool {
        let disconnectReason = (mode == WalletManagerMode.api_only
            ? WalletManagerDisconnectReason.requested
            : (mode == WalletManagerMode.p2p_only
                ? WalletManagerDisconnectReason.posix (errno: 54, message: "Connection reset by peer")
                : WalletManagerDisconnectReason.unknown))
        
        let disconnectEventMatcher =
            EventMatcher (event: WalletManagerEvent.changed (oldState: WalletManagerState.connected,
                                                             newState: WalletManagerState.disconnected (reason: disconnectReason)),
                          strict: !lenientDisconnect,
                          scan: true)       // block height updates might intervene.

        return checkManagerEvents(
            [EventMatcher (event: WalletManagerEvent.created),
             EventMatcher (event: WalletManagerEvent.walletAdded (wallet: wallet)),
             EventMatcher (event: WalletManagerEvent.changed (oldState: WalletManagerState.created,
                                                              newState: WalletManagerState.connected)),
             EventMatcher (event: WalletManagerEvent.changed (oldState: WalletManagerState.connected,
                                                              newState: WalletManagerState.syncing)),
             EventMatcher (event: WalletManagerEvent.syncStarted),

             // On API_ONLY here is no .syncProgress: timestamp: nil, percentComplete: 0
                // EventMatcher (event: WalletManagerEvent.walletChanged(wallet: wallet), strict: true, scan: true),

                EventMatcher (event: WalletManagerEvent.syncEnded (reason: WalletManagerSyncStoppedReason.complete), strict: false, scan: true),
                EventMatcher (event: WalletManagerEvent.changed (oldState: WalletManagerState.syncing,
                                                                 newState: WalletManagerState.connected)),
                disconnectEventMatcher
        ])
    }
    
    func checkWalletEventsCommonlyWith (mode: WalletManagerMode, balance: Amount, transfer: Transfer) -> Bool {
        switch mode {
        case .p2p_only:
            return checkWalletEvents(
                [EventMatcher (event: WalletEvent.created),
                 EventMatcher (event: WalletEvent.transferAdded(transfer: transfer), strict: true, scan: true),
                 EventMatcher (event: WalletEvent.balanceUpdated(amount: balance), strict: true, scan: true),
            ])
        case .api_only:
            // Balance before transfer... doesn't seem right. But worse, all balance events arrive
            // before all transfer events.
            return checkWalletEvents(
                [EventMatcher (event: WalletEvent.created),
                 EventMatcher (event: WalletEvent.transferAdded(transfer: transfer), strict: true, scan: true),
                 EventMatcher (event: WalletEvent.balanceUpdated(amount: balance), strict: true, scan: true),
           ])
        default:
            return false
        }
    }
}

class BRCryptoSystemBaseTests: BRCryptoBaseTests {

    var listener: CryptoTestSystemListener!
    var client: SystemClient!
    var system: System!

    var registerCurrencyCodes = [String]()
    var currencyCodesToMode = ["btc":WalletManagerMode.api_only]

    var currencyModels: [SystemClient.Currency] = []

    func createDefaultListener() -> CryptoTestSystemListener {
        return CryptoTestSystemListener (networkCurrencyCodesToMode: currencyCodesToMode,
                                         registerCurrencyCodes: registerCurrencyCodes,
                                         isMainnet: isMainnet)
    }

    func createDefaultClient () -> SystemClient {
        guard let testConfiguration = TestConfiguration.loadFrom (bundle: Bundle.module)
        else { preconditionFailure("No TestConfiguration") }

        return BlocksetSystemClient.createForTest(blocksetAccess: testConfiguration.blocksetAccess)
    }

    func prepareSystem (listener: CryptoTestSystemListener? = nil, client: SystemClient? = nil) {

        self.listener = listener ?? createDefaultListener()
        self.client   = client   ?? createDefaultClient()

        system = System (client:    self.client,
                         listener:  self.listener,
                         account:   self.account,
                         onMainnet: self.isMainnet,
                         path:      self.coreDataDir)

        XCTAssertEqual (coreDataDir + "/" + self.account.fileSystemIdentifier, system.path)
        XCTAssertEqual (account.uids, system.account.uids)

        system.configure()
        wait (for: [self.listener.networkExpectation], timeout: 15)
        wait (for: [self.listener.managerExpectation], timeout: 15)
        wait (for: [self.listener.walletExpectation ], timeout: 15)
    }

    override func setUp() {
        super.setUp()
    }
}
