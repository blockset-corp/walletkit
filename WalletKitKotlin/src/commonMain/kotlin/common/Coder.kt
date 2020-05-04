package com.breadwallet.core.common

import kotlinx.io.core.Closeable

expect class Coder : Closeable {

    public fun encode(source: ByteArray): String?
    public fun decode(source: String): ByteArray?

    override fun close()

    companion object {
        public fun createForAlgorithm(algorithm: CoderAlgorithm): Coder
    }
}

enum class CoderAlgorithm {
    HEX, BASE58, BASE58CHECK
}
