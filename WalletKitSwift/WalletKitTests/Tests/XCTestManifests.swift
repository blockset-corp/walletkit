import XCTest

#if !canImport(ObjectiveC)
public func allTests() -> [XCTestCaseEntry] {
    return [
        testCase (WKBlocksetTests.allTests),
        testCase (WKAccountTests.allTests),
        testCase (WKAmountTests.allTests),
        testCase (WKCommonTests.allTests),
        testCase (WKNetworkTests.allTests),
        testCase (WKTransferTests.allTests),
        testCase (WKWalletTests.allTests),
        testCase (WKWalletManagerTests.allTests),
        testCase (WKSystemTests.allTests),
        testCase (BRSupportTests.allTests),
    ]
}
#endif
