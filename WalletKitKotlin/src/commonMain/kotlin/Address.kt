package com.breadwallet.core

import kotlinx.io.core.Closeable

/**
 * An Address for transferring an amount.
 */
expect class Address : Closeable {

    override fun close()

    override fun equals(other: Any?): Boolean
    override fun hashCode(): Int
    override fun toString(): String

    companion object {
        /**
         * Create an Address from [string] and [network].
         *
         * The provided [string] must be valid for the provided [network] - that is, an ETH
         * address (as a string) differs from a BTC address and a BTC mainnet address differs
         * from a BTC testnet address.
         *
         * In practice, 'target' addresses (for receiving crypto) are generated from the wallet
         * and 'source' addresses (for sending crypto) are a User input.
         *
         * @param string A string representing a crypto address
         * @param network A string representing a crypto address
         *
         * @return An address or null if [string] is invalid for [network]
         */
        public fun create(string: String, network: Network): Address?
    }
}
