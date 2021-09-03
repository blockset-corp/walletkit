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
import com.blockset.walletkit.nativex.WKAmount;
import com.blockset.walletkit.CurrencyPair;
import com.blockset.walletkit.nativex.WKComparison;
import com.google.common.base.Optional;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;

import java.math.RoundingMode;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.text.NumberFormat;
import java.util.Objects;

import static com.google.common.base.Preconditions.checkArgument;

/* package */
final class Amount implements com.blockset.walletkit.Amount {

    /* package */
    static Amount create(double value, com.blockset.walletkit.Unit unit) {
        Unit cryptoUnit = Unit.from(unit);
        WKAmount core = WKAmount.create(value, cryptoUnit.getCoreBRCryptoUnit());
        return Amount.create(core);
    }

    /* package */
    static Amount create(long value, com.blockset.walletkit.Unit unit) {
        Unit cryptoUnit = Unit.from(unit);
        WKAmount core = WKAmount.create(value, cryptoUnit.getCoreBRCryptoUnit());
        return Amount.create(core);
    }

    /* package */
    static Optional<Amount> create(String value, boolean isNegative, com.blockset.walletkit.Unit unit) {
        Unit cryptoUnit = Unit.from(unit);
        Optional<WKAmount> core = WKAmount.create(value, isNegative, cryptoUnit.getCoreBRCryptoUnit());
        return core.transform(Amount::create);
    }

    /* package */
    static Amount create(WKAmount core) {
        Amount amount = new Amount(core);
        ReferenceCleaner.register(amount, core::give);
        return amount;
    }

    /* package */
    static Amount from(com.blockset.walletkit.Amount amount) {
        if (amount == null) {
            return null;
        }

        if (amount instanceof Amount) {
            return (Amount) amount;
        }

        throw new IllegalArgumentException("Unsupported amount instance");
    }

    private static NumberFormat formatterWithUnit(com.blockset.walletkit.Unit unit) {
        DecimalFormat formatter = (DecimalFormat) DecimalFormat.getCurrencyInstance().clone();
        DecimalFormatSymbols formatterSymbols = (DecimalFormatSymbols) formatter.getDecimalFormatSymbols().clone();

        String symbol = unit.getSymbol();
        formatterSymbols.setInternationalCurrencySymbol(symbol);
        formatterSymbols.setCurrencySymbol(symbol);

        int decimals = unit.getDecimals().intValue();
        formatter.setParseBigDecimal(0 != decimals);
        formatter.setRoundingMode(RoundingMode.HALF_EVEN);
        formatter.setDecimalFormatSymbols(formatterSymbols);
        formatter.setMaximumIntegerDigits(Integer.MAX_VALUE);
        formatter.setMaximumFractionDigits(decimals);

        return formatter;
    }

    private final WKAmount core;

    private final Supplier<Unit> unitSupplier;
    private final Supplier<Currency> currencySupplier;
    private final Supplier<String> toStringSupplier;

    private Amount(WKAmount core) {
        this.core = core;

        this.currencySupplier = Suppliers.memoize(() -> Currency.create(core.getCurrency()));
        this.unitSupplier = Suppliers.memoize(() -> Unit.create(core.getUnit()));
        this.toStringSupplier = Suppliers.memoize(() -> toStringAsUnit(getUnit()).or("<nan>"));
    }

    @Override
    public Currency getCurrency() {
        return currencySupplier.get();
    }

    @Override
    public Unit getUnit() {
        return unitSupplier.get();
    }

    @Override
    public boolean hasCurrency(com.blockset.walletkit.Currency currency) {
        return core.hasCurrency(Currency.from(currency).getCoreBRCryptoCurrency());
    }

    @Override
    public boolean isCompatible(com.blockset.walletkit.Amount withAmount) {
        return core.isCompatible(from(withAmount).core);
    }

    @Override
    public boolean isNegative() {
        return core.isNegative();
    }

    @Override
    public boolean isZero() {
        return core.isZero();
    }

    @Override
    public Optional<Amount> add(com.blockset.walletkit.Amount o) {
        checkArgument(isCompatible(o));

        return core.add(from(o).core).transform(Amount::create);
    }

    @Override
    public Optional<Amount> sub(com.blockset.walletkit.Amount o) {
        checkArgument(isCompatible(o));

        return core.sub(from(o).core).transform(Amount::create);
    }

    @Override
    public Amount negate() {
        return Amount.create(core.negate());
    }

    @Override
    public Optional<Amount> convert(com.blockset.walletkit.Unit toUnit) {
        return core.convert(Unit.from(toUnit).getCoreBRCryptoUnit()).transform(Amount::create);
    }

    @Override
    public Optional<String> toStringAsUnit(com.blockset.walletkit.Unit asUnit) {
        return toStringAsUnit(asUnit, null);
    }

    @Override
    public Optional<String> toStringAsUnit(com.blockset.walletkit.Unit asUnit, @Nullable NumberFormat numberFormatter) {
        numberFormatter = numberFormatter != null ? numberFormatter : formatterWithUnit(asUnit);
        return doubleAmount(asUnit).transform(numberFormatter::format);
    }

    @Override
    public Optional<String> toStringFromPair(CurrencyPair pair) {
        return toStringFromPair(pair, null);
    }

    @Override
    public Optional<String> toStringFromPair(CurrencyPair pair, @Nullable NumberFormat numberFormatter) {
        Optional<? extends com.blockset.walletkit.Amount> amount = pair.exchangeAsBase(this);
        if (amount.isPresent()) {
            return amount.get().toStringAsUnit(pair.getQuoteUnit(), numberFormatter);
        } else {
            return Optional.absent();
        }
    }

    @Override
    public String toStringWithBase(int base, String preface) {
        return core.toStringWithBase(base, preface);
    }

    @Override
    public String toString() {
        return toStringSupplier.get();
    }

    @Override
    public int compareTo(com.blockset.walletkit.Amount o) {
        switch (core.compare(from(o).core)) {
            case EQ: return 0;
            case LT: return -1;
            case GT: return 1;
            default: throw new IllegalStateException("Invalid amount comparison");
        }
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }

        if (o == null || getClass() != o.getClass()) {
            return false;
        }

        Amount amount = (Amount) o;
        return WKComparison.EQ == core.compare(amount.core);
    }

    @Override
    public int hashCode() {
        return Objects.hash(toString());
    }

    @Override
    public Optional<Double> doubleAmount(com.blockset.walletkit.Unit asUnit) {
        return core.getDouble(Unit.from(asUnit).getCoreBRCryptoUnit());
    }

    /* package */
    WKAmount getCoreBRCryptoAmount() {
        return core;
    }
}
