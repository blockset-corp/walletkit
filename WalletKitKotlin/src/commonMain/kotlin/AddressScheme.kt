package com.breadwallet.core

/**
 * An AddressScheme generates addresses for a wallet.
 *
 * Depending on the scheme, a given wallet may  generate different address.  For example,
 * a Bitcoin wallet can have a 'Segwit/BECH32' address scheme or a 'Legacy' address scheme.
 *
 * The WalletManager holds an array of AddressSchemes as well as the preferred AddressScheme.
 * The preferred scheme is selected from among the array of schemes.
 */
enum class AddressScheme(
        internal val core: UInt,
        private val label: String
) {

    BTCLegacy(0u, "BTC Legacy"),
    BTCSegwit(1u, "BTC Segwit"),
    ETHDefault(2u, "ETH Default"),
    GENDefault(3u, "GEN Default");

    override fun toString() = label

    companion object {
        fun fromCoreInt(core: UInt): AddressScheme = when (core) {
            0u -> BTCLegacy
            1u -> BTCSegwit
            2u -> ETHDefault
            3u -> GENDefault
            else -> error("Unknown core AddressScheme value ($core)")
        }
    }
}
