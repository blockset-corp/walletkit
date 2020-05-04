package com.breadwallet.core

sealed class TransferState {

    object CREATED : TransferState()
    object SIGNED : TransferState()
    object SUBMITTED : TransferState()
    object PENDING : TransferState()
    object DELETED : TransferState()

    data class INCLUDED(
            public val confirmation: TransferConfirmation
    ) : TransferState()

    data class FAILED(
            public val error: TransferSubmitError
    ) : TransferState()

    override fun toString() = when (this) {
        CREATED -> "Created"
        SIGNED -> "Signed"
        SUBMITTED -> "Submitted"
        PENDING -> "Pending"
        is INCLUDED -> "Included"
        is FAILED -> "Failed"
        DELETED -> "Deleted"
    }
}
