/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKUnit;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;
import com.google.common.primitives.UnsignedInteger;

import java.util.Objects;

/* package */
final class Unit implements com.blockset.walletkit.Unit {

    /* package */
    static Unit create(Currency currency, String code, String name, String symbol) {
        WKUnit core = WKUnit.createAsBase(currency.getCoreBRCryptoCurrency(), code, name, symbol);
        return Unit.create(core);
    }

    /* package */
    static Unit create(Currency currency, String code, String name, String symbol, Unit base, UnsignedInteger decimals) {
        WKUnit core = WKUnit.create(currency.getCoreBRCryptoCurrency(), code, name, symbol, base.core, decimals);
        return Unit.create(core);
    }

    /* package */
    static Unit create(WKUnit core) {
        Unit unit = new Unit(core);
        ReferenceCleaner.register(unit, core::give);
        return unit;
    }

    /* package */
    static Unit from(com.blockset.walletkit.Unit unit) {
        if (unit == null) {
            return null;
        }

        if (unit instanceof Unit) {
            return (Unit) unit;
        }

        throw new IllegalArgumentException("Unsupported unit instance");
    }

    private final WKUnit core;

    private final Supplier<Currency> currencySupplier;
    private final Supplier<String> nameSupplier;
    private final Supplier<String> symbolSupplier;
    private final Supplier<String> uidsSupplier;
    private final Supplier<UnsignedInteger> decimalsSupplier;

    private Unit(WKUnit core) {
        this.core = core;

        // don't cache base unit to avoid recursion; cost of get is cheap

        this.currencySupplier = Suppliers.memoize(() -> Currency.create(core.getCurrency()));
        this.nameSupplier = Suppliers.memoize(core::getName);
        this.symbolSupplier = Suppliers.memoize(core::getSymbol);
        this.uidsSupplier = Suppliers.memoize(core::getUids);
        this.decimalsSupplier = Suppliers.memoize(core::getDecimals);
    }

    @Override
    public Currency getCurrency() {
        return currencySupplier.get();
    }

    @Override
    public String getName() {
        return nameSupplier.get();
    }

    @Override
    public String getSymbol() {
        return symbolSupplier.get();
    }

    @Override
    public Unit getBase() {
        return Unit.create(core.getBaseUnit());
    }

    @Override
    public UnsignedInteger getDecimals() {
        return decimalsSupplier.get();
    }

    @Override
    public boolean isCompatible(com.blockset.walletkit.Unit other) {
        return core.isCompatible(from(other).core);
    }

    @Override
    public boolean hasCurrency(com.blockset.walletkit.Currency currency) {
        return core.hasCurrency(Currency.from(currency).getCoreBRCryptoCurrency());
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }

        if (o == null || getClass() != o.getClass()) {
            return false;
        }

        Unit unit = (Unit) o;
        return core.isIdentical(unit.core);
    }

    @Override
    public int hashCode() {
        return Objects.hash(uidsSupplier.get());
    }

    /* package */
    WKUnit getCoreBRCryptoUnit() {
        return core;
    }
}
