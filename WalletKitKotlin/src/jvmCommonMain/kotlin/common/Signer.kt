package com.breadwallet.core.common

import com.breadwallet.corenative.cleaner.ReferenceCleaner
import com.breadwallet.corenative.crypto.BRCryptoSigner
import kotlinx.io.core.Closeable

actual class Signer internal constructor(
        internal val core: BRCryptoSigner
) : Closeable {

    init {
        ReferenceCleaner.register(core, ::close)
    }

    actual fun sign(digest: ByteArray, key: Key): ByteArray? =
            core.sign(digest, key.core).orNull()

    actual fun recover(digest: ByteArray, signature: ByteArray): Key? =
            core.recover(digest, signature).orNull()?.run(::Key)

    actual override fun close() {
        core.give()
    }

    actual companion object {
        actual fun createForAlgorithm(algorithm: SignerAlgorithm): Signer =
                when (algorithm) {
                    SignerAlgorithm.COMPACT -> BRCryptoSigner.createCompact().orNull()
                    SignerAlgorithm.BASIC_DER -> BRCryptoSigner.createBasicDer().orNull()
                    SignerAlgorithm.BASIC_JOSE -> BRCryptoSigner.createBasicJose().orNull()
                }.let { coreSigner -> Signer(checkNotNull(coreSigner)) }
    }
}
