package com.breadwallet.crypto.explore;

import com.breadwallet.crypto.blockchaindb.BlockchainDb;
import com.breadwallet.crypto.events.network.NetworkEvent;
import com.breadwallet.crypto.events.system.SystemEvent;
import com.breadwallet.crypto.events.system.SystemListener;
import com.breadwallet.crypto.events.transfer.TranferEvent;
import com.breadwallet.crypto.events.wallet.WalletEvent;
import com.breadwallet.crypto.events.walletmanager.WalletManagerEvent;
import com.breadwallet.crypto.explore.handler.ExploreNetworkEventHandler;
import com.breadwallet.crypto.explore.handler.ExploreWalletEventHandler;
import com.breadwallet.crypto.explore.handler.ExploreWalletManagerEventHandler;
import com.breadwallet.crypto.utility.AccountSpecification;
import com.breadwallet.crypto.utility.BlocksetAccess;
import com.breadwallet.crypto.utility.TestConfiguration;

import com.breadwallet.crypto.explore.handler.ExploreSystemEventHandler;
import com.breadwallet.crypto.explore.handler.ExploreTransferEventHandler;

import com.breadwallet.crypto.Account;
import com.breadwallet.crypto.Network;
import com.breadwallet.crypto.System;
import com.breadwallet.crypto.Transfer;
import com.breadwallet.crypto.Wallet;
import com.breadwallet.crypto.WalletManager;

import java.io.IOException;
import java.io.PrintStream;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.ArrayList;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.logging.Level;

import java.util.UUID;

import java.nio.charset.StandardCharsets;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.logging.SimpleFormatter;


import okhttp3.OkHttpClient;

public class SystemInstance implements SystemListener {

    /// @brief Containing all system instances instantiated within one session
    static List<SystemInstance>     systems = new ArrayList();

    /// @brief Shared database object for all system instances
    static BlockchainDb             blockset;

    /// @brief All system instances share the okhttpclient impl which is pool threaded
    static OkHttpClient             httpClient;

    /// @brief Identifier for creating unique instance names
    static int                      id = 0;

    public static void execute (
            boolean             isMainnet,
            TestConfiguration   configuration,
            int                 count,
            Logger              output) {

        PrintStream pout = java.lang.System.out;

        // Create the Blockset `query`
        BlocksetAccess blocksetAccess = configuration.getBlocksetAccess();
        httpClient = new OkHttpClient();
        blockset = BlockchainDb.createForTest(
                httpClient,
                blocksetAccess.getToken(),
                blocksetAccess.getBaseURL(),
                null);
        output.log(Level.INFO, "Blockchain DB is created");

        // Find the AccountSpecification from TestConfiguration on `isMainnet`
        List<AccountSpecification> accountSpecs = new ArrayList();
        for (AccountSpecification accountSpec: configuration.getAccountSpecifications()) {
            if (accountSpec.getNetwork().equals (isMainnet ? "mainnet" : "testnet")) {
                accountSpecs.add(accountSpec);
                output.log(Level.INFO, "Added account spec " + accountSpec.getIdentifier());
            }
        }

        // Create `count` SystemInstances repeatedly using the `accountSpecs`
        for (int i = 0; i < count; i++) {
            AccountSpecification accountSpec = accountSpecs.get (i % accountSpecs.size());
            SystemInstance exploreSystemN = new SystemInstance(isMainnet, accountSpec, blockset);
            output.log(Level.INFO, i + ") SystemInstance: " + exploreSystemN.systemIdentifier);
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

    public static void terminate(Logger output) {

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
            output.log(Level.INFO, "Shutdown " + sys.systemIdentifier);
            sys.taskMgmt.shutdown();

            try {
                if (!sys.taskMgmt.awaitTermination(3, TimeUnit.SECONDS))
                    output.log(Level.WARNING, sys.systemIdentifier + " not shutdown");
            } catch(InterruptedException ie) {}
        }

        // Finally all system instances share the single blockset concurrency
        blockset.orderlyShutdown();
    }

    /// @brief A dedicated crypto system per system instance
    System                      system;

    /// @brief Each system instance cryto system runs within a
    ///        single threaded executor
    ScheduledExecutorService    taskMgmt;

    /// @brief A unique identifier for the system instance useful for
    ///        logging and wallet database creation
    String                      systemIdentifier;

    /// @brief Indicates the system instance is on mainnet or testnet
    boolean                     isMainnet;

    /// @brief A single log instance which can be shared with all event
    ///        handlers. This ensures that all activity within one system
    ///        instance can be collected per instance rather than per class
    Logger                      instanceLog;

    /// @brief Create a system instance on the particular indicate net. Each
    ///        of multiple created instances has a unique identifier, creates its
    ///        own file log and uses the universal crypto system to perform its
    ///        operations
    /// @param isMainnet    Indication to use mainnet vs testnet
    /// @param accountSpec  The account specification for this system instance.
    ///                     Somewhat arbitrary for explore application, may be
    ///                     any account to generate activity
    /// @param query        The database into which ? information is stored
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

        // Create a unique identifier for this system instance
        createIdentifier(accountSpec.getIdentifier());

        // Create a shared log utility for all uses within this
        // system instance
        createInstanceLogger();

        // This is where the DB will be saved as a subdirectory based on the `Account`.  Because
        // a multiple `SystemInstances` can be created with the same account; this "path" must
        // be unique for each SystemInstance; otherwise different instances with the same account
        // will overwrite one another.
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

    /// @brief Create a file logger to be used on operations within this instance
    private void createInstanceLogger() {
        // Create a unique system identifier for this system instance. Surely
        // granularity of millisecond should be more than sufficient
        DateTimeFormatter f = DateTimeFormatter.ofPattern("HH_mm_ss_SSS");
        LocalDateTime d = LocalDateTime.now();
        String timestamp = f.format(d);

        instanceLog = Logger.getLogger(ExploreConstants.ExploreTag
                                       + systemIdentifier);
        String logPattern = ExploreConstants.ExploreHome
                            + ExploreConstants.ExploreLogFolder
                            + ExploreConstants.ExploreTag
                            + systemIdentifier
                            + "_"
                            + timestamp
                            + "_rot%u.log";
        try {

            // Simple formatter format string is taken from explorelog.properties file
            FileHandler forFile = new FileHandler(logPattern);
            forFile.setFormatter(new SimpleFormatter());
            instanceLog.addHandler(forFile);
            instanceLog.setLevel(Level.FINE);
            instanceLog.log(Level.INFO, "Log instance started");

        } catch (IOException| SecurityException except) {
            instanceLog.log(Level.WARNING,
                        "File permissions for "
                           + logPattern
                           + " not available");
        }
    }

    /// @brief Create a unique identifier using the account identifier and
    ///        system time.
    private void createIdentifier(String accountId) {

        // Create a unique system identifier for this system instance. Surely
        // granularity of millisecond should be more than sufficient
        DateTimeFormatter f = DateTimeFormatter.ofPattern("HH_mm_ss_SSS");
        LocalDateTime d = LocalDateTime.now();
        systemIdentifier = "instance-"
                + Integer.toString(++id)
                + "-"
                + accountId;
    }

    @java.lang.Override
    public void handleSystemEvent(System system, SystemEvent event) {
        event.accept(new ExploreSystemEventHandler<Void>(system,
                                                        this.isMainnet,
                                                        instanceLog));
    }

    @java.lang.Override
    public void handleNetworkEvent(System system, Network network, NetworkEvent event) {
        event.accept(new ExploreNetworkEventHandler<Void>(network, instanceLog));
    }

    @java.lang.Override
    public void handleTransferEvent(System system, WalletManager manager, Wallet wallet, Transfer transfer, TranferEvent event) {
        event.accept(new ExploreTransferEventHandler<Void>(instanceLog));

    }

    @java.lang.Override
    public void handleWalletEvent(
            System          system,
            WalletManager   manager,
            Wallet          wallet,
            WalletEvent     event) {
        event.accept(new ExploreWalletEventHandler<Void>(wallet, instanceLog));
    }

    @java.lang.Override
    public void handleManagerEvent(System system, WalletManager manager, WalletManagerEvent event) {
        event.accept(new ExploreWalletManagerEventHandler<Void>(instanceLog));
    }
}
