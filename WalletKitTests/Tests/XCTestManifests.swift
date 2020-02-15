import XCTest

#if !canImport(ObjectiveC)
public func allTests() -> [XCTestCaseEntry] {
    return [
        testCase (BRBlockChainDBTest.allTests),
        testCase (BRCryptoAccountTests.allTests),
        testCase (BRCryptoAmountTests.allTests),
        testCase (BRCryptoCommonTests.allTests),
        testCase (BRCryptoNetworkTests.allTests),
        testCase (BRCryptoTransferTests.allTests),
        testCase (BRCryptoWalletTests.allTests),
        testCase (BRCryptoWalletManagerTests.allTests),
        testCase (BRCryptoSystemTests.allTests),
        testCase (BRSupportTests.allTests),
    ]
}
#endif
