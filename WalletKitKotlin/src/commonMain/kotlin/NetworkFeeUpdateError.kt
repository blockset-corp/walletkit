package com.breadwallet.core

sealed class NetworkFeeUpdateError : Exception() {

    object FeesUnavailable : NetworkFeeUpdateError()
}
