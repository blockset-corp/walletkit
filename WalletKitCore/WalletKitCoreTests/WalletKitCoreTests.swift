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
    var bitcoinChain: BRBitcoinChain = BITCOIN_CHAIN_BTC

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
        let account = wkAccountCreate(paperKey, 0, uids)
        defer { wkAccountGive (account) }

        let configurations: [(Bool, UInt64)] = [(true, 500_000), (false, 1_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();

            let network = createBitcoinNetwork (isMainnet: isMainnet, blockHeight: blockHeight)
            defer { wkNetworkGive (network) }

            let success = runCryptoTestsWithAccountAndNetwork (account, network, storagePath)
            XCTAssertEqual(WK_TRUE, success)
        }
    }

    func testCryptoWithAccountAndNetworkBCH() {
        let account = wkAccountCreate(paperKey, 0, uids)
        defer { wkAccountGive (account) }

        let configurations: [(Bool, UInt64)] = [(true, 500_000), (false, 1_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();

            let network = createBitcoinCashNetwork (isMainnet: isMainnet, blockHeight: blockHeight)
            defer { wkNetworkGive (network) }

            let success = runCryptoTestsWithAccountAndNetwork (account, network, storagePath)
            XCTAssertEqual(WK_TRUE, success)
        }
    }
    
    func testCryptoWithAccountAndNetworkBSV() {
        let account = wkAccountCreate(paperKey, 0, uids)
        defer { wkAccountGive (account) }

        let configurations: [(Bool, UInt64)] = [(true, 500_000), (false, 1_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();

            let network = createBitcoinSVNetwork (isMainnet: isMainnet, blockHeight: blockHeight)
            defer { wkNetworkGive (network) }

            let success = runCryptoTestsWithAccountAndNetwork (account, network, storagePath)
            XCTAssertEqual(WK_TRUE, success)
        }
    }

    func testCryptoWithAccountAndNetworkETH() {
        let account = wkAccountCreate(paperKey, 0, uids)
        defer { wkAccountGive (account) }

        let configurations: [(Bool, UInt64)] = [(true, 8_000_000), (false, 4_500_000),]
        configurations.forEach { (isMainnet, blockHeight) in
            storagePathClear();

            let network = createEthereumNetwork (isMainnet: isMainnet, blockHeight: blockHeight)
            defer { wkNetworkGive (network) }

            let success = runCryptoTestsWithAccountAndNetwork (account, network, storagePath)
            XCTAssertEqual(WK_TRUE, success)
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

    // Mark: - Avalanche
    
//    func testAvalanche()
//    {
//        runAvalancheTest()
//    }

    func testAvalanchePrototype()
    {
        runAvalanchePrototypeTest()
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

    // MARK: - Stellar
    func testStellar () {
        runStellarTest()
    }

    // MARK: - Bitcoin

    func testBitcoinSupport () {
        XCTAssert(1 == BRRunSupTests ())
    }

    func testBitcoin () {
        XCTAssert(1 == BRRunTests())
//        XCTAssert(1 == BRRunTestsBWM (paperKey, storagePath, bitcoinChain, (isMainnet ? 1 : 0)));
    }

    func testBitcoinSyncOne() {
        BRRunTestsSync (paperKey, bitcoinChain, (isMainnet ? 1 : 0));
    }

    func runBitcoinSyncMany (_ count: Int) {
        let group = DispatchGroup.init()
        for i in 1...count {
            DispatchQueue.init(label: "Sync \(i)")
                .async {
                    group.enter()
                    let paperKey = i <= self.accountSpecifications.count ? self.accountSpecifications[i - 1].paperKey : nil
                    BRRunTestsSync (paperKey, self.bitcoinChain, (self.isMainnet ? 1 : 0));
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
        for chain in [BITCOIN_CHAIN_BTC, BITCOIN_CHAIN_BCH, BITCOIN_CHAIN_BSV] {
            for useMainnet in [false, true] {
                self.bitcoinChain = chain
                self.isMainnet = useMainnet
                runBitcoinSyncMany (10)
            }
        }
    }

    ///
    /// Run a bitcoin sync using the (new) BRWalletManager which encapsulates BRWallet and
    /// BRPeerManager with 'save' callbacks using the file system.
    ///
    #if REFACTOR
    func testBitcoinWalletManagerSync () {
        print ("BTC: TST: Core Dir: \(storagePath!)")
        storagePathClear()
        BRRunTestWalletManagerSync (paperKey, storagePath, bitcoinChain, (onMainnet ? 1 : 0));
        BRRunTestWalletManagerSync (paperKey, storagePath, bitcoinChain, (onMainnet ? 1 : 0));
    }

    func XtestBitcoinWalletManagerSyncStressBTC() {
        let configurations: [(Int32, UInt64)] = [(1, 500_000), (0, 1_500_000),]
        configurations.forEach { (onMainnet, blockHeight) in
            storagePathClear();
            let success = BRRunTestWalletManagerSyncStress(paperKey, storagePath, 0, blockHeight, BITCOIN_CHAIN_BTC, onMainnet);
            XCTAssertEqual(1, success)
        }
    }

    func XtestBitcoinWalletManagerSyncStressBCH() {
        let configurations: [(Int32, UInt64)] = [(1, 500_000), (0, 1_500_000),]
        configurations.forEach { (onMainnet, blockHeight) in
            storagePathClear();
            let success = BRRunTestWalletManagerSyncStress(paperKey, storagePath, 0, blockHeight, BITCOIN_CHAIN_BCH, onMainnet);
            XCTAssertEqual(1, success)
        }
    }
    
    func XtestBitcoinWalletManagerSyncStressBSV() {
        let configurations: [(Int32, UInt64)] = [(1, 500_000), (0, 1_500_000),]
        configurations.forEach { (onMainnet, blockHeight) in
            storagePathClear();
            let success = BRRunTestWalletManagerSyncStress(paperKey, storagePath, 0, blockHeight, BITCOIN_CHAIN_BSV, onMainnet);
            XCTAssertEqual(1, success)
        }
    }
    #endif
    func XtestPerformanceExample() {
        //        runTests(0);
        self.measure {
            runPerfTestsCoder (10, 0);
        }
    }

    private func createBitcoinNetwork(isMainnet: Bool, blockHeight: UInt64) -> WKNetwork {
        let uids = "bitcoin-" + (isMainnet ? "mainnet" : "testnet")
        let network = wkNetworkFindBuiltin(uids, isMainnet);
        defer { wkNetworkGive (network) }

        let currency = wkCurrencyCreate ("bitcoin", "bitcoin", "btc", "native", nil)
        defer { wkCurrencyGive (currency) }

        let satUnit = wkUnitCreateAsBase (currency, "sat", "satoshis", "SAT")
        defer { wkUnitGive (satUnit) }

        let btcUnit = wkUnitCreate (currency, "btc", "bitcoin", "B", satUnit, 8)
        defer { wkUnitGive (btcUnit) }

        let factor = wkAmountCreateInteger (1_000, satUnit)
        defer { wkAmountGive (factor) }

        let fee = wkNetworkFeeCreate (30_000, factor, satUnit)
        defer { wkNetworkFeeGive (fee) }

        wkNetworkSetHeight (network, blockHeight)

        wkNetworkAddCurrency (network, currency, satUnit, btcUnit)

        wkNetworkAddCurrencyUnit(network, currency, satUnit)
        wkNetworkAddCurrencyUnit(network, currency, btcUnit)

        wkNetworkAddNetworkFee(network, fee)

        return wkNetworkTake (network)
    }

    private func createBitcoinCashNetwork(isMainnet: Bool, blockHeight: UInt64) -> WKNetwork {
        let uids = "bitcoincash-" + (isMainnet ? "mainnet" : "testnet")
        let network = wkNetworkFindBuiltin(uids, isMainnet);
        defer { wkNetworkGive (network) }

        let currency = wkCurrencyCreate ("bitcoin-cash", "bitcoin cash", "bch", "native", nil)
        defer { wkCurrencyGive (currency) }

        let satUnit = wkUnitCreateAsBase (currency, "sat", "satoshis", "SAT")
        defer { wkUnitGive (satUnit) }

        let btcUnit = wkUnitCreate (currency, "btc", "bitcoin", "B", satUnit, 8)
        defer { wkUnitGive (btcUnit) }

        let factor = wkAmountCreateInteger (1_000, satUnit)
        defer { wkAmountGive (factor) }

        let fee = wkNetworkFeeCreate (30_000, factor, satUnit)
        defer { wkNetworkFeeGive (fee) }

        wkNetworkSetHeight (network, blockHeight)

        wkNetworkAddCurrency (network, currency, satUnit, btcUnit)

        wkNetworkAddCurrencyUnit(network, currency, satUnit)
        wkNetworkAddCurrencyUnit(network, currency, btcUnit)

        wkNetworkAddNetworkFee(network, fee)

        return wkNetworkTake (network)
    }
    
    private func createBitcoinSVNetwork(isMainnet: Bool, blockHeight: UInt64) -> WKNetwork {
        let uids = "bitcoinsv-" + (isMainnet ? "mainnet" : "testnet")
        let network = wkNetworkFindBuiltin(uids, isMainnet);
        defer { wkNetworkGive (network) }

        let currency = wkCurrencyCreate ("bitcoin-sv", "bitcoin sv", "bsv", "native", nil)
        defer { wkCurrencyGive (currency) }

        let satUnit = wkUnitCreateAsBase (currency, "sat", "satoshis", "SAT")
        defer { wkUnitGive (satUnit) }

        let btcUnit = wkUnitCreate (currency, "bsv", "bitcoinsv", "B", satUnit, 8)
        defer { wkUnitGive (btcUnit) }

        let factor = wkAmountCreateInteger (1_000, satUnit)
        defer { wkAmountGive (factor) }

        let fee = wkNetworkFeeCreate (30_000, factor, satUnit)
        defer { wkNetworkFeeGive (fee) }

        wkNetworkSetHeight (network, blockHeight)

        wkNetworkAddCurrency (network, currency, satUnit, btcUnit)

        wkNetworkAddCurrencyUnit(network, currency, satUnit)
        wkNetworkAddCurrencyUnit(network, currency, btcUnit)

        wkNetworkAddNetworkFee(network, fee)

        return wkNetworkTake (network)
    }

    private func createEthereumNetwork(isMainnet: Bool, blockHeight: UInt64) -> WKNetwork {
        let uids = "ethereum-" + (isMainnet ? "mainnet" : "ropsten")
        let network = wkNetworkFindBuiltin (uids, isMainnet)
        defer { wkNetworkGive (network) }

        let currency = wkCurrencyCreate ("ethereum", "ethereum", "eth", "native", nil)
        defer { wkCurrencyGive (currency) }

        let weiUnit = wkUnitCreateAsBase (currency, "wei", "wei", "wei")
        defer { wkUnitGive (weiUnit) }

        let gweiUnit = wkUnitCreate (currency, "gwei", "gwei", "gwei", weiUnit, 9)
        defer { wkUnitGive (gweiUnit) }

        let etherUnit = wkUnitCreate (currency, "ether", "ether", "ether", weiUnit, 18)
        defer { wkUnitGive (etherUnit) }

        let factor = wkAmountCreateDouble (2.0, gweiUnit)
        defer { wkAmountGive (factor) }

        let fee = wkNetworkFeeCreate (1_000, factor, gweiUnit)
        defer { wkNetworkFeeGive (fee) }

        wkNetworkSetHeight (network, blockHeight)

        wkNetworkAddCurrency (network, currency, weiUnit, etherUnit)

        wkNetworkAddCurrencyUnit(network, currency, weiUnit)
        wkNetworkAddCurrencyUnit(network, currency, gweiUnit)
        wkNetworkAddCurrencyUnit(network, currency, etherUnit)

        wkNetworkAddNetworkFee(network, fee)

        return wkNetworkTake (network)
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

        // Avalanche
//        ("testAvalanche",          testAvalanche),
        ("testAvalanchePrototype",          testAvalanchePrototype),

        // Ripple
        ("testRipple",          testRipple),

        // Hedera
        ("testHedera",          testHedera),
        
        // Tezos
        ("testTezos",           testTezos),

        // Stellar
        ("testStellar",         testStellar),

        // Bitcoin
        ("testSupportBTC",      testBitcoinSupport),
        ("testBTC",             testBitcoin),
        ("testSyncOneBTC",      testBitcoinSyncOne),
//        ("testManaagerSyncBTC", testBitcoinWalletManagerSync)
    ]
}
