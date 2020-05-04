package com.breadwallet.core

/**
 * A TransferFeeBasis summarizes the fee required to complete a transfer.
 *
 * This is used both  when created a transfer (the 'estimated fee basis')
 * and once a transfer is confirmed (the 'confirmed fee basis').
 *
 * The provided properties allow the App to present detailed information
 * - specifically the 'cost factor' and the 'price per cost factor'.
 */
expect class TransferFeeBasis {

    /** The unit for both the pricePerCostFactor and fee. */
    public val unit: CUnit

    /** The fee basis currency; this should/must be the Network's currency */
    public val currency: Currency

    /** The pricePerCostFactor as an amount in currency */
    public val pricePerCostFactor: Amount

    /** The costFactor which is an arbitrary scale on the pricePerCostFactor */
    public val costFactor: Double

    /** The fee, computed as `pricePerCostFactor * costFactor` */
    public val fee: Amount

    override fun equals(other: Any?): Boolean

    override fun hashCode(): Int
}
