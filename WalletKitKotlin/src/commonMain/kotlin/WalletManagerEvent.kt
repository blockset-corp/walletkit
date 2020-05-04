package com.breadwallet.core

/**
 * A WalletManager Event represents a asynchronous announcement of a manager's state change.
 */
sealed class WalletManagerEvent {
    object Created : WalletManagerEvent()
    object Deleted : WalletManagerEvent()
    data class Changed(
            val oldState: WalletManagerState,
            val newState: WalletManagerState
    ) : WalletManagerEvent()

    class WalletAdded(val wallet: Wallet) : WalletManagerEvent()
    class WalletChanged(val wallet: Wallet) : WalletManagerEvent()
    class WalletDeleted(val wallet: Wallet) : WalletManagerEvent()

    object SyncStarted : WalletManagerEvent()
    data class SyncProgress(
            val timestamp: Long?,
            val percentComplete: Float
    ) : WalletManagerEvent()

    data class SyncStopped(
            val reason: SyncStoppedReason
    ) : WalletManagerEvent()

    data class SyncRecommended(
            val depth: WalletManagerSyncDepth
    ) : WalletManagerEvent()

    /**
     * An event capturing a change in the block height of the network associated with a
     * WalletManager. Developers should listen for this event when making use of
     * Transfer::confirmations, as that value is calculated based on the associated network's
     * block height. Displays or caches of that confirmation count should be updated when this
     * event occurs.
     */
    data class BlockUpdated(
            val height: ULong
    ) : WalletManagerEvent()
}
