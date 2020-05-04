package com.breadwallet.core

import kotlinx.cinterop.alloc
import kotlinx.cinterop.nativeHeap
import kotlinx.cinterop.toCValues

actual typealias Secret = brcrypto.BRCryptoSecret

actual fun createSecret(data: ByteArray): Secret =
        nativeHeap.alloc {
            data.asUByteArray().toCValues().place(this.data)
        }

