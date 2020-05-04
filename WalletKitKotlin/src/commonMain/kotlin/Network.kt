package com.breadwallet.core

import kotlinx.io.core.Closeable

/**
 * A Blockchain Network.
 *
 * Networks are created based from a cross-product of block chain and network type.
 * Specifically {BTC, BCH, ETH, ...} x {Mainnet, Testnet, ...}.  Thus there will be
 * networks of [BTC-Mainnet, BTC-Testnet, ..., ETH-Mainnet, ETH-Testnet, ETH-Rinkeby, ...]
 */
expect class Network : Closeable {

    /** A unique-identifier-string */
    internal val uids: String

    /** The name */
    public val name: String

    public val type: NetworkType

    /** If 'mainnet' then true, otherwise false */
    public val isMainnet: Boolean

    /**
     * The current height of the blockChain network.  On a reorganization, this might
     * go backwards.
     *
     * (No guarantee that this monotonically increases)
     */
    public var height: ULong
        internal set

    /**
     * The network fees.
     *
     * Expect the User to select their preferred fee, based on time-to-confirm,
     * and then have their preferred fee held in WalletManager.defaultNetworkFee.
     */
    public var fees: List<NetworkFee>
        internal set

    /**
     * Return the minimum fee which should be the fee with the largest confirmation time.
     */
    public val minimumFee: NetworkFee

    public val confirmationsUntilFinal: UInt

    public val defaultAddressScheme: AddressScheme

    public val supportedAddressSchemes: List<AddressScheme>

    public val supportedWalletManagerModes: List<WalletManagerMode>

    public val defaultWalletManagerMode: WalletManagerMode

    /**
     * Create a Network Peer for use in P2P modes when a WalletManager connects.
     *
     * @param address An numeric-dot-notation IP address
     * @param port A port number
     * @param publicKey An optional public key
     *
     * @return A NetworkPeer if the address correctly parses; otherwise null
     */
    public fun createPeer(address: String, port: UShort, publicKey: String?): NetworkPeer?

    /** The native currency. */
    public val currency: Currency

    /** All currencies - at least those we are handling/interested-in. */
    public val currencies: Set<Currency>

    public fun currencyByCode(code: String): Currency?

    public fun currencyByIssuer(issuer: String): Currency?

    public fun hasCurrency(currency: Currency): Boolean

    public fun baseUnitFor(currency: Currency): CUnit?

    public fun defaultUnitFor(currency: Currency): CUnit?

    public fun unitsFor(currency: Currency): Set<CUnit>?

    public fun hasUnitFor(currency: Currency, unit: CUnit): Boolean?

    public fun addressFor(string: String): Address?

    public fun addCurrency(currency: Currency, baseUnit: CUnit, defaultUnit: CUnit)

    public fun addUnitFor(currency: Currency, unit: CUnit)

    public fun supportsWalletManagerMode(mode: WalletManagerMode): Boolean

    public fun supportsAddressScheme(addressScheme: AddressScheme): Boolean

    public fun requiresMigration(): Boolean

    override fun equals(other: Any?): Boolean
    override fun hashCode(): Int
    override fun toString(): String
    override fun close()

    companion object {
        internal fun installBuiltins(): List<Network>
        fun findBuiltin(uids: String): Network?
    }
}

data class NetworkAssociation(
        val baseUnit: CUnit,
        val defaultUnit: CUnit,
        val units: Set<CUnit>
)
