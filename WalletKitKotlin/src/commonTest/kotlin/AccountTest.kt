package com.breadwallet.core

import io.ktor.utils.io.core.toByteArray
import kotlin.test.*

class AccountTest {

    val phrase = "ginger settle marine tissue robot crane night number ramp coast roast critic".toByteArray()
    val address = "0x8fB4CB96F7C15F9C39B3854595733F728E1963Bc"
    val timestamp = 1514764800L

    @Test
    fun testPhrase() {
        assertTrue(Account.validatePhrase(phrase, WORDS))

        val phrase = Account.generatePhrase(WORDS)
        assertNotNull(phrase)

        assertTrue(Account.validatePhrase(phrase, WORDS))
        assertFalse(Account.validatePhrase("Ask @jmo for a pithy quote".toByteArray(), WORDS))
    }

    @Test
    fun testAccount() {
        val walletId = "5766b9fa-e9aa-4b6d-9b77-b5f1136e5e96"
        val a1 = Account.createFromPhrase(phrase, timestamp, walletId)

        assertNotNull(a1)

        // TODO: assertEquals(address, a1.addressAsETH)
        assertEquals(timestamp, a1.timestamp)
        assertEquals(walletId, a1.uids)

        assertTrue(a1.validate(a1.serialize))

        val a2 = Account.createFromSerialization(a1.serialize, walletId)
        assertNotNull(a2)

        // TODO: assertEquals(a1.addressAsETH, a2.addressAsETH)
        assertEquals(walletId, a2.uids)

        val timestamp3 = 1576550071L
        val phrase3 = Account.generatePhrase(WORDS)
        assertNotNull(phrase3)

        val a3 = Account.createFromPhrase(phrase3, timestamp3, "ignore")
        assertNotNull(a3)

        assertFalse(a3.validate(a1.serialize))
    }

    @Test
    fun testAddressETH() {
        val network = checkNotNull(Network.findBuiltin("ethereum-mainnet"))

        val e1 = Address.create("0xb0F225defEc7625C6B5E43126bdDE398bD90eF62", network)
        val e2 = Address.create("0xd3CFBA03Fc13dc01F0C67B88CBEbE776D8F3DE8f", network)
        assertNotNull(e1)
        assertNotNull(e2)

        assertEquals("0xb0F225defEc7625C6B5E43126bdDE398bD90eF62", e1.toString())
        assertEquals("0xd3CFBA03Fc13dc01F0C67B88CBEbE776D8F3DE8f", e2.toString())

        val e3 = Address.create("0xb0F225defEc7625C6B5E43126bdDE398bD90eF62", network)

        assertEquals(e1, e1)
        assertEquals(e1, e3)

        assertNotEquals(e1, e2)
    }

    @Test
    fun testAddressBTC() {
        val network = checkNotNull(Network.findBuiltin("bitcoin-mainnet"))

        assertEquals(
                "1CC3X2gu58d6wXUWMffpuzN9JAfTUWu4Kj",
                network.addressFor("1CC3X2gu58d6wXUWMffpuzN9JAfTUWu4Kj").toString()
        )

        assertNull(network.addressFor("qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v"))
        assertNull(network.addressFor("bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v"))
    }

    @Test
    fun testAddressBCH() {
        val network = checkNotNull(Network.findBuiltin("bitcoincash-mainnet"))

        assertEquals(
                "bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v",
                network.addressFor("bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v").toString()
        )

        assertEquals(
                "bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v",
                network.addressFor("qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v")?.toString()
        )

        assertNull(network.addressFor("bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq"))
        assertNull(network.addressFor("1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2"))

        // TODO: Testnet BCH Addresses work as Mainnet BCH Addresses
        //        XCTAssertNil (network.addressFor("bchtest:pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t"))
        //        XCTAssertNil (network.addressFor("pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t"))
    }

    @Test
    fun testAddressBCHTestnet() {
        val network = checkNotNull(Network.findBuiltin("bitcoincash-testnet"))

        assertEquals(
                "bchtest:pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t",
                network.addressFor("bchtest:pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t").toString()
        )

        assertEquals(
                "bchtest:pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t",
                network.addressFor("pr6m7j9njldwwzlg9v7v53unlr4jkmx6eyvwc0uz5t").toString()
        )

        assertNull(network.addressFor("bc1qar0srrr7xfkvy5l643lydnw9re59gtzzwf5mdq"))
        // TODO: Testnet BCH Addresses work as Mainnet BCH Addresses
        //        XCTAssertNil (network.addressFor("1BvBMSEYstWetqTFn5Au4m4GFg7xJaNVN2"))
        //        XCTAssertNil (network.addressFor("qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v"))
        //        XCTAssertNil (network.addressFor("bitcoincash:qp0k6fs6q2hzmpyps3vtwmpx80j9w0r0acmp8l6e9v"))
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

    @Test
    fun testAddressScheme() {
        /* TODO: Platform specific tests
            XCTAssertEqual (AddressScheme.btcLegacy,  AddressScheme(core: AddressScheme.btcLegacy.core))
            XCTAssertEqual (AddressScheme.btcSegwit,  AddressScheme(core: AddressScheme.btcSegwit.core))
            XCTAssertEqual (AddressScheme.ethDefault, AddressScheme(core: AddressScheme.ethDefault.core))
            XCTAssertEqual (AddressScheme.genDefault, AddressScheme(core: AddressScheme.genDefault.core))
        */
        assertEquals("BTC Legacy", AddressScheme.BTCLegacy.toString())
        assertEquals("BTC Segwit", AddressScheme.BTCSegwit.toString())
        assertEquals("ETH Default", AddressScheme.ETHDefault.toString())
        assertEquals("GEN Default", AddressScheme.GENDefault.toString())
    }
}
