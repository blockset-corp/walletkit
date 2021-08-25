/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKTransfer;
import com.blockset.walletkit.TransferDirection;
import com.blockset.walletkit.TransferState;
import com.google.common.base.Optional;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;
import com.google.common.primitives.UnsignedLong;

import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

import static com.google.common.base.Preconditions.checkState;

/* package */
final class Transfer implements com.blockset.walletkit.Transfer {

    /* package */
    static Transfer create (WKTransfer core, Wallet wallet, boolean take) {
        Transfer transfer = new Transfer(
                (take ? core.take() : core),
                wallet);
        ReferenceCleaner.register(transfer, core::give);
        return transfer;
    }

    /* package */
    static Transfer from(com.blockset.walletkit.Transfer transfer) {
        if (transfer == null) {
            return null;
        }

        if (transfer instanceof Transfer) {
            return (Transfer) transfer;
        }

        throw new IllegalArgumentException("Unsupported transfer instance");
    }

    private final WKTransfer core;
    private final Wallet wallet;

    private final Supplier<Unit> unitSupplier;
    private final Supplier<Unit> unitForFeeSupplier;
    private final Supplier<Optional<TransferFeeBasis>> estimatedFeeBasisSupplier;
    private final Supplier<Optional<Address>> sourceSupplier;
    private final Supplier<Optional<Address>> targetSupplier;
    private final Supplier<Amount> amountSupplier;
    private final Supplier<TransferDirection> directionSupplier;
    private final Supplier<Set<TransferAttribute>> attributesSupplier;

    private Transfer(WKTransfer core, Wallet wallet) {
        this.core = core;
        this.wallet = wallet;

        this.unitSupplier = Suppliers.memoize(() -> Unit.create(core.getUnitForAmount()));
        this.unitForFeeSupplier = Suppliers.memoize(() -> Unit.create(core.getUnitForFee()));
        this.estimatedFeeBasisSupplier = Suppliers.memoize(() -> core.getEstimatedFeeBasis().transform(TransferFeeBasis::create));

        this.sourceSupplier = Suppliers.memoize(() -> core.getSourceAddress().transform(Address::create));
        this.targetSupplier = Suppliers.memoize(() -> core.getTargetAddress().transform(Address::create));
        this.amountSupplier = Suppliers.memoize(() -> Amount.create(core.getAmount()));
        this.directionSupplier = Suppliers.memoize(() -> Utilities.transferDirectionFromCrypto(core.getDirection()));

        attributesSupplier = Suppliers.memoize(() -> {
            Set<TransferAttribute> attributes = new HashSet<>();
            UnsignedLong count = core.getAttributeCount();
            for (UnsignedLong i = UnsignedLong.ZERO; i.compareTo(count) < 0; i = i.plus(UnsignedLong.ONE)) {
                Optional<TransferAttribute> attribute = core.getAttributeAt(i)
                        .transform(TransferAttribute::create); // Uses the 'take' from '...AttributeAt'
                if (attribute.isPresent())
                    attributes.add(attribute.get().copy());
            }
            return attributes;
        });
    }

    @Override
    public Wallet getWallet() {
        return wallet;
    }

    @Override
    public Optional<Address> getSource() {
        return sourceSupplier.get();
    }

    @Override
    public Optional<Address> getTarget() {
        return targetSupplier.get();
    }

    @Override
    public Amount getAmount() {
        return amountSupplier.get();
    }

    @Override
     public Amount getAmountDirected() {
        return Amount.create(core.getAmountDirected());
    }

    @Override
    public Amount getFee() {
        Optional<TransferFeeBasis> maybeFee = getConfirmedFeeBasis().or(getEstimatedFeeBasis());
        checkState(maybeFee.isPresent());
        return maybeFee.get().getFee();
    }

    @Override
    public Optional<TransferFeeBasis> getEstimatedFeeBasis() {
        return estimatedFeeBasisSupplier.get();
    }

    @Override
    public Optional<TransferFeeBasis> getConfirmedFeeBasis() {
        return core.getConfirmedFeeBasis().transform(TransferFeeBasis::create);
    }

    @Override
    public TransferDirection getDirection() {
        return directionSupplier.get();
    }

    @Override
    public Optional<String> getIdentifier() {
        return core.getIdentifier();
    }

    @Override
    public Optional<TransferHash> getHash() {
        return core.getHash().transform(TransferHash::create);
    }

    @Override
    public Set<TransferAttribute> getAttributes() {
        return attributesSupplier.get();
    }

    @Override
    public com.blockset.walletkit.Unit getUnit() {
        return unitSupplier.get();
    }

    @Override
    public com.blockset.walletkit.Unit getUnitForFee() {
        return unitForFeeSupplier.get();
    }

    @Override
    public TransferState getState() {
        return Utilities.transferStateFromCrypto(core.getState());
    }

    @Override
    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }

        if (!(object instanceof Transfer)) {
            return false;
        }

        Transfer that = (Transfer) object;
        return core.equals(that.core);
    }

    @Override
    public int hashCode() {
        return Objects.hash(core);
    }

    /* package */
    WKTransfer getCoreBRCryptoTransfer() {
        return core;
    }
}
