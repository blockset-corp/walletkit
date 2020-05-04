package com.breadwallet.core

sealed class WalletEvent {

    object Created : WalletEvent()
    object Deleted : WalletEvent()

    data class FeeBasisUpdated(
            val feeBasis: TransferFeeBasis
    ) : WalletEvent()

    data class FeeBasisEstimated(
            val feeBasis: TransferFeeBasis
    ) : WalletEvent()

    data class Change(
            val oldState: WalletState,
            val newState: WalletState
    ) : WalletEvent()

    data class BalanceUpdated(
            val balance: Amount
    ) : WalletEvent()

    data class TransferAdded(
            val transfer: Transfer
    ) : WalletEvent()

    data class TransferChanged(
            val transfer: Transfer
    ) : WalletEvent()

    data class TransferDeleted(
            val transfer: Transfer
    ) : WalletEvent()

    data class TransferSubmitted(
            val transfer: Transfer
    ) : WalletEvent()
}
