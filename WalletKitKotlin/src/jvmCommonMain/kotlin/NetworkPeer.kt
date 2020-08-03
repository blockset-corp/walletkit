package com.breadwallet.core

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoPeer
import com.google.common.primitives.UnsignedInteger
import kotlinx.io.core.Closeable

actual class NetworkPeer internal constructor(
        internal val core: BRCryptoPeer
) : Closeable {

    init {
        ReferenceCleaner.register(core, ::close)
    }

    internal actual constructor(
            network: Network,
            address: String,
            port: UShort,
            publicKey: String?
    ) : this(
            checkNotNull(
                    BRCryptoPeer.create(
                            network.core,
                            address,
                            UnsignedInteger.valueOf(port.toLong()),
                            publicKey
                    ).orNull()
            )
    )

    actual val network: Network = Network(core.network)
    actual val address: String = core.address
    actual val port: UShort = core.port.toShort().toUShort()
    actual val publicKey: String? = core.publicKey.orNull()

    actual override fun hashCode(): Int = core.hashCode()
    actual override fun equals(other: Any?): Boolean =
            other is NetworkPeer && core.isIdentical(other.core)

    actual override fun close() {
        core.give()
    }
}
