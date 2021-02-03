package com.breadwallet.crypto.explore;

import com.breadwallet.corecrypto.CryptoApiProvider;
import com.breadwallet.crypto.CryptoApi;
import com.breadwallet.crypto.utility.TestConfiguration;

import com.google.common.util.concurrent.Uninterruptibles;

import java.io.IOException;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.concurrent.TimeUnit;
import java.util.logging.FileHandler;
import java.util.logging.Logger;
import java.util.logging.Level;
import java.util.logging.SimpleFormatter;

/// @brief Explore application main. Runs with number of instances,
///        time to run and config file (see ExpArg)
public class Main {

    private final static Logger     Log;

    static {
        Log = Logger.getLogger(ExploreConstants.ExploreTag
                               + Main.class.getName());

        // Create a unique system identifier for this system instance. Surely
        // granularity of millisecond should be more than sufficient
        DateTimeFormatter f = DateTimeFormatter.ofPattern("HH_mm_ss_SSS");
        LocalDateTime d = LocalDateTime.now();
        String timestamp = f.format(d);

        String logPattern = ExploreConstants.ExploreHome
                            + ExploreConstants.ExploreLogFolder
                            + ExploreConstants.ExploreTag
                            + timestamp
                            + "_Main_rot%u.log";
        try {
            // Simple formatter output format is most conveniently described
            // in the explorelog.properties file.
            FileHandler forFile = new FileHandler(logPattern);
            forFile.setFormatter(new SimpleFormatter());
            Log.addHandler(forFile);
        } catch (IOException | SecurityException except) {
            Log.log(Level.WARNING,
                    "File permissions for "
                    + logPattern
                    + " not available");
        }

        // Initialize crypto
        try {
            CryptoApi.initialize(CryptoApiProvider.getInstance());
        } catch (IllegalStateException e) {
            // ignore, good to go
        }
    }

    public static void main(String[] args) {

        ExpArg prefs = new ExpArg(args);
        if (!prefs.Ok())
        {
            prefs.Usage(Main.class.getCanonicalName());
            return;
        }

        // With config..
        Log.log(Level.INFO,
                "Launching test configuration (on "
                + (prefs.isMainNet ? "mainnet" : "testnet")
                + " for "
                + prefs.runSeconds
                + " secs, instances: "
                + prefs.systemInstances
                + ", config: "
                + prefs.walletConfig
                + ")");

        TestConfiguration configuration = TestConfiguration.loadFrom(prefs.walletConfig);
        Log.log(Level.INFO, "Loaded test configuration");

        SystemInstance.execute(prefs.isMainNet,
                               configuration,
                               prefs.systemInstances,
                               Log);
        Log.log(Level.INFO, "Executing");

        // Delay
        Uninterruptibles.sleepUninterruptibly(prefs.runSeconds, TimeUnit.SECONDS);

        // Cleanup
        Log.log(Level.INFO, "Finished, orderly shutdown...");
        SystemInstance.destroy();
        SystemInstance.terminate(Log);

        Log.log(Level.INFO, "done");

        // Someone is creating a non-daemon thread
        // and is holding the application from exiting...
        // Following prints are informational in case this
        // problem can be isolated
        /*Set<Thread> threadSet = Thread.getAllStackTraces().keySet();
        for (Thread th: threadSet) {
            if (!th.isDaemon()) {
                System.out.println(th.getId() + " " + th.getName());
                StackTraceElement[] st = th.getStackTrace();
                for (StackTraceElement elem : st) {
                    System.out.println("   "
                                       + elem.getClassName() + " "
                                       + elem.getFileName() + " "
                                       + elem.getMethodName() + " "
                                       + elem.getLineNumber());
                }
            }
        }*/
        System.exit(0);
    }
}
