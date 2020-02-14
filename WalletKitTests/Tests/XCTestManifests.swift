import XCTest

#if !canImport(ObjectiveC)
public func allTests() -> [XCTestCaseEntry] {
    return [
        testCase(WalletKitExampleTests.allTests),
        testCase(BRBlockChainDBTest.allTests),
    ]
}
#endif
