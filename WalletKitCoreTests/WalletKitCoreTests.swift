import XCTest
@testable import WalletKitCore
@testable import WalletKitSupportTests

final class WalletKitCoreTests: XCTestCase {
    func testBase () {
        runBaseTests()
    }

    func testUtil () {
        runUtilTests()
    }

    func testRLP () {
        runRlpTests()
    }

    func testEvent () {
        runEventTests ()
    }

    func testBC () {
        runBcTests()
    }

    func testBitcoin () {
         BRRunTests()
     }

    static var allTests = [
        ("testBase", testBase),
    ]
}
