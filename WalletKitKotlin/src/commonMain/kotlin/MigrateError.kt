package com.breadwallet.core

sealed class MigrateError : Exception() {

    object Block : MigrateError()
    object Create : MigrateError()
    object Invalid : MigrateError()
    object Peer : MigrateError()
    object Transaction : MigrateError()
}
