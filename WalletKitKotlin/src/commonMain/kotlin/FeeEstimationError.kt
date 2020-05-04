package com.breadwallet.core

sealed class FeeEstimationError : Exception() {

    class InsufficientFunds : FeeEstimationError()
    class ServiceFailure : FeeEstimationError()
    class ServiceUnavailable : FeeEstimationError()

    fun toLimitEstimationError(): LimitEstimationError =
            when (this) {
                is InsufficientFunds -> LimitEstimationError.InsufficientFunds()
                is ServiceFailure -> LimitEstimationError.ServiceFailure()
                is ServiceUnavailable -> LimitEstimationError.ServiceUnavailable()
            }
}
