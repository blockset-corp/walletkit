package com.breadwallet.core.model

import kotlinx.serialization.SerialName
import kotlinx.serialization.Serializable

@Serializable
data class BdbSubscriptions(
        @SerialName("_embedded")
        val embedded: Embedded
) {

    @Serializable
    data class Embedded(
            val subscriptions: List<BdbSubscription>
    )
}

@Serializable
data class BdbSubscription(
        @SerialName("subscription_id")
        val subscriptionId: String,
        @SerialName("device_id")
        val deviceId: String,
        val endpoint: String,
        val currencies: List<BdbSubscriptionCurrency>
) {

    @Serializable
    data class BdbNewSubscription(
            @SerialName("device_id")
            val deviceId: String,
            val endpoint: BdbSubscriptionEndpoint,
            val currencies: List<BdbSubscriptionCurrency>
    )

    @Serializable
    data class BdbSubscriptionCurrency(
            @SerialName("currency_id")
            val currencyId: String,
            val addresses: List<String>,
            val events: List<BdbSubscriptionEvent>
    )

    @Serializable
    data class BdbSubscriptionEvent(
            val name: String,
            val confirmations: Int // TODO: UInt
    )

    @Serializable
    data class BdbSubscriptionEndpoint(
            val kind: String,
            val environment: String,
            val value: String
    )
}
