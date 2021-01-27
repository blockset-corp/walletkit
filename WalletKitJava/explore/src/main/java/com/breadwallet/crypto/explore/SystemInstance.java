package com.breadwallet.crypto.explore;

import com.breadwallet.crypto.blockchaindb.BlockchainDb;
import com.breadwallet.crypto.events.network.DefaultNetworkEventVisitor;
import com.breadwallet.crypto.events.network.NetworkDeletedEvent;
import com.breadwallet.crypto.events.network.NetworkEvent;
import com.breadwallet.crypto.events.system.DefaultSystemEventVisitor;
import com.breadwallet.crypto.events.system.SystemChangedEvent;
import com.breadwallet.crypto.events.system.SystemCreatedEvent;
import com.breadwallet.crypto.events.system.SystemDeletedEvent;
import com.breadwallet.crypto.events.system.SystemDiscoveredNetworksEvent;
import com.breadwallet.crypto.events.system.SystemEvent;
import com.breadwallet.crypto.events.system.SystemEventVisitor;
import com.breadwallet.crypto.events.system.SystemListener;
import com.breadwallet.crypto.events.system.SystemManagerAddedEvent;
import com.breadwallet.crypto.events.system.SystemNetworkAddedEvent;
import com.breadwallet.crypto.events.transfer.DefaultTransferEventVisitor;
import com.breadwallet.crypto.events.transfer.TranferEvent;
import com.breadwallet.crypto.events.transfer.TransferChangedEvent;
import com.breadwallet.crypto.events.transfer.TransferCreatedEvent;
import com.breadwallet.crypto.events.transfer.TransferDeletedEvent;
import com.breadwallet.crypto.events.wallet.DefaultWalletEventVisitor;
import com.breadwallet.crypto.events.wallet.WalletBalanceUpdatedEvent;
import com.breadwallet.crypto.events.wallet.WalletChangedEvent;
import com.breadwallet.crypto.events.wallet.WalletCreatedEvent;
import com.breadwallet.crypto.events.wallet.WalletDeletedEvent;
import com.breadwallet.crypto.events.wallet.WalletEvent;
import com.breadwallet.crypto.events.wallet.WalletFeeBasisUpdatedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferAddedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferChangedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferDeletedEvent;
import com.breadwallet.crypto.events.wallet.WalletTransferSubmittedEvent;
import com.breadwallet.crypto.events.walletmanager.DefaultWalletManagerEventVisitor;
import com.breadwallet.crypto.events.walletmanager.WalletManagerBlockUpdatedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerChangedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerCreatedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerDeletedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncProgressEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncRecommendedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncStartedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerSyncStoppedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletAddedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletChangedEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerWalletDeletedEvent;
import com.breadwallet.crypto.utility.AccountSpecification;
import com.breadwallet.crypto.utility.BlocksetAccess;
import com.breadwallet.crypto.utility.TestConfiguration;

import com.breadwallet.crypto.Account;
import com.breadwallet.crypto.AddressScheme;
import com.breadwallet.crypto.Network;
import com.breadwallet.crypto.System;
import com.breadwallet.crypto.Transfer;
import com.breadwallet.crypto.Wallet;
import com.breadwallet.crypto.WalletManager;

import java.util.List;
import java.util.ArrayList;
import java.util.Date;
import java.util.UUID;
import java.util.stream.Stream;

import java.nio.charset.StandardCharsets;

import javax.annotation.Nullable;

import okhttp3.OkHttpClient;

public class SystemInstance implements SystemListener {
    static List<SystemInstance> systems = new ArrayList();

    public static void execute (boolean isMainnet, TestConfiguration configuration, int count) {

        // Create the Blockset `query`
        BlocksetAccess blocksetAccess = configuration.getBlocksetAccess();
        BlockchainDb query = BlockchainDb.createForTest(
                new OkHttpClient(),
                blocksetAccess.getToken(),
                blocksetAccess.getBaseURL(),
                null);

        // Find the AccountSpecification from TestConfiguration on `isMainnet`
        List<AccountSpecification> accountSpecs = new ArrayList();
        for (AccountSpecification accountSpec: configuration.getAccountSpecifications()) {
            if (accountSpec.getNetwork().equals (isMainnet ? "mainnet" : "testnet"))
                accountSpecs.add (accountSpec);
        }

        // Create `count` SystemInstances repeatedly using the `accountSpecs`
        for (int i = 0; i < count; i++) {
            AccountSpecification accountSpec = accountSpecs.get (i % accountSpecs.size());
            systems.add (new SystemInstance(isMainnet, accountSpec, query));
        }

        // Run a system for each one.
        for (SystemInstance systemInstance : systems) {
            systemInstance.system.configure();
            systemInstance.system.resume();
        }
    }

    public static void destroy () {
        for (SystemInstance systemInstance : systems) {
            systemInstance.system.pause();
        }
    }


    System system;

    public SystemInstance (boolean isMainnet,
                           AccountSpecification accountSpec,
                           BlockchainDb query) {
        Account account = Account.createFromPhrase(
                accountSpec.getPaperKey().getBytes(StandardCharsets.UTF_8),
                accountSpec.getTimestamp(),
                UUID.randomUUID().toString())
                .orNull();

        assert null != account : "Missed Account";

        // This is where the DB will be saved as a subdirectory based on the `Account`.  Because
        // a multiple `SystemInstances` can be created with the same account; this "path" must
        // be unique for each SystemInstance; otherwise different instances with the same account
        // will overwrite one another.
        String path = "path";

        system = System.create(
                null,
                this,
                account,
                isMainnet,
                path,
                query);
    }

    @java.lang.Override
    public void handleSystemEvent(System system, SystemEvent event) {
        event.accept(new DefaultSystemEventVisitor<Void>() {
                         @Nullable
                         @java.lang.Override
                         public Void visit(SystemCreatedEvent event) {
                             return super.visit(event);
                         }

                         @Nullable
                         @java.lang.Override
                         public Void visit(SystemChangedEvent event) {
                             return super.visit(event);
                         }

                         @Nullable
                         @java.lang.Override
                         public Void visit(SystemDeletedEvent event) {
                             return super.visit(event);
                         }

                         @Nullable
                         @java.lang.Override
                         public Void visit(SystemManagerAddedEvent event) {
                             return super.visit(event);
                         }

                         @Nullable
                         @java.lang.Override
                         public Void visit(SystemNetworkAddedEvent event) {
                             return super.visit(event);
                         }

                         @Nullable
                         @java.lang.Override
                         public Void visit(SystemDiscoveredNetworksEvent event) {
                             return super.visit(event);
                         }
                     }
        );
    }

    @java.lang.Override
    public void handleNetworkEvent(System system, Network network, NetworkEvent event) {
        event.accept(
                new DefaultNetworkEventVisitor<Void>() {
                    @java.lang.Override
                    public Void visit(NetworkDeletedEvent event) {
                        return null;
                    }
                }
        );
    }

    @java.lang.Override
    public void handleTransferEvent(System system, WalletManager manager, Wallet wallet, Transfer transfer, TranferEvent event) {
        event.accept(
                new DefaultTransferEventVisitor<Void>() {
                    @Nullable
                    @java.lang.Override
                    public Void visit(TransferChangedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(TransferCreatedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(TransferDeletedEvent event) {
                        return super.visit(event);
                    }
                }
        );

    }

    @java.lang.Override
    public void handleWalletEvent(System system, WalletManager manager, Wallet wallet, WalletEvent event) {
        event.accept(
                new DefaultWalletEventVisitor<Void>() {
                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletBalanceUpdatedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletChangedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletCreatedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletDeletedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletFeeBasisUpdatedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletTransferAddedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletTransferChangedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletTransferDeletedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletTransferSubmittedEvent event) {
                        return super.visit(event);
                    }
                }
        );
    }

    @java.lang.Override
    public void handleManagerEvent(System system, WalletManager manager, WalletManagerEvent event) {
        event.accept(
                new DefaultWalletManagerEventVisitor<Void>() {
                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerBlockUpdatedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerChangedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerCreatedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerDeletedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerSyncProgressEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerSyncStartedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerSyncStoppedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerSyncRecommendedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerWalletAddedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerWalletChangedEvent event) {
                        return super.visit(event);
                    }

                    @Nullable
                    @java.lang.Override
                    public Void visit(WalletManagerWalletDeletedEvent event) {
                        return super.visit(event);
                    }
                }
        );
    }
}
