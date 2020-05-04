package com.breadwallet.core

/**
 * A Transfer represents the transfer of an `amount` of currency from `source` to `target`.
 *
 * A Transfer is held in a `Wallet` (holding the amount's currency); the Transfer requires a `fee`
 * to complete.  Once the transfer is signed/submitted it can be identified by a `TransferHash`.
 * Once the transfer has been included in the currency's blockchain it will have a
 * `TransferConfirmation`.
 *
 * A Transfer is Equatable but not Hashable; Hashable would naturally be implemented in terms of
 * the TransferHash however that hash isn't available until after a transfer is signed.
 */
expect class Transfer {

    /** The owning wallet */
    public val wallet: Wallet

    /** The source pays the fee and sends the amount. */
    public val source: Address?

    /** The target receives the amount */
    public val target: Address?

    /** The amount to transfer - always positive (from source to target) */
    public val amount: Amount

    /**
     * The amount to transfer after considering the direction.
     *
     * If we received the transfer, the amount will be positive; if we sent the transfer, the
     * amount will be negative; if the transfer is 'self directed', the amount will be zero.
     */
    public val amountDirected: Amount

    /** The fee paid - before the transfer is confirmed, this is the estimated fee. */
    public val fee: Amount

    /**
     * The basis for the estimated fee.
     *
     * This is only not-nil if we have created the transfer IN THIS MEMORY INSTANCE
     * (assume this for now).
     */
    public val estimatedFeeBasis: TransferFeeBasis?

    /**
     * The basis for the confirmed fee.
     */
    public val confirmedFeeBasis: TransferFeeBasis?

    /** The direction */
    public val direction: TransferDirection

    /** An optional hash */
    public val hash: TransferHash?

    /** The unit for display of the transfer amount. */
    public val unit: CUnit

    /** The unit for display of the transfer fee. */
    public val unitForFee: CUnit

    /** An optional confirmation. */
    public val confirmation: TransferConfirmation?

    /**
     * Get the number of confirmations of transfer at the current network height.
     *
     * Since this value is calculated based on the associated network's height, it is recommended that a developer
     * refreshes any cached result in response to [WalletManagerBlockUpdatedEvent] events on the owning
     * WalletManager, in addition to further [TransferChangedEvent] events on this Transfer.
     *
     * If the transfer has not been confirmed or if the network's height is less than the confirmation height,
     * `absent` is returned.
     *
     * The minimum returned value is 1; if the height is the same as the confirmation block, then the transfer has
     * been confirmed once.
     */
    public val confirmations: ULong?

    /** The current state. */
    public val state: TransferState

    /**
     * Get the number of confirmations of transfer at a provided `blockHeight`.
     *
     * If the transfer has not been confirmed or if the `blockHeight` is less than the confirmation height,
     * `absent` is returned.
     *
     * The minimum returned value is 1; if `blockHeight` is the same as the confirmation block, then the
     * transfer has been confirmed once.
     */
    public fun getConfirmationsAt(blockHeight: ULong): ULong?

    override fun equals(other: Any?): Boolean

    override fun hashCode(): Int
}
