package com.breadwallet.core.common

import kotlinx.io.core.Closeable

expect class Signer : Closeable {

    public fun sign(digest: ByteArray, key: Key): ByteArray?
    public fun recover(digest: ByteArray, signature: ByteArray): Key?

    override fun close()

    companion object {
        public fun createForAlgorithm(algorithm: SignerAlgorithm): Signer
    }
}

enum class SignerAlgorithm {
    BASIC_DER, BASIC_JOSE, COMPACT
}
