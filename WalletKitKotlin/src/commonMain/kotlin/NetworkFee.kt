package com.breadwallet.core

import kotlinx.io.core.Closeable

/**
 * A Network Fee represents the 'amount per cost factor' paid to mine a transfer.
 *
 * For BTC this amount is 'SAT/BYTE'; for ETH this amount is 'gasPrice'.  The actual fee for the
 * transfer depends on properties of the transfer; for BTC, the cost factor is 'size in kB';
 * for ETH, the cost factor is 'gas'.
 *
 * A Network supports a variety of fees.  Essentially the higher the fee the more enticing the
 * transfer is to a miner and thus the more quickly the transfer gets into the block chain.
 *
 * A NetworkFee is Equatable on the underlying Core representation.  It is natural to compare
 * NetworkFee based on timeIntervalInMilliseconds.
 */
expect class NetworkFee : Closeable {

    /**
     * Initialize based on the timeInternal and pricePerCostFactor.  Used by
     * BlockchainDB when parsing a NetworkFee from BlockchainDB.Model.BlockchainFee
     */
    internal constructor(
            timeIntervalInMilliseconds: ULong,
            pricePerCostFactor: Amount
    )

    /** The estimated time internal for a transaction confirmation. */
    public val timeIntervalInMilliseconds: ULong

    /**
     * The amount, as a rate on 'cost factor', to pay in network fees for the desired
     * time internal to confirmation.
     *
     * The 'cost factor' is blockchain specific - for BTC it is 'transaction size in kB';
     * for ETH it is 'gas'.
     */
    internal val pricePerCostFactor: Amount

    override fun equals(other: Any?): Boolean
    override fun hashCode(): Int
    override fun close()
}
