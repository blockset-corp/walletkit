package com.breadwallet.core

import com.breadwallet.core.common.HashAlgorithm
import com.breadwallet.core.common.Hasher
import io.ktor.utils.io.core.toByteArray
import kotlin.test.Test


class HasherTest {
    @Test
    fun testHasher() {
        var d: ByteArray?
        var a: ByteArray
        // SHA1
        d = "Free online SHA1 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0x6f.toByte(), 0xc2.toByte(), 0xe2.toByte(), 0x51.toByte(), 0x72.toByte(),
                0xcb.toByte(), 0x15.toByte(), 0x19.toByte(), 0x3c.toByte(), 0xb1.toByte(),
                0xc6.toByte(), 0xd4.toByte(), 0x8f.toByte(), 0x60.toByte(), 0x7d.toByte(),
                0x42.toByte(), 0xc1.toByte(), 0xd2.toByte(), 0xa2.toByte(), 0x15.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.SHA1).hash(d))
        // SHA256
        d = "Free online SHA256 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0x43.toByte(), 0xfd.toByte(), 0x9d.toByte(), 0xeb.toByte(), 0x93.toByte(),
                0xf6.toByte(), 0xe1.toByte(), 0x4d.toByte(), 0x41.toByte(), 0x82.toByte(),
                0x66.toByte(), 0x04.toByte(), 0x51.toByte(), 0x4e.toByte(), 0x3d.toByte(),
                0x78.toByte(), 0x73.toByte(), 0xa5.toByte(), 0x49.toByte(), 0xac.toByte(),
                0x87.toByte(), 0xae.toByte(), 0xbe.toByte(), 0xbf.toByte(), 0x3d.toByte(),
                0x1c.toByte(), 0x10.toByte(), 0xad.toByte(), 0x6e.toByte(), 0xb0.toByte(),
                0x57.toByte(), 0xd0.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.SHA256).hash(d))
        // SHA256_2
        d = "Free online SHA256_2 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0xe3.toByte(), 0x9a.toByte(), 0xee.toByte(), 0xfe.toByte(), 0xd2.toByte(),
                0x39.toByte(), 0x02.toByte(), 0xd7.toByte(), 0x81.toByte(), 0xef.toByte(),
                0xdc.toByte(), 0x83.toByte(), 0x8a.toByte(), 0x0b.toByte(), 0x5d.toByte(),
                0x16.toByte(), 0xc0.toByte(), 0xab.toByte(), 0x90.toByte(), 0x1d.toByte(),
                0xc9.toByte(), 0x26.toByte(), 0xbb.toByte(), 0x6e.toByte(), 0x2c.toByte(),
                0x1e.toByte(), 0xdf.toByte(), 0xb9.toByte(), 0xc1.toByte(), 0x8d.toByte(),
                0x89.toByte(), 0xb8.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.SHA256_2).hash(d))
        // SHA224
        d = "Free online SHA224 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0x09.toByte(), 0xcd.toByte(), 0xa9.toByte(), 0x39.toByte(), 0xab.toByte(),
                0x1d.toByte(), 0x6e.toByte(), 0x7c.toByte(), 0x3f.toByte(), 0x81.toByte(),
                0x3b.toByte(), 0xa2.toByte(), 0x3a.toByte(), 0xf3.toByte(), 0x4b.toByte(),
                0xdf.toByte(), 0xe9.toByte(), 0x35.toByte(), 0x50.toByte(), 0x6d.toByte(),
                0xc4.toByte(), 0x92.toByte(), 0xeb.toByte(), 0x77.toByte(), 0xd8.toByte(),
                0x22.toByte(), 0x6a.toByte(), 0x64.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.SHA224).hash(d))
        // SHA384
        d = "Free online SHA384 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0xef.toByte(), 0x82.toByte(), 0x38.toByte(), 0x77.toByte(), 0xa4.toByte(),
                0x66.toByte(), 0x4c.toByte(), 0x96.toByte(), 0x41.toByte(), 0xc5.toByte(),
                0x3a.toByte(), 0xc2.toByte(), 0x05.toByte(), 0x59.toByte(), 0xc3.toByte(),
                0x4b.toByte(), 0x5d.toByte(), 0x2c.toByte(), 0x67.toByte(), 0x94.toByte(),
                0x77.toByte(), 0xde.toByte(), 0x22.toByte(), 0xff.toByte(), 0xfa.toByte(),
                0xb3.toByte(), 0x51.toByte(), 0xe5.toByte(), 0xe3.toByte(), 0x3e.toByte(),
                0xa5.toByte(), 0x3e.toByte(), 0x42.toByte(), 0x36.toByte(), 0x15.toByte(),
                0xe1.toByte(), 0xee.toByte(), 0x3c.toByte(), 0x85.toByte(), 0xe0.toByte(),
                0xd7.toByte(), 0xfa.toByte(), 0xcb.toByte(), 0x84.toByte(), 0xdf.toByte(),
                0x2b.toByte(), 0xa2.toByte(), 0x17.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.SHA384).hash(d))
        // SHA512
        d = "Free online SHA512 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0x04.toByte(), 0xf1.toByte(), 0x15.toByte(), 0x41.toByte(), 0x35.toByte(),
                0xee.toByte(), 0xcb.toByte(), 0xe4.toByte(), 0x2e.toByte(), 0x9a.toByte(),
                0xdc.toByte(), 0x8e.toByte(), 0x1d.toByte(), 0x53.toByte(), 0x2f.toByte(),
                0x9c.toByte(), 0x60.toByte(), 0x7a.toByte(), 0x84.toByte(), 0x47.toByte(),
                0xb7.toByte(), 0x86.toByte(), 0x37.toByte(), 0x7d.toByte(), 0xb8.toByte(),
                0x44.toByte(), 0x7d.toByte(), 0x11.toByte(), 0xa5.toByte(), 0xb2.toByte(),
                0x23.toByte(), 0x2c.toByte(), 0xdd.toByte(), 0x41.toByte(), 0x9b.toByte(),
                0x86.toByte(), 0x39.toByte(), 0x22.toByte(), 0x4f.toByte(), 0x78.toByte(),
                0x7a.toByte(), 0x51.toByte(), 0xd1.toByte(), 0x10.toByte(), 0xf7.toByte(),
                0x25.toByte(), 0x91.toByte(), 0xf9.toByte(), 0x64.toByte(), 0x51.toByte(),
                0xa1.toByte(), 0xbb.toByte(), 0x51.toByte(), 0x1c.toByte(), 0x4a.toByte(),
                0x82.toByte(), 0x9e.toByte(), 0xd0.toByte(), 0xa2.toByte(), 0xec.toByte(),
                0x89.toByte(), 0x13.toByte(), 0x21.toByte(), 0xf3.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.SHA512).hash(d))
        // SHA3
        d = "abc".toByteArray()
        a = byteArrayOf(
                0x3a.toByte(), 0x98.toByte(), 0x5d.toByte(), 0xa7.toByte(), 0x4f.toByte(),
                0xe2.toByte(), 0x25.toByte(), 0xb2.toByte(), 0x04.toByte(), 0x5c.toByte(),
                0x17.toByte(), 0x2d.toByte(), 0x6b.toByte(), 0xd3.toByte(), 0x90.toByte(),
                0xbd.toByte(), 0x85.toByte(), 0x5f.toByte(), 0x08.toByte(), 0x6e.toByte(),
                0x3e.toByte(), 0x9d.toByte(), 0x52.toByte(), 0x5b.toByte(), 0x46.toByte(),
                0xbf.toByte(), 0xe2.toByte(), 0x45.toByte(), 0x11.toByte(), 0x43.toByte(),
                0x15.toByte(), 0x32.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.SHA3).hash(d))
        // RMD160
        d = "Free online RIPEMD160 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0x95.toByte(), 0x01.toByte(), 0xa5.toByte(), 0x6f.toByte(), 0xb8.toByte(),
                0x29.toByte(), 0x13.toByte(), 0x2b.toByte(), 0x87.toByte(), 0x48.toByte(),
                0xf0.toByte(), 0xcc.toByte(), 0xc4.toByte(), 0x91.toByte(), 0xf0.toByte(),
                0xec.toByte(), 0xbc.toByte(), 0x7f.toByte(), 0x94.toByte(), 0x5b.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.RMD160).hash(d))
        // HASH160
        d = "Free online HASH160 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0x62.toByte(), 0x0a.toByte(), 0x75.toByte(), 0x2d.toByte(), 0x20.toByte(),
                0x09.toByte(), 0xd4.toByte(), 0xc6.toByte(), 0x59.toByte(), 0x8b.toByte(),
                0x7f.toByte(), 0x63.toByte(), 0x4d.toByte(), 0x34.toByte(), 0xc5.toByte(),
                0xec.toByte(), 0xd5.toByte(), 0x23.toByte(), 0x36.toByte(), 0x72.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.HASH160).hash(d))
        // KECCAK256
        d = "".toByteArray()
        a = byteArrayOf(
                0xc5.toByte(), 0xd2.toByte(), 0x46.toByte(), 0x01.toByte(), 0x86.toByte(),
                0xf7.toByte(), 0x23.toByte(), 0x3c.toByte(), 0x92.toByte(), 0x7e.toByte(),
                0x7d.toByte(), 0xb2.toByte(), 0xdc.toByte(), 0xc7.toByte(), 0x03.toByte(),
                0xc0.toByte(), 0xe5.toByte(), 0x00.toByte(), 0xb6.toByte(), 0x53.toByte(),
                0xca.toByte(), 0x82.toByte(), 0x27.toByte(), 0x3b.toByte(), 0x7b.toByte(),
                0xfa.toByte(), 0xd8.toByte(), 0x04.toByte(), 0x5d.toByte(), 0x85.toByte(),
                0xa4.toByte(), 0x70.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.KECCAK256).hash(d))
        // MD5
        d = "Free online MD5 Calculator, type text here...".toByteArray()
        a = byteArrayOf(
                0x0b.toByte(), 0x3b.toByte(), 0x20.toByte(), 0xea.toByte(), 0xf1.toByte(),
                0x69.toByte(), 0x64.toByte(), 0x62.toByte(), 0xf5.toByte(), 0x0d.toByte(),
                0x1a.toByte(), 0x3b.toByte(), 0xbd.toByte(), 0xd3.toByte(), 0x0c.toByte(),
                0xef.toByte()
        )
        assertContentEquals(a, Hasher.createForAlgorithm(HashAlgorithm.MD5).hash(d))
    }
}
