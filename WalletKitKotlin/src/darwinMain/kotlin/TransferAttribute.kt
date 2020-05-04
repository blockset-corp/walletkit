package com.breadwallet.core

import brcrypto.*
import kotlinx.cinterop.toKStringFromUtf8

actual class TransferAttribute(
        internal val core: BRCryptoTransferAttribute
) {

    actual val key: String
        get() = checkNotNull(cryptoTransferAttributeGetKey(core)).toKStringFromUtf8()

    actual val isRequired: Boolean
        get() = cryptoTransferAttributeIsRequired(core) == CRYPTO_TRUE

    actual var value: String?
        get() = cryptoTransferAttributeGetValue(core)?.toKStringFromUtf8()
        set(value) {
            cryptoTransferAttributeSetValue(core, value)
        }
}