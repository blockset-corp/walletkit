package com.breadwallet.core


data class TransferConfirmation(
        public val blockNumber: ULong,
        public val transactionIndex: ULong,
        public val timestamp: ULong,
        public val fee: Amount?
)
