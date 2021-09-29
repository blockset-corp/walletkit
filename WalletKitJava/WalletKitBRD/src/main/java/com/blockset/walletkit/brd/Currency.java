/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import androidx.annotation.Nullable;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKCurrency;
import com.google.common.base.Optional;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;

import java.util.Objects;

/* package */
final class Currency implements com.blockset.walletkit.Currency {

    /* package */
    static Currency create (String uids, String name, String code, String type, @Nullable String issuer) {
        WKCurrency core = WKCurrency.create(uids, name, code, type, issuer);
        return Currency.create(core);
    }

    /* package */
    static Currency create(WKCurrency core) {
        Currency currency = new Currency(core);
        ReferenceCleaner.register(currency, core::give);
        return currency;
    }

    /* package */
    static Currency from(com.blockset.walletkit.Currency currency) {
        if (currency == null) {
            return null;
        }

        if (currency instanceof Currency) {
            return (Currency) currency;
        }

        throw new IllegalArgumentException("Unsupported currency instance");
    }

    private final WKCurrency core;

    private final Supplier<String> uidsSupplier;
    private final Supplier<String> nameSupplier;
    private final Supplier<String> codeSupplier;
    private final Supplier<String> typeSupplier;

    private final Supplier<String> issuerSupplier;

    private Currency(WKCurrency core) {
        this.core = core;

        this.uidsSupplier = Suppliers.memoize(core::getUids);
        this.nameSupplier = Suppliers.memoize(core::getName);
        this.codeSupplier = Suppliers.memoize(core::getCode);
        this.typeSupplier = Suppliers.memoize(core::getType);
        this.issuerSupplier = Suppliers.memoize(core::getIssuer);
    }

    @Override
    public String getUids() {
        return uidsSupplier.get();
    }

    @Override
    public String getName() {
        return nameSupplier.get();
    }

    @Override
    public String getCode() {
        return codeSupplier.get();
    }

    @Override
    public String getType() {
        return typeSupplier.get();
    }

    @Override
    public Optional<String> getIssuer() {
        return Optional.fromNullable(issuerSupplier.get());
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }

        if (o == null || getClass() != o.getClass()) {
            return false;
        }

        Currency currency = (Currency) o;
        return core.isIdentical(currency.core);
    }

    @Override
    public int hashCode() {
        return Objects.hash(getUids());
    }

    /* package */
    WKCurrency getCoreBRCryptoCurrency() {
        return core;
    }
}
