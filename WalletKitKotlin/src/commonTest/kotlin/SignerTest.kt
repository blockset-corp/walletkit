package com.breadwallet.core.common

import com.breadwallet.core.assertContentEquals
import com.breadwallet.core.createSecret
import io.ktor.utils.io.core.toByteArray
import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertNotNull
import kotlin.test.assertTrue

class SignerTest {
    @Test
    fun testSigner() {
        var msg: String
        var digest: ByteArray?
        var signature: ByteArray?
        var signer: Signer
        var answer: ByteArray
        val key: Key = Key(createSecret(
                byteArrayOf(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1)
        ))
        // Basic DER
        msg = "How wonderful that we have met with a paradox. Now we have some hope of making progress."
        digest = Hasher.createForAlgorithm(HashAlgorithm.SHA256).hash(msg.toByteArray())
        signer = Signer.createForAlgorithm(SignerAlgorithm.BASIC_DER)
        signature = signer.sign(assertNotNull(digest), key)
        answer = byteArrayOf(
                0x30.toByte(), 0x45.toByte(), 0x02.toByte(), 0x21.toByte(), 0x00.toByte(),
                0xc0.toByte(), 0xda.toByte(), 0xfe.toByte(), 0xc8.toByte(), 0x25.toByte(),
                0x1f.toByte(), 0x1d.toByte(), 0x50.toByte(), 0x10.toByte(), 0x28.toByte(),
                0x9d.toByte(), 0x21.toByte(), 0x02.toByte(), 0x32.toByte(), 0x22.toByte(),
                0x0b.toByte(), 0x03.toByte(), 0x20.toByte(), 0x2c.toByte(), 0xba.toByte(),
                0x34.toByte(), 0xec.toByte(), 0x11.toByte(), 0xfe.toByte(), 0xc5.toByte(),
                0x8b.toByte(), 0x3e.toByte(), 0x93.toByte(), 0xa8.toByte(), 0x5b.toByte(),
                0x91.toByte(), 0xd3.toByte(), 0x02.toByte(), 0x20.toByte(), 0x75.toByte(),
                0xaf.toByte(), 0xdc.toByte(), 0x06.toByte(), 0xb7.toByte(), 0xd6.toByte(),
                0x32.toByte(), 0x2a.toByte(), 0x59.toByte(), 0x09.toByte(), 0x55.toByte(),
                0xbf.toByte(), 0x26.toByte(), 0x4e.toByte(), 0x7a.toByte(), 0xaa.toByte(),
                0x15.toByte(), 0x58.toByte(), 0x47.toByte(), 0xf6.toByte(), 0x14.toByte(),
                0xd8.toByte(), 0x00.toByte(), 0x78.toByte(), 0xa9.toByte(), 0x02.toByte(),
                0x92.toByte(), 0xfe.toByte(), 0x20.toByte(), 0x50.toByte(), 0x64.toByte(),
                0xd3.toByte())
        assertContentEquals(answer, signature)
        // Basic JOSE
        msg = "How wonderful that we have met with a paradox. Now we have some hope of making progress."
        digest = Hasher.createForAlgorithm(HashAlgorithm.SHA256).hash(msg.toByteArray())
        signer = Signer.createForAlgorithm(SignerAlgorithm.BASIC_JOSE)
        signature = signer.sign(assertNotNull(digest), key)
        answer = byteArrayOf(
                0xc0.toByte(), 0xda.toByte(), 0xfe.toByte(), 0xc8.toByte(), 0x25.toByte(),
                0x1f.toByte(), 0x1d.toByte(), 0x50.toByte(), 0x10.toByte(), 0x28.toByte(),
                0x9d.toByte(), 0x21.toByte(), 0x02.toByte(), 0x32.toByte(), 0x22.toByte(),
                0x0b.toByte(), 0x03.toByte(), 0x20.toByte(), 0x2c.toByte(), 0xba.toByte(),
                0x34.toByte(), 0xec.toByte(), 0x11.toByte(), 0xfe.toByte(), 0xc5.toByte(),
                0x8b.toByte(), 0x3e.toByte(), 0x93.toByte(), 0xa8.toByte(), 0x5b.toByte(),
                0x91.toByte(), 0xd3.toByte(), 0x75.toByte(), 0xaf.toByte(), 0xdc.toByte(),
                0x06.toByte(), 0xb7.toByte(), 0xd6.toByte(), 0x32.toByte(), 0x2a.toByte(),
                0x59.toByte(), 0x09.toByte(), 0x55.toByte(), 0xbf.toByte(), 0x26.toByte(),
                0x4e.toByte(), 0x7a.toByte(), 0xaa.toByte(), 0x15.toByte(), 0x58.toByte(),
                0x47.toByte(), 0xf6.toByte(), 0x14.toByte(), 0xd8.toByte(), 0x00.toByte(),
                0x78.toByte(), 0xa9.toByte(), 0x02.toByte(), 0x92.toByte(), 0xfe.toByte(),
                0x20.toByte(), 0x50.toByte(), 0x64.toByte(), 0xd3.toByte())
        assertContentEquals(answer, signature)
        // Compact
        msg = "foo"
        digest = Hasher.createForAlgorithm(HashAlgorithm.SHA256).hash(msg.toByteArray())
        signer = Signer.createForAlgorithm(SignerAlgorithm.COMPACT)
        signature = signer.sign(assertNotNull(digest), key)
        val keyPublic: Key? = signer.recover(assertNotNull(digest), assertNotNull(signature))
        assertNotNull(keyPublic)
        assertTrue(key.publicKeyMatch(keyPublic))
    }

    @Test
    fun testCompactSigner() {
        val secrets = arrayOf<ByteArray>(
                "5HxWvvfubhXpYYpS3tJkw6fq9jE9j18THftkZjHHfmFiWtmAbrj".toByteArray(),
                "5KC4ejrDjv152FGwP386VD1i2NYc5KkfSMyv1nGy1VGDxGHqVY3".toByteArray(),
                "Kwr371tjA9u2rFSMZjTNun2PXXP3WPZu2afRHTcta6KxEUdm1vEw".toByteArray(),
                "L3Hq7a8FEQwJkW1M2GNKDW28546Vp5miewcCzSqUD9kCAXrJdS3g".toByteArray())
        val signatures = arrayOf(
                "1c5dbbddda71772d95ce91cd2d14b592cfbc1dd0aabd6a394b6c2d377bbe59d31d14ddda21494a4e221f0824f0b8b924c43fa43c0ad57dccdaa11f81a6bd4582f6",
                "1c52d8a32079c11e79db95af63bb9600c5b04f21a9ca33dc129c2bfa8ac9dc1cd561d8ae5e0f6c1a16bde3719c64c2fd70e404b6428ab9a69566962e8771b5944d",
                "205dbbddda71772d95ce91cd2d14b592cfbc1dd0aabd6a394b6c2d377bbe59d31d14ddda21494a4e221f0824f0b8b924c43fa43c0ad57dccdaa11f81a6bd4582f6",
                "2052d8a32079c11e79db95af63bb9600c5b04f21a9ca33dc129c2bfa8ac9dc1cd561d8ae5e0f6c1a16bde3719c64c2fd70e404b6428ab9a69566962e8771b5944d")
        val message: ByteArray = assertNotNull(Hasher.createForAlgorithm(HashAlgorithm.SHA256_2)
                .hash("Very deterministic message".toByteArray()))
        for (i in secrets.indices) {
            val maybeKey: Key? = Key.createFromPrivateKey(secrets[i].decodeToString())
            assertNotNull(maybeKey)
            val outputSig: ByteArray? = Signer.createForAlgorithm(SignerAlgorithm.COMPACT).sign(message, maybeKey)
            val outputSigHex: String? = Coder.createForAlgorithm(CoderAlgorithm.HEX).encode(assertNotNull(outputSig))
            assertEquals(assertNotNull(outputSigHex), assertNotNull(signatures[i]))
        }
    }
}
