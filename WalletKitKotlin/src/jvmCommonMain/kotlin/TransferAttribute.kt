package com.breadwallet.core

import com.breadwallet.corenative.crypto.BRCryptoTransferAttribute

actual class TransferAttribute(
        internal val core: BRCryptoTransferAttribute
) {

    actual val key: String
        get() = core.key

    actual val isRequired: Boolean
        get() = core.isRequired

    actual var value: String?
        get() = core.value.orNull()
        set(value) {
            core.setValue(value)
        }
}