package com.breadwallet.core

expect class Secret

expect fun createSecret(data: ByteArray): Secret
