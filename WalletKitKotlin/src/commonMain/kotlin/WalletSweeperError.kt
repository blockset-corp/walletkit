package com.breadwallet.core

sealed class WalletSweeperError : Exception() {

    class InsufficientFunds : WalletSweeperError()
    class InvalidKey : WalletSweeperError()
    class InvalidSourceWallet : WalletSweeperError()
    class NoTransfersFound : WalletSweeperError()
    class QueryError : WalletSweeperError()
    class UnableToSweep : WalletSweeperError()
    class UnsupportedCurrency : WalletSweeperError()
    class Unexpected(override val message: String) : WalletSweeperError()
}
