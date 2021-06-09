/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.systemclient.brd;

import com.blockset.walletkit.SystemClient;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.collect.ImmutableMap;
import com.google.common.primitives.UnsignedInteger;

import java.util.Locale;
import java.util.Map;

import static com.google.common.base.Preconditions.checkNotNull;

public class CurrencyDenomination implements SystemClient.CurrencyDenomination {

    // creators

    public static CurrencyDenomination create(String name,
                                              String code,
                                              UnsignedInteger decimals,
                                              String symbol) {
        return new CurrencyDenomination(
                checkNotNull(name),
                checkNotNull(code),
                checkNotNull(decimals),
                checkNotNull(symbol)
        );
    }

    @JsonCreator
    public static CurrencyDenomination create(@JsonProperty("name") String name,
                                              @JsonProperty("short_name") String code,
                                              @JsonProperty("decimals") UnsignedInteger decimals) {
        return create(
                name,
                code,
                decimals,
                lookupSymbol(code)
        );
    }

    // fields

    private final String name;
    private final String code;
    private final UnsignedInteger decimals;
    private final String symbol;

    private CurrencyDenomination(String name,
                                 String code,
                                 UnsignedInteger decimals,
                                 String symbol) {
        this.name = name;
        this.code = code;
        this.decimals = decimals;
        this.symbol = symbol;
    }

    // getters

    @JsonProperty("name")
    public String getName() {
        return name;
    }

    @JsonProperty("short_name")
    public String getCode() {
        return code;
    }

    @JsonProperty("decimals")
    public UnsignedInteger getDecimals() {
        return decimals;
    }

    @JsonIgnore
    public String getSymbol() {
        return symbol == null ? lookupSymbol(code) : symbol;
    }

    // internals

    private static final Map<String, String> CURRENCY_SYMBOLS = ImmutableMap.of(
            "btc", "₿",
            "eth", "Ξ"
    );

    private static String lookupSymbol(String code) {
        String symbol = CURRENCY_SYMBOLS.get(code);
        return symbol != null ? symbol : code.toUpperCase(Locale.ROOT);
    }
}
