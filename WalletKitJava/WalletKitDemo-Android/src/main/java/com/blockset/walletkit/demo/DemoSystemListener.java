/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.demo;

import android.support.annotation.Nullable;

import com.blockset.walletkit.Account;
import com.blockset.walletkit.AddressScheme;
import com.blockset.walletkit.Coder;
import com.blockset.walletkit.Currency;
import com.blockset.walletkit.Network;
import com.blockset.walletkit.NetworkType;
import com.blockset.walletkit.System;
import com.blockset.walletkit.Transfer;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.WalletManager;
import com.blockset.walletkit.WalletManagerMode;
import com.blockset.walletkit.blockchaindb.models.bdb.HederaAccount;
import com.blockset.walletkit.errors.AccountInitializationError;
import com.blockset.walletkit.errors.AccountInitializationMultipleHederaAccountsError;
import com.blockset.walletkit.events.network.NetworkEvent;
import com.blockset.walletkit.events.system.DefaultSystemEventVisitor;
import com.blockset.walletkit.events.system.SystemDiscoveredNetworksEvent;
import com.blockset.walletkit.events.system.SystemEvent;
import com.blockset.walletkit.events.system.SystemListener;
import com.blockset.walletkit.events.system.SystemManagerAddedEvent;
import com.blockset.walletkit.events.system.SystemNetworkAddedEvent;
import com.blockset.walletkit.events.transfer.TranferEvent;
import com.blockset.walletkit.events.wallet.DefaultWalletEventVisitor;
import com.blockset.walletkit.events.wallet.WalletCreatedEvent;
import com.blockset.walletkit.events.wallet.WalletEvent;
import com.blockset.walletkit.events.walletmanager.WalletManagerEvent;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

import static com.google.common.base.Preconditions.checkState;

public class DemoSystemListener implements SystemListener {

    private static final Logger Log = Logger.getLogger(DemoSystemListener.class.getName());

    private final WalletManagerMode preferredMode;
    private final boolean isMainnet;
    private final List<String> currencyCodesNeeded;

    /* package */
    DemoSystemListener(WalletManagerMode preferredMode, boolean isMainnet, List<String> currencyCodesNeeded) {
        this.preferredMode = preferredMode;
        this.isMainnet = isMainnet;
        this.currencyCodesNeeded = new ArrayList<>(currencyCodesNeeded);
    }

    // SystemListener Handlers

    @Override
    public void handleSystemEvent(System system, SystemEvent event) {
        ApplicationExecutors.runOnBlockingExecutor(() -> {
            Log.log(Level.FINE, String.format("System: %s", event));

            event.accept(new DefaultSystemEventVisitor<Void>() {
                @Nullable
                @Override
                public Void visit(SystemNetworkAddedEvent event) {
                    createWalletManager(system, event.getNetwork());
                    return null;
                }

                @Nullable
                @Override
                public Void visit(SystemManagerAddedEvent event) {
                    connectWalletManager(event.getWalletManager());
                    return null;
                }

                @Nullable
                @Override
                public Void visit(SystemDiscoveredNetworksEvent event) {
                    logDiscoveredCurrencies(event.getNetworks());
                    return null;
                }
            });
        });
    }

    @Override
    public void handleNetworkEvent(System system, Network network, NetworkEvent event) {
        ApplicationExecutors.runOnBlockingExecutor(() -> {
            Log.log(Level.FINE, String.format("Network: %s", event));
        });
    }

    @Override
    public void handleManagerEvent(System system, WalletManager manager, WalletManagerEvent event) {
        ApplicationExecutors.runOnBlockingExecutor(() -> {
            Log.log(Level.FINE, String.format("Manager (%s): %s", manager.getName(), event));
        });
    }

    @Override
    public void handleWalletEvent(System system, WalletManager manager, Wallet wallet, WalletEvent event) {
        ApplicationExecutors.runOnBlockingExecutor(() -> {
            Log.log(Level.FINE, String.format("Wallet (%s:%s): %s", manager.getName(), wallet.getName(), event));

            event.accept(new DefaultWalletEventVisitor<Void>() {
                @Nullable
                @Override
                public Void visit(WalletCreatedEvent event) {
                    logWalletAddresses(wallet);
                    return null;
                }
            });
        });
    }

    @Override
    public void handleTransferEvent(System system, WalletManager manager, Wallet wallet, Transfer transfer, TranferEvent event) {
        ApplicationExecutors.runOnBlockingExecutor(() -> {
            Log.log(Level.FINE, String.format("Transfer (%s:%s): %s", manager.getName(), wallet.getName(), event));
        });
    }

    // Misc.

    private void createWalletManager(System system, Network network) {
        boolean isNetworkNeeded = false;
        for (String currencyCode : currencyCodesNeeded) {
            Optional<? extends Currency> currency = network.getCurrencyByCode(currencyCode);
            if (currency.isPresent()) {
                isNetworkNeeded = true;
                break;
            }
        }

        if (isMainnet == network.isMainnet() && isNetworkNeeded) {
            WalletManagerMode mode = network.supportsWalletManagerMode(preferredMode) ?
                    preferredMode : network.getDefaultWalletManagerMode();

            AddressScheme addressScheme = network.getDefaultAddressScheme();
            Log.log(Level.FINE, String.format("Creating %s WalletManager with %s and %s", network, mode, addressScheme));
            boolean success = system.createWalletManager(network, mode, addressScheme, Collections.emptySet());
            if (!success) {
                Account account = system.getAccount();

                system.wipe(network);
                if (!system.accountIsInitialized(account, network)) {

                    checkState (network.getType() == NetworkType.HBAR);
                    List<byte[]> serializationData = new ArrayList<>();

                    system.accountInitialize(system.getAccount(), network, true, new CompletionHandler<byte[], AccountInitializationError>() {
                        @Override
                        public void handleData(byte[] data) {
                            serializationData.add(data);
                            createWalletManagerIfAppropriate(serializationData, system, network, mode, addressScheme);
                        }

                        @Override
                        public void handleError(AccountInitializationError error) {
                            if (error instanceof AccountInitializationMultipleHederaAccountsError) {
                                List<HederaAccount> accounts = ((AccountInitializationMultipleHederaAccountsError) error).getAccounts();

                                // Find the account with the largest balance
                                Collections.sort (accounts, HederaAccount.BALANCE_COMPARATOR.reversed());

                                system.accountInitializeUsingHedera (system.getAccount(), network, accounts.get(0))
                                        .transform((bytes) -> { serializationData.add(bytes); return true; });
                            }
                            createWalletManagerIfAppropriate(serializationData, system, network, mode, addressScheme);
                        }
                    });

                }
            }
        }
    }

    private void createWalletManagerIfAppropriate (List<byte[]> serializationData,
                                                   System system,
                                                   Network network,
                                                   WalletManagerMode mode,
                                                   AddressScheme addressScheme) {
        if (!serializationData.isEmpty()) {
            Coder hexCoder = Coder.createForAlgorithm(Coder.Algorithm.HEX);

            // Normally, save the `serializationData`; but not here - DEMO-SPECIFIC
            Log.log(Level.INFO, String.format("Account: SerializationData: %s",
                    hexCoder.encode(serializationData.get(0))));

            checkState(system.createWalletManager(network, mode, addressScheme, Collections.emptySet()));
        }
    }

    private void connectWalletManager(WalletManager walletManager) {
        walletManager.connect(null);
    }

    private void logWalletAddresses(Wallet wallet) {
        Log.log(Level.FINE, String.format("Wallet (target) addresses: %s", wallet.getTarget()));
    }

    private void logDiscoveredCurrencies(List<Network> networks) {
        for (Network network: networks) {
            for (Currency currency: network.getCurrencies()) {
                Log.log(Level.FINE, String.format("Discovered: %s for %s", currency.getCode(), network.getName()));
            }
        };
    }
}
