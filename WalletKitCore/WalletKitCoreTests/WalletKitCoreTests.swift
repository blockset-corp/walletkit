import XCTest
@testable import WalletKitCore
@testable import WalletKitCoreSupportTests

final class WalletKitCoreTests: XCTestCase {

    struct AccountSpecification {
         let identifier: String
         let network: String
         let paperKey: String
         let timestamp: Date

         init (dict: [String: String]) {
             self.identifier = dict["identifier"]! //as! String
             self.network    = dict["network"]!
             self.paperKey   = dict["paperKey"]!

             let dateFormatter = DateFormatter()
             dateFormatter.dateFormat = "yyyy-MM-dd"
             dateFormatter.locale = Locale(identifier: "en_US_POSIX")

             self.timestamp = dateFormatter.date(from: dict["timestamp"]!)!
         }
     }

    var paperKey: String! = nil
    var isMainnet = true
    var uids: String = "5766b9fa-e9aa-4b6d-9b77-b5f1136e5e96"
    var isBTC: Int32! = 1 // or is BCH

    var accountSpecifications: [AccountSpecification] = []
     var accountSpecification: AccountSpecification! {
         return accountSpecifications.count > 0
             ? accountSpecifications[0]
             : nil
     }

    var ethAccount: BREthereumAccount!

    override func setUp() {
        super.setUp()

        accountSpecifications = [
            AccountSpecification (dict: [
                "identifier": "ginger",
                "paperKey":   "ginger settle marine tissue robot crane night number ramp coast roast critic",
                "timestamp":  "2018-01-01",
                "network":    (isMainnet ? "mainnet" : "testnet")
                ])
        ]

        paperKey = accountSpecification.paperKey

        ethAccount = ethAccountCreate (paperKey)

        storagePath = FileManager.default
             .urls(for: .documentDirectory, in: .userDomainMask)[0]
             .appendingPathComponent("Core").path

        // Create the storage path
         do {
             try FileManager.default.createDirectory (atPath: storagePath,
                                                      withIntermediateDirectories: true,
                                                      attributes: nil)
         }
         catch {
             XCTAssert(false)
         }
         storagePathClear()


    }

    var storagePath: String!

    func storagePathClear () {
        do {
            if FileManager.default.fileExists(atPath: storagePath) {
                try FileManager.default.removeItem(atPath: storagePath)
            }
        }
        catch {
            print ("Error: \(error)")
            XCTAssert(false)
        }
    }

    // MARK: - Crypto

    func testCrypto () {
        runCryptoTests ()
    }

    func testCryptoWithAccountAndNetworkBTC() {
        let account = cryptoAccountCreate(paperKey, 0, uids)
        defer { cryptoAccountGive (account) }

        let configurations: [(Bool, UInt64)] = [(true, 500_000), (false, 1_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();

            let network = createBitcoinNetwork (isMainnet: isMainnet, blockHeight: blockHeight)
            defer { cryptoNetworkGive (network) }

            let success = runCryptoTestsWithAccountAndNetwork (account, network, storagePath)
            XCTAssertEqual(CRYPTO_TRUE, success)
        }
    }

    func testCryptoWithAccountAndNetworkBCH() {
        let account = cryptoAccountCreate(paperKey, 0, uids)
        defer { cryptoAccountGive (account) }

        let configurations: [(Bool, UInt64)] = [(true, 500_000), (false, 1_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();

            let network = createBitcoinCashNetwork (isMainnet: isMainnet, blockHeight: blockHeight)
            defer { cryptoNetworkGive (network) }

            let success = runCryptoTestsWithAccountAndNetwork (account, network, storagePath)
            XCTAssertEqual(CRYPTO_TRUE, success)
        }
    }

    func testCryptoWithAccountAndNetworkETH() {
        let account = cryptoAccountCreate(paperKey, 0, uids)
        defer { cryptoAccountGive (account) }

        let configurations: [(Bool, UInt64)] = [(true, 8_000_000), (false, 4_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();

            let network = createEthereumNetwork (isMainnet: isMainnet, blockHeight: blockHeight)
            defer { cryptoNetworkGive (network) }

            let success = runCryptoTestsWithAccountAndNetwork (account, network, storagePath)
            XCTAssertEqual(CRYPTO_TRUE, success)
        }
    }

    // MARK: - Ethereum

    func testRLPETH () {
        runRlpTests()
    }

    func testUtilETH () {
        runUtilTests()
    }

    func testEventETH () {
        runEventTests ()
    }

    func testBaseETH () {
        runBaseTests()
    }

    func testBlockchainETH () {
        runBcTests()
    }

    func testContractETH () {
        runContractTests()
    }

    func testBasicsETH () {
        runTests (0)
    }

    func testLESETH () {
        runLESTests(paperKey)
        runNodeTests()
    }

    func testEWM () {
        runEWMTests(paperKey, storagePath)
    }

    /// Run an Etheruem Sync.  Two syncs are run back-to-back with the second sync meant to
    /// start from the saved state of the first sync.
    ///
    /// Note: The first sync saves state to the file system..
    ///
    /// - Throws: something
    ///
    func testEthereumSyncStorage () throws {
        let mode = CRYPTO_SYNC_MODE_P2P_ONLY;
        let timestamp : UInt64 = 0

        let network = (isMainnet ? ethNetworkMainnet : ethNetworkTestnet)

        print ("ETH: TST: Core Dir: \(storagePath!)")
        storagePathClear()
        runSyncTest (network, ethAccount, mode, timestamp, 5 * 60, storagePath);
        runSyncTest (network, ethAccount, mode, timestamp, 1 * 60, storagePath);
    }


    // MARK: - Ripple

    func testRipple () {
        runRippleTest()
    }

    // MARK: - Hedera
    func testHedera () {
        runHederaTest()
    }
    
    // MARK: - Tezos
    func testTezos () {
        runTezosTest()
    }
    
    // MARK: - Bitcoin

    func testBitcoinSupport () {
        XCTAssert(1 == BRRunSupTests ())
    }

    func testBitcoin () {
        XCTAssert(1 == BRRunTests())
        XCTAssert(1 == BRRunTestsBWM (paperKey, storagePath, isBTC, (isMainnet ? 1 : 0)));
    }

    func testBitcoinSyncOne() {
        BRRunTestsSync (paperKey, isBTC, (isMainnet ? 1 : 0));
    }

    func runBitcoinSyncMany (_ count: Int) {
        let group = DispatchGroup.init()
        for i in 1...count {
            DispatchQueue.init(label: "Sync \(i)")
                .async {
                    group.enter()
                    let paperKey = i <= self.accountSpecifications.count ? self.accountSpecifications[i - 1].paperKey : nil
                    BRRunTestsSync (paperKey, self.isBTC, (self.isMainnet ? 1 : 0));
                    group.leave()
            }
        }
        group.wait()
    }

    /// Run 25 simultaneous bitcoin syncs using the provided paperKeys and random keys after that.
    func XtestBitcoinSyncMany () {
        runBitcoinSyncMany(25)
    }

    func XtestBitcoinSyncAll () {
        for useBTC in [false, true] {
            for useMainnet in [false, true] {
                self.isBTC = useBTC ? 1 : 0
                self.isMainnet = useMainnet
                runBitcoinSyncMany (10)
            }
        }
    }

    ///
    /// Run a bitcoin sync using the (new) BRWalletManager which encapsulates BRWallet and
    /// BRPeerManager with 'save' callbacks using the file system.
    ///
    func testBitcoinWalletManagerSync () {
        print ("BTC: TST: Core Dir: \(storagePath!)")
        storagePathClear()
        BRRunTestWalletManagerSync (paperKey, storagePath, isBTC, (isMainnet ? 1 : 0));
        BRRunTestWalletManagerSync (paperKey, storagePath, isBTC, (isMainnet ? 1 : 0));
    }

    func XtestBitcoinWalletManagerSyncStressBTC() {
        let configurations: [(Int32, UInt64)] = [(1, 500_000), (0, 1_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();
            let success = BRRunTestWalletManagerSyncStress(paperKey, storagePath, 0, blockHeight, 1, isMainnet);
            XCTAssertEqual(1, success)
        }
    }

    func XtestBitcoinWalletManagerSyncStressBCH() {
        let configurations: [(Int32, UInt64)] = [(1, 500_000), (0, 1_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();
            let success = BRRunTestWalletManagerSyncStress(paperKey, storagePath, 0, blockHeight, 0, isMainnet);
            XCTAssertEqual(1, success)
        }
    }

    func XtestPerformanceExample() {
        //        runTests(0);
        self.measure {
            runPerfTestsCoder (10, 0);
        }
    }

    private func createBitcoinNetwork(isMainnet: Bool, blockHeight: UInt64) -> BRCryptoNetwork {
        let uids = "bitcoin-" + (isMainnet ? "mainnet" : "testnet")
        let network = cryptoNetworkFindBuiltin(uids);
        defer { cryptoNetworkGive (network) }

        let currency = cryptoCurrencyCreate ("bitcoin", "bitcoin", "btc", "native", nil)
        defer { cryptoCurrencyGive (currency) }

        let satUnit = cryptoUnitCreateAsBase (currency, "sat", "satoshis", "SAT")
        defer { cryptoUnitGive (satUnit) }

        let btcUnit = cryptoUnitCreate (currency, "btc", "bitcoin", "B", satUnit, 8)
        defer { cryptoUnitGive (btcUnit) }

        let factor = cryptoAmountCreateInteger (1_000, satUnit)
        defer { cryptoAmountGive (factor) }

        let fee = cryptoNetworkFeeCreate (30_000, factor, satUnit)
        defer { cryptoNetworkFeeGive (fee) }

        cryptoNetworkSetHeight (network, blockHeight)

        cryptoNetworkSetCurrency (network, currency)
        cryptoNetworkAddCurrency (network, currency, satUnit, btcUnit)

        cryptoNetworkAddCurrencyUnit(network, currency, satUnit)
        cryptoNetworkAddCurrencyUnit(network, currency, btcUnit)

        cryptoNetworkAddNetworkFee(network, fee)

        return cryptoNetworkTake (network)
    }

    private func createBitcoinCashNetwork(isMainnet: Bool, blockHeight: UInt64) -> BRCryptoNetwork {
        let uids = "bitcoincash-" + (isMainnet ? "mainnet" : "testnet")
        let network = cryptoNetworkFindBuiltin(uids);
        defer { cryptoNetworkGive (network) }

        let currency = cryptoCurrencyCreate ("bitcoin-cash", "bitcoin cash", "bch", "native", nil)
        defer { cryptoCurrencyGive (currency) }

        let satUnit = cryptoUnitCreateAsBase (currency, "sat", "satoshis", "SAT")
        defer { cryptoUnitGive (satUnit) }

        let btcUnit = cryptoUnitCreate (currency, "btc", "bitcoin", "B", satUnit, 8)
        defer { cryptoUnitGive (btcUnit) }

        let factor = cryptoAmountCreateInteger (1_000, satUnit)
        defer { cryptoAmountGive (factor) }

        let fee = cryptoNetworkFeeCreate (30_000, factor, satUnit)
        defer { cryptoNetworkFeeGive (fee) }

        cryptoNetworkSetHeight (network, blockHeight)

        cryptoNetworkSetCurrency (network, currency)
        cryptoNetworkAddCurrency (network, currency, satUnit, btcUnit)

        cryptoNetworkAddCurrencyUnit(network, currency, satUnit)
        cryptoNetworkAddCurrencyUnit(network, currency, btcUnit)

        cryptoNetworkAddNetworkFee(network, fee)

        return cryptoNetworkTake (network)
    }

    private func createEthereumNetwork(isMainnet: Bool, blockHeight: UInt64) -> BRCryptoNetwork {
        let uids = "ethereum-" + (isMainnet ? "mainnet" : "ropsten")
        let network = cryptoNetworkFindBuiltin (uids)
        defer { cryptoNetworkGive (network) }

        let currency = cryptoCurrencyCreate ("ethereum", "ethereum", "eth", "native", nil)
        defer { cryptoCurrencyGive (currency) }

        let weiUnit = cryptoUnitCreateAsBase (currency, "wei", "wei", "wei")
        defer { cryptoUnitGive (weiUnit) }

        let gweiUnit = cryptoUnitCreate (currency, "gwei", "gwei", "gwei", weiUnit, 9)
        defer { cryptoUnitGive (gweiUnit) }

        let etherUnit = cryptoUnitCreate (currency, "ether", "ether", "ether", weiUnit, 18)
        defer { cryptoUnitGive (etherUnit) }

        let factor = cryptoAmountCreateDouble (2.0, gweiUnit)
        defer { cryptoAmountGive (factor) }

        let fee = cryptoNetworkFeeCreate (1_000, factor, gweiUnit)
        defer { cryptoNetworkFeeGive (fee) }

        cryptoNetworkSetHeight (network, blockHeight)

        cryptoNetworkSetCurrency (network, currency)
        cryptoNetworkAddCurrency (network, currency, weiUnit, etherUnit)

        cryptoNetworkAddCurrencyUnit(network, currency, weiUnit)
        cryptoNetworkAddCurrencyUnit(network, currency, gweiUnit)
        cryptoNetworkAddCurrencyUnit(network, currency, etherUnit)

        cryptoNetworkAddNetworkFee(network, fee)

        return cryptoNetworkTake (network)
    }

    static var allTests = [
        // Crypto
        ("testCrypto",          testCrypto),
        ("testCryptoBTC",       testCryptoWithAccountAndNetworkBTC),
        ("testCryptoBCH",       testCryptoWithAccountAndNetworkBCH),
        ("testCryptoETH",       testCryptoWithAccountAndNetworkETH),

        // Ethereum
        ("testRLP",             testRLPETH),
        ("testUtil",            testUtilETH),
        ("testEvent",           testEventETH),
        ("testBase",            testBaseETH),
        ("testBC",              testBlockchainETH),
        ("testContracdt",       testContractETH),
        ("testBasics",          testBasicsETH),
        ("testLES",             testLESETH),
        ("testEWM",             testEWM),

        // Ripple
        ("testRipple",          testRipple),

        // Hedera
        ("testHedera",          testHedera),
        
        // Tezos
        ("testTezos",           testTezos),

        // Bitcoin
        ("testSupportBTC",      testBitcoinSupport),
        ("testBTC",             testBitcoin),
        ("testSyncOneBTC",      testBitcoinSyncOne),
        ("testManaagerSyncBTC", testBitcoinWalletManagerSync)
    ]
}
