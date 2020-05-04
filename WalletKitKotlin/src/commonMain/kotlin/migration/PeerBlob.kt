package com.breadwallet.core.migration

// services value indicating a node carries full blocks, not just headers
const val SERVICES_NODE_NETWORK: ULong = 0x01u

// BIP111: https://github.com/bitcoin/bips/blob/master/bip-0111.mediawiki
const val SERVICES_NODE_BLOOM: ULong = 0x01u

// BIP144: https://github.com/bitcoin/bips/blob/master/bip-0144.mediawiki
const val SERVICES_NODE_WITNESS: ULong = 0x08u

// https://github.com/Bitcoin-UAHF/spec/blob/master/uahf-technical-spec.md
const val SERVICES_NODE_BCASH: ULong = 0x20u

class PeerBlob private constructor(
        val btc: Btc? = null
) {

    class Btc(
            val address: UInt,
            val port: UInt,
            val services: ULong,
            val timestamp: UInt
    )
}
