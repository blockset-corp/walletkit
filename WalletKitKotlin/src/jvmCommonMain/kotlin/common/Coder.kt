package com.breadwallet.core.common

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoCoder
import kotlinx.io.core.Closeable

actual class Coder internal constructor(
        internal val core: BRCryptoCoder
) : Closeable {

    init {
        ReferenceCleaner.register(core, ::close)
    }

    actual fun encode(source: ByteArray): String? =
            core.encode(source).orNull()

    actual fun decode(source: String): ByteArray? =
            core.decode(source).orNull()

    actual override fun close() {
        core.give()
    }

    actual companion object {
        actual fun createForAlgorithm(algorithm: CoderAlgorithm): Coder =
                when (algorithm) {
                    CoderAlgorithm.HEX -> BRCryptoCoder.createHex().orNull()
                    CoderAlgorithm.BASE58 -> BRCryptoCoder.createBase58().orNull()
                    CoderAlgorithm.BASE58CHECK -> BRCryptoCoder.createBase58Check().orNull()
                }.let { coreCoder -> Coder(checkNotNull(coreCoder)) }
    }
}
