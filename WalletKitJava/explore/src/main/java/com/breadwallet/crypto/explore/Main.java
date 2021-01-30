package com.breadwallet.crypto.explore;

import com.breadwallet.corecrypto.CryptoApiProvider;
import com.breadwallet.crypto.CryptoApi;
import com.breadwallet.crypto.utility.TestConfiguration;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.Uninterruptibles;

import java.util.concurrent.TimeUnit;

public class Main {

    static {
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
        System.out.println("Launching test configuration ("
                           + (prefs.isMainNet ? "mainnet" : "testnet")
                           + ", instances: "
                           + prefs.systemInstances
                           + ", config: "
                           + prefs.walletConfig
                           + ")");

        TestConfiguration configuration = TestConfiguration.loadFrom(prefs.walletConfig);
        System.out.println("Loaded test configuration");

        try {
            Thread.sleep(5000);
        } catch(InterruptedException ie) {}

        SystemInstance.execute(prefs.isMainNet, configuration, prefs.systemInstances);
        System.out.println("Executed");

        // delay
        System.out.println("Sleep uninteruptibly");
        Uninterruptibles.sleepUninterruptibly(60, TimeUnit.SECONDS);
        System.out.println("Sleep done.. exit");

//        SystemInstance.destroy();;
//        java.lang.System.out.println ("done");
    }
}
