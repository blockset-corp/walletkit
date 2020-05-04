package com.breadwallet.core.common

import kotlinx.io.core.Closeable

expect class Hasher : Closeable {

    public fun hash(data: ByteArray): ByteArray?

    override fun close()

    companion object {
        public fun createForAlgorithm(algorithm: HashAlgorithm): Hasher
    }
}

enum class HashAlgorithm {
    SHA1,
    SHA224,
    SHA256,
    SHA256_2,
    SHA384,
    SHA512,
    SHA3,
    RMD160,
    HASH160,
    KECCAK256,
    MD5
}
