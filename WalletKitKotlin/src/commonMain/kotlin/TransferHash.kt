package com.breadwallet.core

import kotlinx.io.core.Closeable

expect class TransferHash : Closeable {
    override fun equals(other: Any?): Boolean
    override fun hashCode(): Int
    override fun toString(): String
    override fun close()
}
