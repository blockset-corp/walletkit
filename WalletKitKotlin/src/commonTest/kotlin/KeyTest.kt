package com.breadwallet.core

import com.breadwallet.core.common.Key
import io.ktor.utils.io.core.toByteArray
import kotlin.test.*

class KeyTest {
    @Test
    fun testKey() {
        var s: String?
        val t: ByteArray
        val n: ByteArray
        var k: Key?
        var l: Key?
        var p: Boolean

        // Password Protected
        // mainnet (https://bitcoinpaperwallet.com/bitcoinpaperwallet/generate-wallet.html?design=alt-testnet)
        s = "6PRPGFR3vou5h9VXHVTUpnDisZpnKik5c7zWJrw8abi4AYW8fy4uPFYFXk"
        p = Key.isProtectedPrivateKey(s)
        assertTrue(p)
        k = Key.createFromProtectedPrivateKey(s, "hodl")
        assertNotNull(k)
        assertEquals("5KYkuC2SX6twF8C4yhDRJsy9tgBnn9aFsEXgaMLwRciopuRnBfT", k.encodeAsPrivate.decodeToString())
        // testnet (https://bitcoinpaperwallet.com/bitcoinpaperwallet/generate-wallet.html?design=alt-testnet)
        s = "6PRVDGjn5m1Tj6wbP8kG6YozjE5qBHxE9BPfF8HxZHLdd1tAkENLjjsQve"
        p = Key.isProtectedPrivateKey(s)
        assertTrue(p)
        k = Key.createFromProtectedPrivateKey(s, "hodl")
        assertNotNull(k)
        assertEquals("92HBCaQbnqkGkSm8z2rxKA3DRdSrsniEswiWFe5CQXdfPqThR9N", k.encodeAsPrivate.decodeToString())
        s = "5Kb8kLf9zgWQnogidDA76MzPL6TsZZY36hWXMssSzNydYXYB9KF"
        p = Key.isProtectedPrivateKey(s)
        assertFalse(p)
        k = Key.createFromProtectedPrivateKey(s, "hodl")
        assertNull(k)
        s = "KyvGbxRUoofdw3TNydWn2Z78dBHSy2odn1d3wXWN2o3SAtccFNJL"
        p = Key.isProtectedPrivateKey(s)
        assertFalse(p)
        k = Key.createFromProtectedPrivateKey(s, "hodl")
        assertNull(k)

        // Uncompressed
        s = "5Kb8kLf9zgWQnogidDA76MzPL6TsZZY36hWXMssSzNydYXYB9KF"
        k = Key.createFromPrivateKey(s)
        assertNotNull(k)
        assertTrue(k.hasSecret)
        assertEquals(s, k.encodeAsPrivate.decodeToString())
        // Compressed
        s = "KyvGbxRUoofdw3TNydWn2Z78dBHSy2odn1d3wXWN2o3SAtccFNJL"
        k = Key.createFromPrivateKey(s)
        assertNotNull(k)
        assertTrue(k.hasSecret)
        assertEquals(s, k.encodeAsPrivate.decodeToString())
        l = Key.createFromPrivateKey(k.encodeAsPrivate.decodeToString())
        assertNotNull(l)
        assertTrue(k.privateKeyMatch(l))
        t = k.encodeAsPublic
        l = Key.createFromPublicKey(t.decodeToString())
        assertNotNull(l)
        assertFalse(l.hasSecret)
        assertContentEquals(t, l.encodeAsPublic)
        assertTrue(l.publicKeyMatch(k))

        // Bad Key
        s = "XyvGbxRUoofdw3TNydWn2Z78dBHSy2odn1d3wXWN2o3SAtccFNJL"
        k = Key.createFromPrivateKey(s)
        assertNull(k)

        // Phrase
        s = "ginger settle marine tissue robot crane night number ramp coast roast critic"
        k = Key.createFromPhrase(s, WORDS)
        assertNotNull(k)
        s = "ginger settle marine tissue robot crane night number ramp coast roast critic"
        k = Key.createFromPhrase(s, null)
        assertNull(k)
        s = "not-a-chance"
        k = Key.createFromPhrase(s, WORDS)
        assertNull(k)
        // Pigeon
        s = "5Kb8kLf9zgWQnogidDA76MzPL6TsZZY36hWXMssSzNydYXYB9KF"
        k = Key.createFromPrivateKey(s)
        n = "nonce".toByteArray()
        l = Key.createForPigeonFromKey(assertNotNull(k), n)
        assertNotNull(l)
        // BIP32ApiAuth
        s = "ginger settle marine tissue robot crane night number ramp coast roast critic"
        k = Key.createForBIP32ApiAuth(s, WORDS)
        assertNotNull(k)
        s = "ginger settle marine tissue robot crane night number ramp coast roast critic"
        k = Key.createForBIP32ApiAuth(s, null)
        assertNull(k)
        s = "not-a-chance"
        k = Key.createForBIP32ApiAuth(s, WORDS)
        assertNull(k)
        // BIP32BitID
        s = "ginger settle marine tissue robot crane night number ramp coast roast critic"
        k = Key.createForBIP32BitID(s, 2, "some uri", WORDS)
        assertNotNull(k)
        s = "ginger settle marine tissue robot crane night number ramp coast roast critic"
        k = Key.createForBIP32BitID(s, 2, "some uri", null)
        assertNull(k)
        s = "not-a-chance"
        k = Key.createForBIP32BitID(s, 2, "some uri", WORDS)
        assertNull(k)
    }
}
