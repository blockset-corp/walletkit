/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.errors.QueryError;
import com.blockset.walletkit.SystemClient.Blockchain;
import com.blockset.walletkit.SystemClient.BlockchainFee;
import com.blockset.walletkit.SystemClient.CurrencyDenomination;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Function;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedLong;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.logging.Level;
import java.util.logging.Logger;

/* package */
final class NetworkDiscovery {

    private static final Logger Log = Logger.getLogger(NetworkDiscovery.class.getName());

    /* package */
    interface Callback {
        void discovered(Network network);
        void updated(Network network);
        void complete(List<Network> networks);
    }

    /* package */
    static void discoverNetworks(SystemClient query,
                                 boolean isMainnet,
                                 List<Network> networks,
                                 List<SystemClient.Currency> appCurrencies,
                                 Callback callback) {
        CountUpAndDownLatch latch = new CountUpAndDownLatch(() -> callback.complete(networks));

        // The existing networks
        final List<Network> existingNetworks = networks;

        // The 'supportedNetworks' will be builtin networks matching isMainnet.
        final List<Network> supportedNetworks = networks; // findBuiltinNetworks(isMainnet);

         getBlockChains(latch, query, isMainnet, remoteModels -> {
             // If there are no 'remoteModels' the query might have failed.
             //
             // We ONLY support built-in blockchains; but the remotes have some
             // needed values - specifically the network fees and block height.

             getCurrencies(latch, query, isMainnet, appCurrencies, currencyModels -> {
                 // If there are no 'currencyModels' the query might have failed.
                 //
                 // Process each supportedNetwork based on the remote model
                 for (Network network: supportedNetworks) {
                    boolean existing = false; // existingNetworks.contains(network);
                    String blockchainModelId = network.getUids();

                    Network coreNetwork = Network.from(network);

                    for (SystemClient.Currency currencyModel : currencyModels) {
                        if (currencyModel.getBlockchainId().equals(blockchainModelId)) {
                            Currency currency = Currency.create(
                                    currencyModel.getId(),
                                    currencyModel.getName(),
                                    currencyModel.getCode(),
                                    currencyModel.getType(),
                                    currencyModel.getAddress().orNull());

                            Optional<CurrencyDenomination> baseDenomination = findFirstBaseDenomination(currencyModel.getDenominations());
                            List<CurrencyDenomination> nonBaseDenominations = findAllNonBaseDenominations(currencyModel.getDenominations());

                            Unit baseUnit = baseDenomination.isPresent()
                                    ? currencyDenominationToBaseUnit(currency, baseDenomination.get())
                                    : currencyToDefaultBaseUnit(currency);

                            List<Unit> units = currencyDenominationToUnits(currency, nonBaseDenominations, baseUnit);

                            units.add(0, baseUnit);
                            Collections.sort(units, (o1, o2) -> o2.getDecimals().compareTo(o1.getDecimals()));
                            Unit defaultUnit = units.get(0);

                            // The currency and unit here will not override builtins.
                            coreNetwork.addCurrency(currency, baseUnit, defaultUnit);
                            for (Unit u : units) {
                                coreNetwork.addUnitFor(currency, u);
                            }
                        }
                    }

                    Unit feeUnit = coreNetwork.baseUnitFor(network.getCurrency()).orNull();
                    if (null == feeUnit) { /* never here */
                        return null;
                    }

                    // Find a blockchainModel for this network; there might not be one.
                    Blockchain blockchainModel = null;
                    for (Blockchain model : remoteModels)
                        if (model.getId().equals(blockchainModelId)) {
                            blockchainModel = model;
                            break;
                        }

                    // If we have a blockchainModel for this network, process the model
                    if (null != blockchainModel) {

                        // Update the network's height
                        if (blockchainModel.getBlockHeight().isPresent()) {
                            Optional<UnsignedLong> blockHeight = blockchainModel.getBlockHeight();
                            coreNetwork.setHeight(blockHeight.isPresent() ? blockHeight.get() : Blockchain.BLOCK_HEIGHT_UNSPECIFIED);
                        }

                        if (blockchainModel.getVerifiedBlockHash().isPresent())
                            coreNetwork.setVerifiedBlockHashAsString(blockchainModel.getVerifiedBlockHash().get());

                        if (blockchainModel.getVerifiedBlockHash().isPresent())
                            network.setVerifiedBlockHashAsString(blockchainModel.getVerifiedBlockHash().get());

                        // Extract the network fees
                        List<NetworkFee> fees = new ArrayList<>();
                        for (BlockchainFee bdbFee : blockchainModel.getFeeEstimates()) {
                            Optional<Amount> amount = Amount.create(bdbFee.getAmount(), false, feeUnit);
                            if (amount.isPresent()) {
                                fees.add(NetworkFee.create(bdbFee.getConfirmationTimeInMilliseconds(), amount.get()));
                            }
                        }

                        if (fees.isEmpty()) {
                            Log.log(Level.FINE, String.format("Missed Fees %s", blockchainModel.getName()));
                        } else {
                            coreNetwork.setFees(fees);
                        }
                    } else {
                        Log.log(Level.FINE, String.format("Missed Model for Network: %s", blockchainModelId));
                    }

                    if (!existing) {
                        // Announce the network
                        callback.discovered(network);

                        // Keep a running total of discovered networks
//                        networks.add(network);
                    }
                    else {
                        callback.updated(network);
                    }
                }
                return null;
             });
            return null;
        });
    }

    private static void getBlockChains(CountUpAndDownLatch latch,
                                       SystemClient query,
                                       boolean isMainnet,
                                       Function<Collection<Blockchain>, Void> func) {
        latch.countUp();
        query.getBlockchains(isMainnet, new CompletionHandler<List<Blockchain>, QueryError>() {
            @Override
            public void handleData(List<Blockchain> remote) {
                try {
                    List<Blockchain> blockchains = new ArrayList<>(remote.size());
                    for (Blockchain blockchain: remote) {
                        if (blockchain.getBlockHeight().isPresent()) {
                            blockchains.add(blockchain);
                        }
                    }
                    func.apply(blockchains);
                } finally {
                    latch.countDown();
                }
            }

            @Override
            public void handleError(QueryError error) {
                try {
                    func.apply(Collections.emptyList());
                } finally {
                    latch.countDown();
                }
            }
        });
    }

    private static void getCurrencies(CountUpAndDownLatch latch,
                                      SystemClient query,
                                      String blockchainId,
                                      Collection<SystemClient.Currency> applicationCurrencies,
                                      Function<Collection<SystemClient.Currency>, Void> func) {
        latch.countUp();
        query.getCurrencies(blockchainId, null, new CompletionHandler<List<SystemClient.Currency>, QueryError>() {
            @Override
            public void handleData(List<SystemClient.Currency> newCurrencies) {
                try {
                    // On success, always merge `default` INTO the result.  We merge defaultUnit
                    // into `result` to always bias to the blockchainDB result.

                    Map<String, SystemClient.Currency> merged = new HashMap<>();
                    for (SystemClient.Currency currency : newCurrencies) {
                        if (currency.getBlockchainId().equals(blockchainId) && currency.getVerified()) {
                            merged.put(currency.getId(), currency);
                        }
                    }

                    func.apply(merged.values());
                } finally {
                    latch.countDown();
                }
            }

            @Override
            public void handleError(QueryError error) {
                try {
                    // On error, use `apps` merged INTO defaults.  We merge into `defaults` to ensure that we get
                    // BTC, BCH, ETH, BRD and that they are correct (don't rely on the App).

                    Map<String, SystemClient.Currency> merged = new HashMap<>();
                    for (SystemClient.Currency currency : applicationCurrencies) {
                        if (currency.getBlockchainId().equals(blockchainId) && currency.getVerified()) {
                            merged.put(currency.getId(), currency);
                        }
                    }

                    func.apply(merged.values());
                } finally {
                    latch.countDown();
                }
            }
        });
    }

    private static void getCurrencies(CountUpAndDownLatch latch,
                                      SystemClient query,
                                      boolean mainnet,
                                      Collection<SystemClient.Currency> applicationCurrencies,
                                      Function<Collection<SystemClient.Currency>, Void> func) {
        latch.countUp();
        query.getCurrencies(null, mainnet, new CompletionHandler<List<SystemClient.Currency>, QueryError>() {
            @Override
            public void handleData(List<SystemClient.Currency> newCurrencies) {
                try {
                    // On success, always merge `default` INTO the result.  We merge defaultUnit
                    // into `result` to always bias to the blockchainDB result.

                    Map<String, SystemClient.Currency> merged = new HashMap<>();
                    for (SystemClient.Currency currency : newCurrencies) {
                        merged.put(currency.getId(), currency);
                    }

                    func.apply(merged.values());
                } finally {
                    latch.countDown();
                }
            }

            @Override
            public void handleError(QueryError error) {
                try {
                    // On error, use `apps` merged INTO defaults.  We merge into `defaults` to ensure that we get
                    // BTC, BCH, ETH, BRD and that they are correct (don't rely on the App).

                    Map<String, SystemClient.Currency> merged = new HashMap<>();
                    for (SystemClient.Currency currency : applicationCurrencies) {
                        merged.put(currency.getId(), currency);
                    }

                    func.apply(merged.values());
                } finally {
                    latch.countDown();
                }
            }
        });
    }

    private static Optional<CurrencyDenomination> findFirstBaseDenomination(List<CurrencyDenomination> denominations) {
        for (CurrencyDenomination denomination : denominations) {
            if (denomination.getDecimals().equals(UnsignedInteger.ZERO)) {
                return Optional.of(denomination);
            }
        }
        return Optional.absent();
    }

    private static List<CurrencyDenomination> findAllNonBaseDenominations(List<CurrencyDenomination> denominations) {
        List<CurrencyDenomination> newDenominations = new ArrayList<>();
        for (CurrencyDenomination denomination : denominations) {
            if (!denomination.getDecimals().equals(UnsignedInteger.ZERO)) {
                newDenominations.add(denomination);
            }
        }
        return newDenominations;
    }

    private static Unit currencyToDefaultBaseUnit(Currency currency) {
        String code = currency.getCode().toLowerCase(Locale.ROOT) + "i";
        String name = currency.getName() + " INT";
        String symb = currency.getCode().toUpperCase(Locale.ROOT) + "I";
        return Unit.create(currency, code, name, symb);
    }

    private static Unit currencyDenominationToBaseUnit(Currency currency,
                                                       CurrencyDenomination denomination) {
        return Unit.create(currency, denomination.getCode(), denomination.getName(), denomination.getSymbol());
    }

    private static List<Unit> currencyDenominationToUnits(Currency currency,
                                                          List<CurrencyDenomination> denominations,
                                                          Unit base) {
        List<Unit> units = new ArrayList<>();
        for (CurrencyDenomination denomination : denominations) {
            units.add(Unit.create(currency, denomination.getCode(), denomination.getName(), denomination.getSymbol(), base,
                      denomination.getDecimals()));
        }
        return units;
    }

    private static Optional<Currency> findCurrency(Map<Currency,
            NetworkAssociation> associations, Blockchain blockchainModel) {
        String code = blockchainModel.getCurrency().toLowerCase(Locale.ROOT);
        for (Currency currency : associations.keySet()) {
            if (code.equals(currency.getUids())) {
                return Optional.of(currency);
            }
        }
        return Optional.absent();
    }

    private static Network findNetwork (List<Network> supportedNetworks, String id) {
        for (Network n : supportedNetworks) {
            if (id.equals(n.getUids()))
                return n;
        }
        return null;
    }

    private static class CountUpAndDownLatch {

        private final Runnable runnable;
        private final AtomicInteger count;

        CountUpAndDownLatch(Runnable runnable) {
            this.count = new AtomicInteger(0);
            this.runnable = runnable;
        }

        void countUp() {
            count.getAndIncrement();
        }

        void countDown() {
            if (0 == count.decrementAndGet()) {
                runnable.run();
            }
        }
    }
}
