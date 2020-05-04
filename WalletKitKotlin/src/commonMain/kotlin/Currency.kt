package com.breadwallet.core

import kotlinx.io.core.Closeable

/** A currency is a medium for exchange. */
expect class Currency : Closeable {

    /** 'A Unique Identifier */
    public val uids: String

    /** The code; e.g. BTC */
    public val code: String

    /** The name; e.g. Bitcoin */
    public val name: String

    /** The type: */
    public val type: String

    /** The issuer, if present.  This is generally an ERC20 address. */
    public val issuer: String?

    override fun equals(other: Any?): Boolean
    override fun hashCode(): Int

    companion object {
        fun create(
                uids: String,
                name: String,
                code: String,
                type: String,
                issuer: String?
        ): Currency
    }
}

/** Used to map Currency -> Built-In-Blockchain-Network */
const val CURRENCY_CODE_AS_BTC = "btc"

/** Used to map Currency -> Built-In-Blockchain-Network */
const val CURRENCY_CODE_AS_BCH = "bch"

/** Used to map Currency -> Built-In-Blockchain-Network */
const val CURRENCY_CODE_AS_ETH = "eth"
