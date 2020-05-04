package com.breadwallet.core

import com.breadwallet.core.common.Coder
import com.breadwallet.core.common.CoderAlgorithm
import kotlin.test.Test
import kotlin.test.assertEquals
import kotlin.test.assertNotNull
import kotlin.test.assertNull

class CoderTest {
    @Test
    fun testCoder() {
        var d: ByteArray?
        val a: String
        val r: String?
        var s: String?
        // HEX
        d = byteArrayOf(0xde.toByte(), 0xad.toByte(), 0xbe.toByte(), 0xef.toByte())
        a = "deadbeef"
        r = Coder.createForAlgorithm(CoderAlgorithm.HEX).encode(d)
        assertEquals(a, r)
        assertContentEquals(assertNotNull(d), Coder.createForAlgorithm(CoderAlgorithm.HEX).decode(assertNotNull(r)))
        // BASE58
        s = "#&$@*^(*#!^"
        assertNull(Coder.createForAlgorithm(CoderAlgorithm.BASE58).decode(s))
        s = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
        d = Coder.createForAlgorithm(CoderAlgorithm.BASE58).decode(s)
        assertEquals(assertNotNull(s), Coder.createForAlgorithm(CoderAlgorithm.BASE58).encode(assertNotNull(d)))
        s = "z"
        d = Coder.createForAlgorithm(CoderAlgorithm.BASE58).decode(s)
        assertEquals(s, Coder.createForAlgorithm(CoderAlgorithm.BASE58).encode(assertNotNull(d)))
        //  BASE58CHECK
        d = byteArrayOf(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)
        s = Coder.createForAlgorithm(CoderAlgorithm.BASE58CHECK).encode(d)
        assertContentEquals(d, Coder.createForAlgorithm(CoderAlgorithm.BASE58CHECK).decode(assertNotNull(s)))
        d = byteArrayOf(
                0x05.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(),
                0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(),
                0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte(), 0xFF.toByte())
        s = Coder.createForAlgorithm(CoderAlgorithm.BASE58CHECK).encode(d)
        assertContentEquals(d, Coder.createForAlgorithm(CoderAlgorithm.BASE58CHECK).decode(assertNotNull(s)))
    }
}
