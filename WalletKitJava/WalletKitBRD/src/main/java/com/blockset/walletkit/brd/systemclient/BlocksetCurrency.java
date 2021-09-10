/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import androidx.annotation.Nullable;

import com.blockset.walletkit.SystemClient.Currency;
import com.blockset.walletkit.SystemClient.CurrencyDenomination;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;

import java.util.ArrayList;
import java.util.List;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetCurrency implements Currency {

    private static final String ADDRESS_INTERNAL = "__native__";

    // creators

    public static BlocksetCurrency create(String id,
                                          String name,
                                          String code,
                                          String type,
                                          String blockchainId,
                                          @Nullable String address,
                                          Boolean verified,
                                          List<BlocksetCurrencyDenomination> denominations) {
        // TODO(fix): What should the supply values be here?
        return create(
                id,
                name,
                code,
                "0",
                "0",
                type,
                blockchainId,
                address == null ? ADDRESS_INTERNAL : address,
                verified,
                denominations
        );
    }

    @JsonCreator
    public static BlocksetCurrency create(@JsonProperty("currency_id") String id,
                                          @JsonProperty("name") String name,
                                          @JsonProperty("code") String code,
                                          @JsonProperty("initial_supply") String initialSupply,
                                          @JsonProperty("total_supply") String totalSupply,
                                          @JsonProperty("type") String type,
                                          @JsonProperty("blockchain_id") String blockchainId,
                                          @JsonProperty("address") String address,
                                          @JsonProperty("verified") Boolean verified,
                                          @JsonProperty("denominations") List<BlocksetCurrencyDenomination> denominations) {
        return new BlocksetCurrency(
                checkNotNull(id),
                checkNotNull(name),
                checkNotNull(code),
                checkNotNull(initialSupply),
                checkNotNull(totalSupply),
                checkNotNull(type),
                checkNotNull(blockchainId),
                checkNotNull(address),
                checkNotNull(verified),
                checkNotNull(denominations)
        );
    }

    // fields

    private final String id;
    private final String name;
    private final String code;
    private final String initialSupply;
    private final String totalSupply;
    private final String blockchainId;
    private final String address;
    private final String type;
    private final List<BlocksetCurrencyDenomination> denominations;
    private final Boolean verified;

    private BlocksetCurrency(String currencyId,
                             String name,
                             String code,
                             String initialSupply,
                             String totalSupply,
                             String type,
                             String blockchainId,
                             String address,
                             Boolean verified,
                             List<BlocksetCurrencyDenomination> denominations) {
        this.id = currencyId;
        this.name = name;
        this.code = code;
        this.initialSupply = initialSupply;
        this.totalSupply = totalSupply;
        this.type = type;
        this.blockchainId = blockchainId;
        this.address = address;
        this.verified = verified;
        this.denominations = denominations;
    }

    // getters

    @Override
    @JsonProperty("currency_id")
    public String getId() {
        return id;
    }

    @Override
    @JsonProperty("name")
    public String getName() {
        return name;
    }

    @Override
    @JsonProperty("code")
    public String getCode() {
        return code;
    }

    @Override
    @JsonProperty("type")
    public String getType() {
        return type;
    }

    @Override
    @JsonProperty("blockchain_id")
    public String getBlockchainId() {
        return blockchainId;
    }

    @Override
    @JsonIgnore
    public Optional<String> getAddress() {
        return ADDRESS_INTERNAL.equals(address) ? Optional.absent() : Optional.fromNullable(address);
    }

    @Override
    @JsonProperty("verified")
    public Boolean getVerified() {
        return verified;
    }

    @Override
    @JsonIgnore
    public List<CurrencyDenomination> getDenominations() {
        return new ArrayList<CurrencyDenomination>(getBlocksetDenominations());
    }



    @JsonProperty("initial_supply")
    public String getInitialSupply() {
        return initialSupply;
    }

    @JsonProperty("total_supply")
    public String getTotalSupply() {
        return totalSupply;
    }

    @JsonProperty("denominations")
    private List<BlocksetCurrencyDenomination> getBlocksetDenominations() {
        return denominations;
    }

    @JsonProperty("address")
    public String getAddressValue() {
        return address;
    }


}
