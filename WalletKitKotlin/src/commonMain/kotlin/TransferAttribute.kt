package com.breadwallet.core


expect class TransferAttribute {

    val key: String
    val isRequired: Boolean
    var value: String?
}