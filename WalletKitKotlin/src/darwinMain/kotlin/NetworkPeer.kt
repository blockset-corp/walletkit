package com.breadwallet.core

import brcrypto.*
import kotlinx.cinterop.toKStringFromUtf8
import kotlinx.io.core.Closeable

actual class NetworkPeer(
        core: BRCryptoPeer,
        take: Boolean = false
) : Closeable {

    internal val core: BRCryptoPeer =
            if (take) checkNotNull(cryptoPeerTake(core))
            else core

    internal actual constructor(
            network: Network,
            address: String,
            port: UShort,
            publicKey: String?
    ) : this(
            checkNotNull(cryptoPeerCreate(
                    network.core,
                    address,
                    port,
                    publicKey
            ))
    )

    actual val network: Network =
            Network(checkNotNull(cryptoPeerGetNetwork(core)), false)

    actual val address: String =
            checkNotNull(cryptoPeerGetAddress(core)).toKStringFromUtf8()

    actual val port: UShort = cryptoPeerGetPort(core)

    actual val publicKey: String? =
            cryptoPeerGetPublicKey(core)?.toKStringFromUtf8()

    actual override fun hashCode(): Int = core.hashCode()
    actual override fun equals(other: Any?): Boolean =
            other is NetworkPeer && CRYPTO_TRUE == cryptoPeerIsIdentical(core, other.core)

    actual override fun close() {
        cryptoPeerGive(core)
    }
}
