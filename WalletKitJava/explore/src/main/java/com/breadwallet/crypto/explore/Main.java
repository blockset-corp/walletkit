package com.breadwallet.crypto.explore;

import com.breadwallet.corecrypto.CryptoApiProvider;
import com.breadwallet.crypto.CryptoApi;
import com.breadwallet.crypto.utility.TestConfiguration;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.Uninterruptibles;

import java.util.Set;
import java.util.concurrent.TimeUnit;
import java.util.logging.ConsoleHandler;
import java.util.logging.Logger;
import java.util.logging.Level;

//import sun.rmi.runtime.Log;

public class Main {

    private final static Logger Log;
    static {
        Log = Logger.getLogger(ExploreConstants.ExploreTag
                               + Main.class.getName());

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
        Log.log(Level.FINER, "Loaded test configuration");

        SystemInstance.execute(prefs.isMainNet, configuration, prefs.systemInstances);
        Log.log(Level.INFO, "Executing");

        // Delay
        Uninterruptibles.sleepUninterruptibly(prefs.runSeconds, TimeUnit.SECONDS);

        // Cleanup
        Log.log(Level.INFO, "Finished, orderly shutdown...");
        SystemInstance.destroy();
        SystemInstance.terminate();

        Log.log(Level.INFO, "done");

        // Someone is creating a non-daemon thread
        // and is holding the application from exiting...
        // Following prints are informational in case this
        // problem can be isolated
        Set<Thread> threadSet = Thread.getAllStackTraces().keySet();
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
        }
        System.exit(0);
    }
}
