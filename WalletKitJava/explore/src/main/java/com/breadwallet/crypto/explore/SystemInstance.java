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
//import com.breadwallet.crypto.events.system.SystemEventVisitor;
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
import com.breadwallet.crypto.explore.handler.ExploreNetworkEventHandler;
import com.breadwallet.crypto.explore.handler.ExploreWalletEventHandler;
import com.breadwallet.crypto.explore.handler.ExploreWalletManagerEventHandler;
import com.breadwallet.crypto.utility.AccountSpecification;
import com.breadwallet.crypto.utility.BlocksetAccess;
import com.breadwallet.crypto.utility.TestConfiguration;

import com.breadwallet.crypto.explore.handler.ExploreSystemEventHandler;
import com.breadwallet.crypto.explore.handler.ExploreTransferEventHandler;

import com.breadwallet.crypto.Account;
//import com.breadwallet.crypto.AddressScheme;
import com.breadwallet.crypto.Network;
import com.breadwallet.crypto.System;
import com.breadwallet.crypto.Transfer;
import com.breadwallet.crypto.Wallet;
import com.breadwallet.crypto.WalletManager;

import java.io.PrintStream;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.ArrayList;
import java.util.logging.ConsoleHandler;
import java.util.logging.Logger;
import java.util.logging.Level;

//import java.util.Date;
import java.util.UUID;
//import java.util.stream.Stream;

import java.nio.charset.StandardCharsets;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import javax.annotation.Nullable;

import okhttp3.OkHttpClient;

public class SystemInstance implements SystemListener {
    static List<SystemInstance>     systems = new ArrayList();
    static BlockchainDb             blockset;
    static OkHttpClient             httpClient;
    static int                      id = 0;

    private final static Logger Log;

    static {
        Log = Logger.getLogger(ExploreConstants.ExploreTag
                               + SystemInstance.class.getName());
    }

    public static void execute (boolean isMainnet, TestConfiguration configuration, int count) {

        PrintStream pout = java.lang.System.out;

        // Create the Blockset `query`
        BlocksetAccess blocksetAccess = configuration.getBlocksetAccess();
        httpClient = new OkHttpClient();
        blockset = BlockchainDb.createForTest(
                httpClient,
                blocksetAccess.getToken(),
                blocksetAccess.getBaseURL(),
                null);
        Log.log(Level.INFO, "Blockchain DB is created");

        // Find the AccountSpecification from TestConfiguration on `isMainnet`
        List<AccountSpecification> accountSpecs = new ArrayList();
        for (AccountSpecification accountSpec: configuration.getAccountSpecifications()) {
            if (accountSpec.getNetwork().equals (isMainnet ? "mainnet" : "testnet")) {
                accountSpecs.add(accountSpec);
                Log.log(Level.INFO, "Added account spec " + accountSpec.getIdentifier());
            }
        }

        // Create `count` SystemInstances repeatedly using the `accountSpecs`
        for (int i = 0; i < count; i++) {
            AccountSpecification accountSpec = accountSpecs.get (i % accountSpecs.size());
            SystemInstance exploreSystemN = new SystemInstance(isMainnet, accountSpec, blockset);
            Log.log(Level.INFO, i + ") SystemInstance: " + exploreSystemN.systemIdentifier);
            systems.add (exploreSystemN);
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

    public static void terminate() {

        // Shutdown okhttpclient thread pools
        ExecutorService httpExe = httpClient.dispatcher().executorService();
        httpExe.shutdown();
        try {
            httpExe.awaitTermination(3, TimeUnit.SECONDS);
        } catch(InterruptedException ie) {}
        httpClient.connectionPool().evictAll();
        if (httpClient.cache()!= null) {
            try {
                httpClient.cache().close();
            } catch(java.io.IOException ioe) {}
        }

        // End concurrency for each of the systems instances created
        for (SystemInstance sys : systems) {
            Log.log(Level.INFO, "Shutdown " + sys.systemIdentifier);
            sys.taskMgmt.shutdown();

            try {
                if (!sys.taskMgmt.awaitTermination(3, TimeUnit.SECONDS))
                    Log.log(Level.WARNING, sys.systemIdentifier + " not shutdown");
            } catch(InterruptedException ie) {}
        }

        // Finally all system instances share the single blockset concurrency
        blockset.orderlyShutdown();
    }

    System                      system;
    ScheduledExecutorService    taskMgmt;
    String                      systemIdentifier;
    boolean                     isMainnet;

    /// @brief System event handler for network events and wallet management
    ExploreSystemEventHandler walletHandling;

    public SystemInstance (boolean isMainnet,
                           AccountSpecification accountSpec,
                           BlockchainDb query) {
        Account account = Account.createFromPhrase(
                accountSpec.getPaperKey().getBytes(StandardCharsets.UTF_8),
                accountSpec.getTimestamp(),
                UUID.randomUUID().toString())
                .orNull();

        assert null != account : "Missed Account";

        this.isMainnet = isMainnet;

        // This is where the DB will be saved as a subdirectory based on the `Account`.  Because
        // a multiple `SystemInstances` can be created with the same account; this "path" must
        // be unique for each SystemInstance; otherwise different instances with the same account
        // will overwrite one another.
        DateTimeFormatter f = DateTimeFormatter.ofPattern("yyyy_MM_dd_HH_mm_ss_SSS");
        LocalDateTime d = LocalDateTime.now();
        systemIdentifier = "system_"
                + f.format(d)
                + "_"
                + Integer.toString(++id)
                + "-"
                + accountSpec.getIdentifier();
        String path = "dbs/" + systemIdentifier;

        // Create single threaded concurrency
        taskMgmt = Executors.newSingleThreadScheduledExecutor();
        system = System.create(
                taskMgmt,
                this,
                account,
                isMainnet,
                path,
                query);
    }


    @java.lang.Override
    public void handleSystemEvent(System system, SystemEvent event) {
        event.accept(new ExploreSystemEventHandler<Void>(system, this.isMainnet));
    }

    @java.lang.Override
    public void handleNetworkEvent(System system, Network network, NetworkEvent event) {
        event.accept(new ExploreNetworkEventHandler<Void>());
    }

    @java.lang.Override
    public void handleTransferEvent(System system, WalletManager manager, Wallet wallet, Transfer transfer, TranferEvent event) {
        event.accept(new ExploreTransferEventHandler<Void>());

    }

    @java.lang.Override
    public void handleWalletEvent(System system, WalletManager manager, Wallet wallet, WalletEvent event) {
        event.accept(new ExploreWalletEventHandler<Void>());
    }

    @java.lang.Override
    public void handleManagerEvent(System system, WalletManager manager, WalletManagerEvent event) {
        event.accept(new ExploreWalletManagerEventHandler<Void>());
    }
}
