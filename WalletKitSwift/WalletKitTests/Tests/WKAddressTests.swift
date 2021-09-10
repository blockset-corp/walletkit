//
//  WKAddressTests.swift
//  WalletKitTests
//
//  Created by Ed Gamble on 9/9/21.
//  Copyright © 2019 Breadwinner AG. All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.
//

import XCTest
@testable import WalletKit

class WKAddressTests: XCTestCase {

    override func setUp() {}

    override func tearDown() {}

    func testAddressETH () {
        let network = Network.findBuiltin(uids: "ethereum-mainnet")!

        let e1 = Address.create (string: "0xb0F225defEc7625C6B5E43126bdDE398bD90eF62", network: network)
        let e2 = Address.create (string: "0xd3CFBA03Fc13dc01F0C67B88CBEbE776D8F3DE8f", network: network)

        XCTAssertNotNil (e1)
        XCTAssertNotNil (e2)

        XCTAssertEqual("0xb0F225defEc7625C6B5E43126bdDE398bD90eF62", e1?.description)
        XCTAssertEqual("0xd3CFBA03Fc13dc01F0C67B88CBEbE776D8F3DE8f", e2?.description)

        let e3 = Address.create (string: "0xb0F225defEc7625C6B5E43126bdDE398bD90eF62", network: network)

        XCTAssertEqual (e1, e1)
        XCTAssertEqual (e1, e3)
        XCTAssertEqual (e3, e1)

        XCTAssertNotEqual (e1, e2)
        XCTAssertNotEqual (e2, e1)

        let _ = Address (core: e3!.core, take: true)
        let _ = Address (core: e3!.core)

        XCTAssertNil(Address.create(string: "ethereum:0xb0F225defEc7625C6B5E43126bdDE398bD90eF62", network: network));
        XCTAssertNil(Address.create(string: "1xb0F225defEc7625C6B5E43126bdDE398bD90eF62", network: network));
        XCTAssertNil(Address.create(string: "0xWWW225defEc7625C6B5E43126bdDE398bD90eF62", network: network));
        XCTAssertNil(Address.create(string: "", network: network));
        XCTAssertNil(Address.create(string: "0xb0F225defEc7625C6B5E43126bdDE398bD90eF6",  network: network));
        XCTAssertNil(Address.create(string: "0xb0F225defEc7625C6B5E43126bdDE398bD90eF",   network: network));
    }

    func testAddressBTC () {
        let network = Network.findBuiltin(uids: "bitcoin-mainnet")!

        XCTAssertEqual(Address.create (string: "1CC3X2gu58d6wXUWMffpuzN9JAfTUWu4Kj", network: network)?.description,
                       "1CC3X2gu58d6wXUWMffpuzN9JAfTUWu4Kj")

        XCTAssertNil (Address.create (string: "qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v", network: network))
        XCTAssertNil (Address.create (string: "bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v", network: network))
    }

    func testAddressBCH () {
        let network = Network.findBuiltin(uids: "bitcoincash-mainnet")!

        XCTAssertEqual (Address.create (string: "bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v", network: network)?.description,
                        "bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v")

        XCTAssertEqual (Address.create (string: "qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v", network: network)?.description,
                        "bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v")

        XCTAssertNil (Address.create (string: "bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq", network: network))
        XCTAssertNil (Address.create (string: "1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2", network: network))

        // TODO: Testnet BCH Addresses work as Mainnet BCH Addresses
        //        XCTAssertNil (network.addressFor("bchtest:pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t"))
        //        XCTAssertNil (network.addressFor("pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t"))
    }

    func testAddressBCHTestnet () {
        let network = Network.findBuiltin(uids: "bitcoincash-testnet")!

        XCTAssertEqual (Address.create (string: "bchtest:pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t", network: network)?.description,
                        "bchtest:pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t")

        XCTAssertEqual (Address.create (string: "pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t", network: network)?.description,
                        "bchtest:pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t")

        XCTAssertNil (Address.create (string: "bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq", network: network))

        // TODO: Testnet BCH Addresses work as Mainnet BCH Addresses
        //        XCTAssertNil (network.addressFor("1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2"))
        //        XCTAssertNil (network.addressFor("qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v"))
        //        XCTAssertNil (network.addressFor("bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v"))
    }

    func testAddressBSV () {
        let network = Network.findBuiltin(uids: "bitcoinsv-mainnet")!

        XCTAssertEqual(Address.create (string: "18qbsWhP6tU9c3CTujpuYywegG2UpxgqdU", network: network)?.description,
                       "18qbsWhP6tU9c3CTujpuYywegG2UpxgqdU")

        XCTAssertNil (Address.create (string: "bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq", network: network))
        XCTAssertNil (Address.create (string: "bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v", network: network))
    }

    func testAddressXRP () {
        let network = Network.findBuiltin (uids: "ripple-mainnet")!

        XCTAssertEqual (Address.create (string: "r41vZ8exoVyUfVzs56yeN8xB5gDhSkho9a", network: network)?.description,
                        "r41vZ8exoVyUfVzs56yeN8xB5gDhSkho9a")

        XCTAssertNil (Address.create (string: "w41vZ8exoVyUfVzs56yeN8xB5gDhSkho9a", network: network))

        let a1 = Address.create (string: "r41vZ8exoVyUfVzs56yeN8xB5gDhSkho9a", network: network);
        let a2 = Address.create (string: "rKKR566YfiTJFVKqJfTX8rHsot9pESQJbb", network: network);
        let a3 = Address.create (string: "rKKR566YfiTJFVKqJfTX8rHsot9pESQJbb", network: network);

        XCTAssertNotNil (a1); XCTAssertNotNil (a2); XCTAssertNotNil (a3);
        XCTAssertTrue  (a1 == a1)
        XCTAssertFalse (a1 == a2)
        XCTAssertTrue  (a2 == a3)
    }

    func testAddressHBAR () {
        let network = Network.findBuiltin (uids: "hedera-mainnet")!

        XCTAssertEqual (Address.create (string: "0.0.14222", network: network)?.description,
                        "0.0.14222")

        XCTAssertNil (Address.create (string: "0.0.x14222", network: network))

        let a1 = Address.create(string: "0.0.14222", network: network);
        let a2 = Address.create(string: "0.0.14223", network: network);
        let a3 = Address.create(string: "0.0.14223", network: network);

        XCTAssertNotNil (a1); XCTAssertNotNil (a2); XCTAssertNotNil (a3);
        XCTAssertTrue  (a1 == a1)
        XCTAssertFalse (a1 == a2)
        XCTAssertTrue  (a2 == a3)
    }

    func testAddressXTZ () {
        let network = Network.findBuiltin (uids: "tezos-mainnet")!

        XCTAssertEqual (Address.create (string: "tz1i5JJDhq7x8gVkpWq2Fwef3k7NEcBj2nJS", network: network)?.description,
                        "tz1i5JJDhq7x8gVkpWq2Fwef3k7NEcBj2nJS")

        XCTAssertNil (Address.create (string: "xz1i5JJDhq7x8gVkpWq2Fwef3k7NEcBj2nJS", network: network))

        let a1 = Address.create(string: "tz1i5JJDhq7x8gVkpWq2Fwef3k7NEcBj2nJS", network: network);
        let a2 = Address.create(string: "tz2TSvNTh2epDMhZHrw73nV9piBX7kLZ9K9m", network: network);
        let a3 = Address.create(string: "tz2TSvNTh2epDMhZHrw73nV9piBX7kLZ9K9m", network: network);

        XCTAssertNotNil (a1); XCTAssertNotNil (a2); XCTAssertNotNil (a3);
        XCTAssertTrue  (a1 == a1)
        XCTAssertFalse (a1 == a2)
        XCTAssertTrue  (a2 == a3)
    }

    func testAddressAVAX () {
        if let network = Network.findBuiltin(uids: "avalanche-mainnet") {

            let AddressParsablePairs:[(String, Bool)] = [ // (address, parsable)
                ("abc", true),
                ("def", false)
            ]

            for (string, parsable) in AddressParsablePairs {
                let address = Address.create(string: string, network: network)
                XCTAssert(parsable ? nil != address : nil == address)
                XCTAssert(!parsable || string == address!.description)
            }
        }

        if let network = Network.findBuiltin(uids: "avalanche-testnet") {

            let AddressParsablePairs:[(String, Bool)] = [ // (address, parsable)
            ]

            for (string, parsable) in AddressParsablePairs {
                let address = Address.create(string: string, network: network)
                XCTAssert(parsable ? nil != address : nil == address)
                XCTAssert(!parsable || string == address!.description)
            }
        }
    }
    /*
     let addr1 = bch.addressFor("bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v") // cashaddr with prefix is valid
     let addr2 = bch.addressFor("qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v") // cashaddr without prefix is valid
     addr1.description == "qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v” // prefix is not included
     addr2.description == "qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v”
     bch.addressFor("bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq") == nil  // bech32 not valid for bch
     bch.addressFor("1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2") == nil  // p2pkh not valid for bch
     btc.addressFor(“qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v”) == nil // cashaddr not valid for btc
     */

    func testAddressScheme () {
        XCTAssertEqual (AddressScheme.btcLegacy,  AddressScheme(core: AddressScheme.btcLegacy.core))
        XCTAssertEqual (AddressScheme.btcSegwit,  AddressScheme(core: AddressScheme.btcSegwit.core))
        XCTAssertEqual (AddressScheme.native, AddressScheme(core: AddressScheme.native.core))
        XCTAssertEqual (AddressScheme.native, AddressScheme(core: AddressScheme.native.core))

        XCTAssertEqual("BTC Legacy",  AddressScheme.btcLegacy.description)
        XCTAssertEqual("BTC Segwit",  AddressScheme.btcSegwit.description)
        XCTAssertEqual("Native", AddressScheme.native.description)
        XCTAssertEqual("Native", AddressScheme.native.description)

    }

    static var allTests = [
        ("testAddressETH",        testAddressETH),
        ("testAddressBTC",        testAddressBTC),
        ("testAddressBCH",        testAddressBCH),
        ("testAddressBCHTestnet", testAddressBCHTestnet),
        ("testAddressBSV",        testAddressBSV),
        ("testAddressXRP",        testAddressXRP),
        ("testAddressHBAR",       testAddressHBAR),
        ("testAddressXTZ",        testAddressXTZ),
        ("testAddressAVAX",       testAddressAVAX),
        ("testAddressScheme",     testAddressScheme),
    ]
}

