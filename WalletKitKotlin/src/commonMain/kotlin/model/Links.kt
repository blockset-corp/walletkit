package com.breadwallet.core.model

import kotlinx.serialization.Serializable


@Serializable
data class Links(
        val next: Link? = null,
        val self: Link? = null
) {
    @Serializable
    data class Link(val href: String)
}