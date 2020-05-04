package com.breadwallet.core

import kotlin.test.assertTrue

fun assertContentEquals(a: ByteArray, b: ByteArray?) {
    assertTrue("Expected <${a.decodeToString()}> but was <${b?.decodeToString()}>") {
        a.contentEquals(b ?: return@assertTrue false)
    }
}
