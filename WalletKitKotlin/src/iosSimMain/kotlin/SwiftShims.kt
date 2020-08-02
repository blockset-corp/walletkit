package com.breadwallet.core

import io.ktor.client.HttpClient
import io.ktor.client.engine.ios.Ios
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.yield

object SwiftShims {
    fun createHttpClient(): HttpClient = HttpClient(Ios)
    fun stringToByteArray(string: String): ByteArray = string.encodeToByteArray()
}