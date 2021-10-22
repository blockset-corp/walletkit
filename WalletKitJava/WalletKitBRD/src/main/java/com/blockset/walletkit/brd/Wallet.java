/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.NetworkType;
import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKAddress;
import com.blockset.walletkit.nativex.WKAmount;
import com.blockset.walletkit.nativex.WKFeeBasis;
import com.blockset.walletkit.nativex.WKNetworkFee;
import com.blockset.walletkit.nativex.WKPaymentProtocolRequest;
import com.blockset.walletkit.nativex.WKTransfer;
import com.blockset.walletkit.nativex.WKTransferAttribute;
import com.blockset.walletkit.nativex.WKWallet;
import com.blockset.walletkit.nativex.WKWalletManager;
import com.blockset.walletkit.nativex.WKWalletSweeper;
import com.blockset.walletkit.AddressScheme;
import com.blockset.walletkit.WalletState;
import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.errors.LimitEstimationError;
import com.blockset.walletkit.errors.LimitEstimationInsufficientFundsError;
import com.blockset.walletkit.errors.LimitEstimationServiceFailureError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;
import com.google.common.base.Supplier;
import com.google.common.base.Suppliers;
import com.google.common.primitives.UnsignedLong;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import javax.annotation.Nullable;

import static com.google.common.base.Preconditions.checkState;

/* package */
final class Wallet implements com.blockset.walletkit.Wallet {

    /* package */
    static Wallet takeAndCreate(WKWallet core, WalletManager walletManager, SystemCallbackCoordinator callbackCoordinator) {
        return Wallet.create(core.take(), walletManager, callbackCoordinator);
    }

    /* package */
    static Wallet create(WKWallet core, WalletManager walletManager, SystemCallbackCoordinator callbackCoordinator) {
        Wallet wallet = new Wallet(core, walletManager, callbackCoordinator);
        ReferenceCleaner.register(wallet, core::give);
        return wallet;
    }

    /* package */
    static Wallet from(com.blockset.walletkit.Wallet wallet) {
        if (wallet == null) {
            return null;
        }

        if (wallet instanceof Wallet) {
            return (Wallet) wallet;
        }

        throw new IllegalArgumentException("Unsupported wallet instance");
    }

    private final WKWallet core;
    private final WalletManager walletManager;
    private final SystemCallbackCoordinator callbackCoordinator;

    private final Supplier<Unit> unitSupplier;
    private final Supplier<Unit> unitForFeeSupplier;
    private final Supplier<Currency> defaultUnitCurrencySupplier;

    private Wallet(WKWallet core, WalletManager walletManager, SystemCallbackCoordinator callbackCoordinator) {
        this.core = core;
        this.walletManager = walletManager;
        this.callbackCoordinator = callbackCoordinator;

        this.unitSupplier = Suppliers.memoize(() -> Unit.create(core.getUnit()));
        this.unitForFeeSupplier = Suppliers.memoize(() -> Unit.create(core.getUnitForFee()));
        this.defaultUnitCurrencySupplier = Suppliers.memoize(() -> Currency.create(core.getCurrency()));
    }

    /* package */
    Transfer transferBy(WKTransfer coreTransfer) {
        if (!core.containsTransfer(coreTransfer)) {
            return Transfer.create(coreTransfer, this, true);
        }
        return null;
    }

    /* package */
    Transfer transferByCoreOrCreate(WKTransfer  coreTransfer,
                                    boolean     create  ) {
        Transfer transfer = transferBy(coreTransfer);
        if (transfer == null && create) {
            transfer = Transfer.create(coreTransfer, this, true);
        }
        return transfer;
    }

    @Override
    public Optional<Transfer> createTransfer(com.blockset.walletkit.Address target,
                                             com.blockset.walletkit.Amount amount,
                                             com.blockset.walletkit.TransferFeeBasis estimatedFeeBasis,
                                             @Nullable Set<com.blockset.walletkit.TransferAttribute> attributes) {
        WKAddress coreAddress = Address.from(target).getCoreBRCryptoAddress();
        WKFeeBasis coreFeeBasis = TransferFeeBasis.from(estimatedFeeBasis).getCoreBRFeeBasis();
        WKAmount coreAmount = Amount.from(amount).getCoreBRCryptoAmount();

        List<WKTransferAttribute> coreAttributes = new ArrayList<>();
        if (null != attributes)
            for (com.blockset.walletkit.TransferAttribute attribute : attributes) {
                coreAttributes.add (TransferAttribute.from(attribute).getCoreBRCryptoTransferAttribute());
            }

        return core.createTransfer(coreAddress, coreAmount, coreFeeBasis, coreAttributes)
                .transform(t -> Transfer.create(t, this, false));
    }

    /* package */
    Optional<Transfer> createTransfer(WalletSweeper sweeper,
                                      com.blockset.walletkit.TransferFeeBasis estimatedFeeBasis) {
        WKWalletSweeper coreSweeper = sweeper.getCoreBRWalletSweeper();
        WKFeeBasis coreFeeBasis = TransferFeeBasis.from(estimatedFeeBasis).getCoreBRFeeBasis();
        return core.createTransferForWalletSweep(coreSweeper, getWalletManager().getCoreBRCryptoWalletManager(), coreFeeBasis)
                .transform(t -> Transfer.create(t, this, false));
    }

    /* package */
    Optional<Transfer> createTransfer(PaymentProtocolRequest request,
                                      com.blockset.walletkit.TransferFeeBasis estimatedFeeBasis) {
        WKPaymentProtocolRequest coreRequest = request.getBRCryptoPaymentProtocolRequest();
        WKFeeBasis coreFeeBasis = TransferFeeBasis.from(estimatedFeeBasis).getCoreBRFeeBasis();
        return core.createTransferForPaymentProtocolRequest(coreRequest, coreFeeBasis)
                .transform(t -> Transfer.create(t, this, false));
    }

    private com.blockset.walletkit.Amount hackTheAmountIfTezos(com.blockset.walletkit.Amount amount) {
        Network network = getWalletManager().getNetwork();

        switch (network.getType()) {
            case XTZ:
                // See BRTezosOperation.c
                long TEZOS_FEE_DEFAULT = 0;

                Unit unitBase = network.baseUnitFor(amount.getCurrency()).get();
                Amount amountSlop = Amount.create(1 + TEZOS_FEE_DEFAULT, unitBase);

                //
                // A Tezos fee estimation for an amount such that:
                //     `(balance - TEZOS_FEE_DEFAULT) <= amount <= balance`
                // will return "balance_too_low" but you can actually send a tranaction with roughly
                //     `amount < (balance - 424)`
                // where 424 is the fee for 1mutez (424 is typical)
                //
                // So, if asked to perform a fee estimate for an amount within TEZOS_FEE_DEFAULT of
                // balance we'll instead use an amount of (balance - TEZOS_FEE_DEFAULT - 1).  Note: if
                // balance < TEZOS_FEE_DEFAULT, we'll use an amout of 1.
                //

                return (this.getBalance().compareTo(amount.add(amountSlop).get()) > 0 /* GT */
                        ? amount
                        : (this.getBalance().compareTo(amountSlop) > 0 /* GT */
                           ? this.getBalance().sub(amountSlop).get()
                           : Amount.create(1, unitBase)));

            default:
                return amount;
        }
    }

    @Override
    public void estimateFee(com.blockset.walletkit.Address target,
                            com.blockset.walletkit.Amount amount,
                            com.blockset.walletkit.NetworkFee fee,
                            @Nullable Set<com.blockset.walletkit.TransferAttribute> attributes,
                            CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError> handler) {
        com.blockset.walletkit.Amount amountHackedIfXTZ = hackTheAmountIfTezos(amount);

        WKWalletManager coreManager = getWalletManager().getCoreBRCryptoWalletManager();
        WKAddress coreAddress = Address.from(target).getCoreBRCryptoAddress();
        WKAmount coreAmount = Amount.from(amountHackedIfXTZ).getCoreBRCryptoAmount();
        WKNetworkFee coreFee = NetworkFee.from(fee).getCoreBRCryptoNetworkFee();
        List<WKTransferAttribute> coreAttributes = new ArrayList<>();
        if (null != attributes)
            for (com.blockset.walletkit.TransferAttribute attribute : attributes) {
                coreAttributes.add (TransferAttribute.from(attribute).getCoreBRCryptoTransferAttribute());
            }
        coreManager.estimateFeeBasis(core, callbackCoordinator.registerFeeBasisEstimateHandler(handler), coreAddress, coreAmount, coreFee, coreAttributes);
    }

    /* package */
    void estimateFee(WalletSweeper sweeper,
                     com.blockset.walletkit.NetworkFee fee, CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError> handler) {
        WKWalletManager coreManager = getWalletManager().getCoreBRCryptoWalletManager();
        WKWalletSweeper coreSweeper = sweeper.getCoreBRWalletSweeper();
        WKNetworkFee coreFee = NetworkFee.from(fee).getCoreBRCryptoNetworkFee();
        coreManager.estimateFeeBasisForWalletSweep(core, callbackCoordinator.registerFeeBasisEstimateHandler(handler), coreSweeper, coreFee);
    }

    /* package */
    void estimateFee(PaymentProtocolRequest request,
                     com.blockset.walletkit.NetworkFee fee, CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError> handler) {
        WKWalletManager coreManager = getWalletManager().getCoreBRCryptoWalletManager();
        WKPaymentProtocolRequest coreRequest = request.getBRCryptoPaymentProtocolRequest();
        WKNetworkFee coreFee = NetworkFee.from(fee).getCoreBRCryptoNetworkFee();
        coreManager.estimateFeeBasisForPaymentProtocolRequest(core, callbackCoordinator.registerFeeBasisEstimateHandler(handler), coreRequest, coreFee);
    }

    @Override
    public void estimateLimitMaximum(com.blockset.walletkit.Address target, com.blockset.walletkit.NetworkFee fee,
                                     CompletionHandler<com.blockset.walletkit.Amount, LimitEstimationError> handler) {
        estimateLimit(true, target, fee, handler);
    }

    @Override
    public void estimateLimitMinimum(com.blockset.walletkit.Address target, com.blockset.walletkit.NetworkFee fee,
                                     CompletionHandler<com.blockset.walletkit.Amount, LimitEstimationError> handler) {
        estimateLimit(false, target, fee, handler);
    }

    private void estimateLimit(boolean asMaximum,
                               com.blockset.walletkit.Address target, com.blockset.walletkit.NetworkFee fee,
                               CompletionHandler<com.blockset.walletkit.Amount, LimitEstimationError> handler) {
        WKWalletManager coreManager = getWalletManager().getCoreBRCryptoWalletManager();

        NetworkFee cryptoFee = NetworkFee.from(fee);
        WKNetworkFee coreFee = cryptoFee.getCoreBRCryptoNetworkFee();
        WKAddress coreAddress = Address.from(target).getCoreBRCryptoAddress();

        // This `amount` is in the `unit` of `wallet`
        WKWalletManager.EstimateLimitResult result = coreManager.estimateLimit(core, asMaximum, coreAddress, coreFee);
        if (result.amount == null) {
            // This is extraneous as `cryptoWalletEstimateLimit()` always returns an amount
            callbackCoordinator.completeLimitEstimateWithError(handler, new LimitEstimationInsufficientFundsError());
            return;
        }

        Amount amount = Amount.create(result.amount);
        boolean needFeeEstimate = result.needFeeEstimate;
        boolean isZeroIfInsuffientFunds = result.isZeroIfInsuffientFunds;

        // If we don't need an estimate, then we invoke `completion` and skip out immediately.  But
        // include a check on a zero amount - which indicates insufficient funds.
        if (!needFeeEstimate) {
            if (isZeroIfInsuffientFunds && amount.isZero()) {
                callbackCoordinator.completeLimitEstimateWithError(handler, new LimitEstimationInsufficientFundsError());
            } else {
                callbackCoordinator.completeLimitEstimateWithSuccess(handler, amount);
            }
            return;
        }

        // We need an estimate of the fees.

        // The currency for the fee
        Currency currencyForFee = cryptoFee.getPricePerCostFactor().getCurrency();

        Wallet walletForFee = null;
        for (Wallet wallet: walletManager.getWallets()) {
            if (currencyForFee.equals(wallet.getCurrency())) {
                walletForFee = wallet;
                break;
            }
        }
        if (null == walletForFee) {
            callbackCoordinator.completeLimitEstimateWithError(handler, new LimitEstimationServiceFailureError());
            return;
        }

        // Skip out immediately if we've no balance.
        if (walletForFee.getBalance().isZero()) {
            callbackCoordinator.completeLimitEstimateWithError(handler, new LimitEstimationInsufficientFundsError());
            return;
        }

        //
        // If the `walletForFee` differs from `wallet` then we just need to estimate the fee
        // once.  Get the fee estimate and just ensure that walletForFee has sufficient balance
        // to pay the fee.
        //
        if (!this.equals(walletForFee)) {
            // This `amount` will not unusually be zero.
            // TODO: Does ETH fee estimation work if the ERC20 amount is zero?
            final Wallet walletForFeeInner = walletForFee;
            estimateFee(target, amount, fee, null, new CompletionHandler<com.blockset.walletkit.TransferFeeBasis,
                    FeeEstimationError>() {
                @Override
                public void handleData(com.blockset.walletkit.TransferFeeBasis feeBasis) {
                    if (walletForFeeInner.getBalance().compareTo(feeBasis.getFee()) >= 0) {
                        handler.handleData(amount);
                    } else {
                        handler.handleError(new LimitEstimationInsufficientFundsError());
                    }
                }

                @Override
                public void handleError(FeeEstimationError error) {
                    handler.handleError(LimitEstimationError.from(error));
                }
            });
            return;
        }

        // The `fee` is in the same unit as the `wallet`

        //
        // If we are estimating the minimum, then get the fee and ensure that the wallet's
        // balance is enough to cover the (minimum) amount plus the fee
        //
        if (!asMaximum) {
            estimateFee(target, amount, fee, null, new CompletionHandler<com.blockset.walletkit.TransferFeeBasis,
                    FeeEstimationError>() {
                @Override
                public void handleData(com.blockset.walletkit.TransferFeeBasis feeBasis) {
                    Optional<Amount> transactionAmount = amount.add(feeBasis.getFee());
                    checkState(transactionAmount.isPresent());

                    if (getBalance().compareTo(transactionAmount.get()) >= 0) {
                        handler.handleData(amount);
                    } else {
                        handler.handleError(new LimitEstimationInsufficientFundsError());
                    }
                }

                @Override
                public void handleError(FeeEstimationError error) {
                    handler.handleError(LimitEstimationError.from(error));
                }
            });
            return;
        }

        //
        // We are forced to deal with XTZ.  Not by our choosing.  The value returned by the above
        // `coreManager.estimateLimit` is something well below `self.balance` for XTZ - because
        // we are desperate to get a non-error response from the XTZ node.  And, if we provide the
        // balance for the estimate, we get a `balance_too_low` error.  This then forces us into
        // a binary search until 'not balance_too_low' which for a range of {0, 1 xtz} is ~25
        // queries of Blockset and the XTZ Node.  Insane.  We will unfortunately sacrifice our
        // User's funds until XTZ matures.
        //
        if (NetworkType.XTZ == walletManager.getNetwork().getType()) {

            // The absolute minimum value that can be transferred.  If we can't get an estimate for
            // this we are utterly dead in the water.
            Amount transferMin  = Amount.create(1, walletManager.getBaseUnit());
            Amount transferZero = Amount.create(0, walletManager.getBaseUnit());

            CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError> estimationHandlerXTZ =
                    new CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError>() {
                        @Override
                        public void handleData(com.blockset.walletkit.TransferFeeBasis feeBasis) {
                            Amount amountEstimated = amount.sub(feeBasis.getFee()).or(transferZero);
                            handler.handleData(amountEstimated.compareTo(amount) == -1
                                    ? amountEstimated
                                    : amount);
                        }

                        @Override
                        public void handleError(FeeEstimationError error) {
                            //
                            // The request failed but we don't know why (limits in the current interface).
                            // Could be a network failure; could be something with the XTZ wallet; could
                            // be a protocol change for XTZ - no matter, we'll return the maximum amount
                            // as the balance.  If the user attempts to send the 'balance' it will fail
                            // as there won't be enough for the fee.
                            //
                            handler.handleData(amount);
                        }
                    };

            estimateFee(target, transferMin, fee, null, estimationHandlerXTZ);

            return;
        }

        // This function will be recursively defined
        CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError> estimationHandler =
                new CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError>() {
            // If the `walletForFee` and `wallet` are identical, then we need to iteratively estimate
            // the fee and adjust the amount until the fee stabilizes.
            com.blockset.walletkit.Amount transferFee = Amount.create(0, getUnit());

            // We'll limit the number of iterations
            int estimationCompleterRecurseLimit = 3;
            int estimationCompleterRecurseCount = 0;

            @Override
            public void handleData(com.blockset.walletkit.TransferFeeBasis feeBasis) {
                estimationCompleterRecurseCount += 1;

                // The estimated transfer fee
                com.blockset.walletkit.Amount newTransferFee = feeBasis.getFee();

                // The estimated transfer amount, updated with the transferFee
                Optional<Amount> newTransferAmount = amount.sub(newTransferFee);
                checkState(newTransferAmount.isPresent());

                // If the two transfer fees match, then we have converged
                if (transferFee.equals(newTransferFee)) {
                    Optional<Amount> transactionAmount = newTransferAmount.get().add(newTransferFee);
                    checkState(transactionAmount.isPresent());

                    if (getBalance().compareTo(transactionAmount.get()) >= 0) {
                        handler.handleData(newTransferAmount.get());
                    } else {
                        handler.handleError(new LimitEstimationInsufficientFundsError());
                    }

                } else if (estimationCompleterRecurseCount < estimationCompleterRecurseLimit) {
                    // but is they haven't converged try again with the new amount
                    transferFee = newTransferFee;
                    estimateFee(target, newTransferAmount.get(), fee, null, this);

                } else {
                    // We've tried too many times w/o convergence; abort
                    handler.handleError(new LimitEstimationServiceFailureError());
                }
            }

            @Override
            public void handleError(FeeEstimationError error) {
                handler.handleError(LimitEstimationError.from(error));
            }
        };

        estimateFee(target, amount, fee, null, estimationHandler);
    }

    @Override
    public WalletManager getWalletManager() {
        return walletManager;
    }

    @Override
    public List<Transfer> getTransfers() {
        List<Transfer> transfers = new ArrayList<>();

        for (WKTransfer transfer: core.getTransfers()) {
            transfers.add(Transfer.create(transfer, this, false));
        }

        return transfers;
    }

    @Override
    public Optional<Transfer> getTransferByHash(com.blockset.walletkit.TransferHash hash) {
        List<Transfer> transfers = getTransfers();

        for (Transfer transfer : transfers) {
            Optional<TransferHash> optional = transfer.getHash();
            if (optional.isPresent() && optional.get().equals(hash)) {
                return Optional.of(transfer);
            }
        }
        return Optional.absent();
    }

    @Override
    public Set<TransferAttribute> getTransferAttributesFor(@Nullable com.blockset.walletkit.Address target) {
        WKAddress coreTarget = (null == target ? null : Address.from(target).getCoreBRCryptoAddress());

        Set<TransferAttribute> attributes = new HashSet<>();
        UnsignedLong count = core.getTransferAttributeCount(coreTarget);

        for (UnsignedLong i = UnsignedLong.ZERO; i.compareTo(count) < 0; i = i.plus(UnsignedLong.ONE)) {
            Optional<TransferAttribute> attribute = core.getTransferAttributeAt(coreTarget, i)
                    .transform(TransferAttribute::create);  // Uses the 'take' from '...AttributeAt'
            if (attribute.isPresent()) {
                attributes.add (attribute.get().copy());
            }
        }
        return attributes;
    }

    @Override
    public Optional<TransferAttribute.Error> validateTransferAttribute(com.blockset.walletkit.TransferAttribute attribute) {
        WKTransferAttribute coreAttribute =
                TransferAttribute.from(attribute).getCoreBRCryptoTransferAttribute();

        return core.validateTransferAttribute(coreAttribute)
                .transform(Utilities::transferAttributeErrorFromCrypto);
    }

    @Override
    public Optional<TransferAttribute.Error> validateTransferAttributes(Set<com.blockset.walletkit.TransferAttribute> attributes) {

        List<WKTransferAttribute> coreAttributes = new ArrayList<>();
        for (com.blockset.walletkit.TransferAttribute attribute : attributes)
            coreAttributes.add (TransferAttribute.from(attribute).getCoreBRCryptoTransferAttribute());

        return core.validateTransferAttributes(coreAttributes)
                .transform(Utilities::transferAttributeErrorFromCrypto);
    }

    @Override
    public Unit getUnit() {
        return unitSupplier.get();
    }

    @Override
    public Unit getUnitForFee() {
        return unitForFeeSupplier.get();
    }

    @Override
    public Amount getBalance() {
        return Amount.create(core.getBalance());
    }

    @Override
    public Optional<Amount> getBalanceMaximum() {
        return core.getBalanceMaximum()
                .transform(Amount::create);
    }

    @Override
    public Optional<Amount> getBalanceMinimum() {
        return core.getBalanceMinimum()
                .transform(Amount::create);
    }

    @Override
    public WalletState getState() {
        return Utilities.walletStateFromCrypto(core.getState());
    }

    @Override
    public Address getTarget() {
        return getTargetForScheme(walletManager.getAddressScheme());
    }

    @Override
    public Address getTargetForScheme(AddressScheme scheme) {
        return Address.create(core.getTargetAddress(Utilities.addressSchemeToCrypto(scheme)));
    }

    @Override
    public boolean containsAddress(com.blockset.walletkit.Address address) {
        return core.containsAddress(Address.from(address).getCoreBRCryptoAddress());
    }

    @Override
    public Currency getCurrency() {
        return defaultUnitCurrencySupplier.get();
    }

    @Override
    public String getName() {
        return getCurrency().getCode();
    }

    @Override
    public boolean equals(Object object) {
        if (this == object) {
            return true;
        }

        if (!(object instanceof Wallet)) {
            return false;
        }

        Wallet that = (Wallet) object;
        return core.equals(that.core);
    }

    @Override
    public int hashCode() {
        return Objects.hash(core);
    }

    /* package */
    Optional<Transfer> getTransfer(WKTransfer transfer) {
        return core.containsTransfer(transfer) ?
                Optional.of(Transfer.create(transfer, this, true)) :
                Optional.absent();
    }

    /* package */
    Transfer createTransfer(WKTransfer transfer) {
        return Transfer.create(transfer, this, true);
    }

    /* package */
    WKWallet getCoreBRCryptoWallet() {
        return core;
    }
}
