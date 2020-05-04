package com.breadwallet.core

sealed class LimitEstimationError : Exception() {

    class InsufficientFunds : LimitEstimationError()
    class ServiceFailure : LimitEstimationError()
    class ServiceUnavailable : LimitEstimationError()

    fun from(error: FeeEstimationError): LimitEstimationError {
        return error.toLimitEstimationError()
    }
}
