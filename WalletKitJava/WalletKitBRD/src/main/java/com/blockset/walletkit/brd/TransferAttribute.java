/*
 * Created by Michael Carrara.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKTransferAttribute;
import com.google.common.base.Optional;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;

import javax.annotation.Nullable;

public class TransferAttribute implements com.blockset.walletkit.TransferAttribute {

    /* package */
    static TransferAttribute create(WKTransferAttribute core) {
        TransferAttribute attribute = new TransferAttribute(core);
        ReferenceCleaner.register(attribute, core::give);
        return attribute;
    }

    static TransferAttribute from(com.blockset.walletkit.TransferAttribute attribute) {
        if (attribute == null) {
            return null;
        }

        if (attribute instanceof TransferAttribute) {
            return (TransferAttribute) attribute;
        }

        throw new IllegalArgumentException("Unsupported TransferAttribute instance");
    }

    private final WKTransferAttribute core;

//    private final int value;
    private final Supplier<String> keySupplier;
    private final Supplier<Boolean> isRequiredSupplier;

    private TransferAttribute(WKTransferAttribute core) {
        this.core = core;

        this.keySupplier = Suppliers.memoize(() -> core.getKey());
        this.isRequiredSupplier = Suppliers.memoize(() -> core.isRequired());
    }

    @Override
    public String getKey() {
        return keySupplier.get();
    }

    @Override
    public Optional<String> getValue() {
        return core.getValue();
    }

    @Override
    public void setValue(@Nullable String value) {
        core.setValue(value);
    }

    @Override
    public boolean isRequired() {
        return isRequiredSupplier.get();
    }

    public TransferAttribute copy () {
        return TransferAttribute.create (core.copy());
    }

    /* package */
    WKTransferAttribute getCoreBRCryptoTransferAttribute() {
        return core;
    }

    public boolean equalsTransferAttribute (TransferAttribute that) {
        return this.core == that.core || this.getKey().equals(that.getKey());
    }

    @Override
    public boolean equals(Object that) {
        return this == that
                || (that instanceof TransferAttribute && equalsTransferAttribute((TransferAttribute) that))
                || (that instanceof com.blockset.walletkit.TransferAttribute && this.getKey().equals(((com.blockset.walletkit.TransferAttribute) that).getKey()));
    }

    @Override
    public int hashCode() {
        return getKey().hashCode();
    }
}
