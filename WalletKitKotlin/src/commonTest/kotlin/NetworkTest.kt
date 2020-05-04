package com.breadwallet.core

import kotlin.test.*


class NetworkTest {

    @Test
    fun testNetworkBTC() {
        val btc = Currency.create("bitcoin-mainnet:__native__", "Bitcoin", "btc", "native", null)

        val unitSat = CUnit.create(btc, "sat", "Satoshi", "SAT")
        val unitBtc = CUnit.create(btc, "btc", "Bitcoin", "B", unitSat, 8u)

        val network = checkNotNull(Network.findBuiltin("bitcoin-mainnet"))

        assertEquals("bitcoin-mainnet", network.uids)
        assertEquals("Bitcoin", network.name)
        assertTrue(network.isMainnet)

        val height = network.height
        network.height *= 2u
        assertEquals(height * 2u, network.height)


        assertEquals(btc, network.currency)
        assertTrue(network.hasCurrency(btc))
        assertEquals(btc, network.currencyByCode("btc"))
        assertNull(network.currencyByIssuer("foo"))

        assertEquals(unitSat, network.baseUnitFor(btc))
        assertEquals(unitBtc, network.defaultUnitFor(btc))

        assertEquals(setOf(unitSat, unitBtc), network.unitsFor(btc))
        assertTrue(network.hasUnitFor(btc, unitBtc) ?: false)
        assertTrue(network.hasUnitFor(btc, unitSat) ?: false)

        val eth = Currency.create("ethereum-mainnet:__native__", "Ethereum", "ETH", "native", null)
        val unitWei = CUnit.create(eth, "ETH-WEI", "WEI", "wei")

        assertFalse(network.hasCurrency(eth))
        assertNull(network.baseUnitFor(eth))
        assertNull(network.unitsFor(eth))

        assertFalse(network.hasUnitFor(eth, unitWei) ?: false)
        assertFalse(network.hasUnitFor(eth, unitBtc) ?: false)
        assertFalse(network.hasUnitFor(btc, unitWei) ?: false)

        assertEquals(1, network.fees.size)

        val networksTable = mapOf(network to 1)
        assertEquals(1, networksTable[network])
    }

    @Test
    fun testNetworkETH() {
        val eth = Currency.create("ethereum-mainnet:__native__", "Ethereum", "eth", "native", null)
        val unitWei = CUnit.create(eth, "wei", "WEI", "wei")
        val unitGwei = CUnit.create(eth, "gwei", "GWEI", "gwei", unitWei, 9u)
        val unitEther = CUnit.create(eth, "eth", "ETHER", "E", unitWei, 18u)

        val brd = Currency.create("ethereum-mainnet:0x558ec3152e2eb2174905cd19aea4e34a23de9ad6", "BRD Token", "brd", "erc20", "0x558ec3152e2eb2174905cd19aea4e34a23de9ad6")

        //val unitBrdi = CUnit.create(brd, "BRD_Integer", "BRD Integer", "BRDI")
        //val unitBrd = CUnit.create(brd, "BRD_Decimal", "BRD Decimal", "BRD", unitBrdi, 18u)

        val btc = Currency.create("bitcoin-mainnet:__native__", "Bitcoin", "btc", "native", null)

        val fee1 = NetworkFee(1u * 60u * 1000uL, Amount.create(2.0, unitGwei))

        val network = checkNotNull(Network.findBuiltin("ethereum-mainnet"))

        assertEquals("Ethereum", network.toString())
        assertTrue(network.hasCurrency(eth))
        assertTrue(network.hasCurrency(brd))
        assertFalse(network.hasCurrency(btc))

        assertNotNull(network.currencyByCode("eth"))
        assertNotNull(network.currencyByCode("brd"))

        assertNotNull(network.currencyByIssuer("0x558ec3152e2eb2174905cd19aea4e34a23de9ad6"))
        assertNotNull(network.currencyByIssuer("0x558ec3152e2eb2174905cd19aea4e34a23de9ad6".toUpperCase()))
        assertNull(network.currencyByIssuer("foo"))

        assertTrue(network.hasUnitFor(eth, unitWei) ?: false)
        assertTrue(network.hasUnitFor(eth, unitGwei) ?: false)
        assertTrue(network.hasUnitFor(eth, unitEther) ?: false)

        // TODO: Fails assertEquals(fee1, network.minimumFee)

        assertNull(network.defaultUnitFor(btc))
        assertNull(network.baseUnitFor(btc))
    }
}
