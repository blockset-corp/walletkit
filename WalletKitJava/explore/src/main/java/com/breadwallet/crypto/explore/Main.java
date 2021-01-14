package com.breadwallet.crypto.explore;

import com.breadwallet.corecrypto.CryptoApiProvider;
import com.breadwallet.crypto.CryptoApi;
import com.breadwallet.crypto.utility.TestConfiguration;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.Uninterruptibles;

import java.util.concurrent.TimeUnit;

public class Main {
    private static final int SYSTEM_COUNT = 100;

    static {
        try {
            CryptoApi.initialize(CryptoApiProvider.getInstance());
        } catch (IllegalStateException e) {
            // ignore, good to go
        }
    }

    public static void main(String[] args) {
        TestConfiguration configuration = TestConfiguration.loadFrom("foo");

        SystemInstance.execute(true, configuration, SYSTEM_COUNT);

        // delay
        Uninterruptibles.sleepUninterruptibly(60, TimeUnit.SECONDS);

//        SystemInstance.destroy();;
//        java.lang.System.out.println ("done");
    }
}
