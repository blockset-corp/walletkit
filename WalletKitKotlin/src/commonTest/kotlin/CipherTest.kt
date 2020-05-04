package com.breadwallet.core

import com.breadwallet.core.common.Cipher
import com.breadwallet.core.common.Key
import io.ktor.utils.io.core.toByteArray
import kotlin.test.Test
import kotlin.test.assertNotNull

class CipherTest {

    @Test
    fun testCipher() {

        // aes-ecb
        val k: ByteArray = byteArrayOf(
                0x2b.toByte(), 0x7e.toByte(), 0x15.toByte(), 0x16.toByte(), 0x28.toByte(),
                0xae.toByte(), 0xd2.toByte(), 0xa6.toByte(), 0xab.toByte(), 0xf7.toByte(),
                0x15.toByte(), 0x88.toByte(), 0x09.toByte(), 0xcf.toByte(), 0x4f.toByte(),
                0x3c.toByte())
        val d: ByteArray = byteArrayOf(
                0x6b.toByte(), 0xc1.toByte(), 0xbe.toByte(), 0xe2.toByte(), 0x2e.toByte(),
                0x40.toByte(), 0x9f.toByte(), 0x96.toByte(), 0xe9.toByte(), 0x3d.toByte(),
                0x7e.toByte(), 0x11.toByte(), 0x73.toByte(), 0x93.toByte(), 0x17.toByte(),
                0x2a.toByte(), 0xae.toByte(), 0x2d.toByte(), 0x8a.toByte(), 0x57.toByte(),
                0x1e.toByte(), 0x03.toByte(), 0xac.toByte(), 0x9c.toByte(), 0x9e.toByte(),
                0xb7.toByte(), 0x6f.toByte(), 0xac.toByte(), 0x45.toByte(), 0xaf.toByte(),
                0x8e.toByte(), 0x51.toByte(), 0x30.toByte(), 0xc8.toByte(), 0x1c.toByte(),
                0x46.toByte(), 0xa3.toByte(), 0x5c.toByte(), 0xe4.toByte(), 0x11.toByte(),
                0xe5.toByte(), 0xfb.toByte(), 0xc1.toByte(), 0x19.toByte(), 0x1a.toByte(),
                0x0a.toByte(), 0x52.toByte(), 0xef.toByte(), 0xf6.toByte(), 0x9f.toByte(),
                0x24.toByte(), 0x45.toByte(), 0xdf.toByte(), 0x4f.toByte(), 0x9b.toByte(),
                0x17.toByte(), 0xad.toByte(), 0x2b.toByte(), 0x41.toByte(), 0x7b.toByte(),
                0xe6.toByte(), 0x6c.toByte(), 0x37.toByte(), 0x10.toByte())
        val a: ByteArray = assertNotNull(Cipher.createForAesEcb(k).encrypt(d))
        assertContentEquals(d, Cipher.createForAesEcb(k).decrypt(a))

        // cha-cha
        val nonce12 = byteArrayOf(
                0x07.toByte(), 0x00.toByte(), 0x00.toByte(), 0x00.toByte(), 0x40.toByte(),
                0x41.toByte(), 0x42.toByte(), 0x43.toByte(), 0x44.toByte(), 0x45.toByte(),
                0x46.toByte(), 0x47.toByte())
        val ad = byteArrayOf(
                0x50.toByte(), 0x51.toByte(), 0x52.toByte(), 0x53.toByte(), 0xc0.toByte(),
                0xc1.toByte(), 0xc2.toByte(), 0xc3.toByte(), 0xc4.toByte(), 0xc5.toByte(),
                0xc6.toByte(), 0xc7.toByte())
        val msg: ByteArray = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.".toByteArray()
        val key: Key = Key(createSecret(byteArrayOf(
                0x80.toByte(), 0x81.toByte(), 0x82.toByte(), 0x83.toByte(), 0x84.toByte(),
                0x85.toByte(), 0x86.toByte(), 0x87.toByte(), 0x88.toByte(), 0x89.toByte(),
                0x8a.toByte(), 0x8b.toByte(), 0x8c.toByte(), 0x8d.toByte(), 0x8e.toByte(),
                0x8f.toByte(), 0x90.toByte(), 0x91.toByte(), 0x92.toByte(), 0x93.toByte(),
                0x94.toByte(), 0x95.toByte(), 0x96.toByte(), 0x97.toByte(), 0x98.toByte(),
                0x99.toByte(), 0x9a.toByte(), 0x9b.toByte(), 0x9c.toByte(), 0x9d.toByte(),
                0x9e.toByte(), 0x9f.toByte()
        )))
        val alg = Cipher.createForChaCha20Poly1305(key, nonce12, ad)
        val cipher = byteArrayOf(
                0xd3.toByte(), 0x1a.toByte(), 0x8d.toByte(), 0x34.toByte(), 0x64.toByte(),
                0x8e.toByte(), 0x60.toByte(), 0xdb.toByte(), 0x7b.toByte(), 0x86.toByte(),
                0xaf.toByte(), 0xbc.toByte(), 0x53.toByte(), 0xef.toByte(), 0x7e.toByte(),
                0xc2.toByte(), 0xa4.toByte(), 0xad.toByte(), 0xed.toByte(), 0x51.toByte(),
                0x29.toByte(), 0x6e.toByte(), 0x08.toByte(), 0xfe.toByte(), 0xa9.toByte(),
                0xe2.toByte(), 0xb5.toByte(), 0xa7.toByte(), 0x36.toByte(), 0xee.toByte(),
                0x62.toByte(), 0xd6.toByte(), 0x3d.toByte(), 0xbe.toByte(), 0xa4.toByte(),
                0x5e.toByte(), 0x8c.toByte(), 0xa9.toByte(), 0x67.toByte(), 0x12.toByte(),
                0x82.toByte(), 0xfa.toByte(), 0xfb.toByte(), 0x69.toByte(), 0xda.toByte(),
                0x92.toByte(), 0x72.toByte(), 0x8b.toByte(), 0x1a.toByte(), 0x71.toByte(),
                0xde.toByte(), 0x0a.toByte(), 0x9e.toByte(), 0x06.toByte(), 0x0b.toByte(),
                0x29.toByte(), 0x05.toByte(), 0xd6.toByte(), 0xa5.toByte(), 0xb6.toByte(),
                0x7e.toByte(), 0xcd.toByte(), 0x3b.toByte(), 0x36.toByte(), 0x92.toByte(),
                0xdd.toByte(), 0xbd.toByte(), 0x7f.toByte(), 0x2d.toByte(), 0x77.toByte(),
                0x8b.toByte(), 0x8c.toByte(), 0x98.toByte(), 0x03.toByte(), 0xae.toByte(),
                0xe3.toByte(), 0x28.toByte(), 0x09.toByte(), 0x1b.toByte(), 0x58.toByte(),
                0xfa.toByte(), 0xb3.toByte(), 0x24.toByte(), 0xe4.toByte(), 0xfa.toByte(),
                0xd6.toByte(), 0x75.toByte(), 0x94.toByte(), 0x55.toByte(), 0x85.toByte(),
                0x80.toByte(), 0x8b.toByte(), 0x48.toByte(), 0x31.toByte(), 0xd7.toByte(),
                0xbc.toByte(), 0x3f.toByte(), 0xf4.toByte(), 0xde.toByte(), 0xf0.toByte(),
                0x8e.toByte(), 0x4b.toByte(), 0x7a.toByte(), 0x9d.toByte(), 0xe5.toByte(),
                0x76.toByte(), 0xd2.toByte(), 0x65.toByte(), 0x86.toByte(), 0xce.toByte(),
                0xc6.toByte(), 0x4b.toByte(), 0x61.toByte(), 0x16.toByte(), 0x1a.toByte(),
                0xe1.toByte(), 0x0b.toByte(), 0x59.toByte(), 0x4f.toByte(), 0x09.toByte(),
                0xe2.toByte(), 0x6a.toByte(), 0x7e.toByte(), 0x90.toByte(), 0x2e.toByte(),
                0xcb.toByte(), 0xd0.toByte(), 0x60.toByte(), 0x06.toByte(), 0x91.toByte()
        )
        assertContentEquals(cipher, alg.encrypt(msg))
        assertContentEquals(msg, alg.decrypt(cipher))

        // pigeon
        val pubKey: Key = assertNotNull(Key.createFromPublicKey("02d404943960a71535a79679f1cf1df80e70597c05b05722839b38ebc8803af517"))
        val pigeon = Cipher.createForPigeon(key, pubKey, nonce12)
        val pigeonCipher: ByteArray = assertNotNull(pigeon.encrypt(msg))
        assertContentEquals(msg, pigeon.decrypt(pigeonCipher))
    }
}
