package com.breadwallet.core

sealed class TransferSubmitError : Exception() {

    object UNKNOWN : TransferSubmitError()

    data class POSIX(
            val errNum: Int,
            val errMessage: String?
    ) : TransferSubmitError() {
        override val message: String = "Posix ($errNum: $errMessage)"
    }
}
