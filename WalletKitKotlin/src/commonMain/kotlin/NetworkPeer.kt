package com.breadwallet.core

import kotlinx.io.core.Closeable

/**
 * A NetworkPeer is a Peer on a Network.
 *
 * This is optionally used in Peer-to-Peer modes to specify one or more peers
 * to connect to for network synchronization.  Normally the P2P protocol
 * dynamically discovers peers and thus NetworkPeer is not commonly used.
 */
expect class NetworkPeer : Closeable {

    internal constructor(
            network: Network,
            address: String,
            port: UShort,
            publicKey: String?
    )

    /** The network */
    public val network: Network

    /** The address */
    public val address: String

    /** The port */
    public val port: UShort

    /** The public key */
    public val publicKey: String?

    override fun equals(other: Any?): Boolean
    override fun hashCode(): Int
    override fun close()
}
