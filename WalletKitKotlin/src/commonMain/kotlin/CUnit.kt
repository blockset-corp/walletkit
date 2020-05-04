package com.breadwallet.core

import kotlinx.io.core.Closeable

/**
 * A unit of measure for a currency.
 *
 * There can be multiple units for a given currency (analogous to 'System International'
 * units of (meters, kilometers, miles, ...) for a dimension of 'length').  For example,
 * Ethereum has units of: WEI, GWEI, ETHER, METHER, ... and Bitcoin of: BTC, SATOSHI, ...
 *
 * Each Currency has a 'baseUnit' - which is defined as the 'integer-ish' unit - such as SATOSHI
 * ane WEI for Bitcoin and Ethereum, respectively.  There can be multiple 'derivedUnits' - which
 * are derived by scaling off of a baseUnit.  For example, BTC and ETHER respectively.
 */
expect class CUnit : Closeable {

    public val currency: Currency

    internal val uids: String

    public val name: String
    public val symbol: String
    public val base: CUnit
    public val decimals: UInt // TODO: Maybe use UByte here

    public fun isCompatible(unit: CUnit): Boolean
    public fun hasCurrency(currency: Currency): Boolean

    override fun equals(other: Any?): Boolean
    override fun hashCode(): Int

    companion object {
        internal fun create(
                currency: Currency,
                uids: String,
                name: String,
                symbol: String
        ): CUnit

        internal fun create(
                currency: Currency,
                uids: String,
                name: String,
                symbol: String,
                base: CUnit,
                decimals: UInt
        ): CUnit
    }
}
