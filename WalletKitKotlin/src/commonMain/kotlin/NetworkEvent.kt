package com.breadwallet.core

sealed class NetworkEvent {
    object Created : NetworkEvent()
    object FeesUpdated : NetworkEvent()
}
