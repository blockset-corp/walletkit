/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import android.content.Context;

import androidx.test.InstrumentationRegistry;

import com.blockset.walletkit.utility.TestConfiguration;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class TestConfigurationLoader {
    public static TestConfiguration getTestConfiguration() {
        final Context context = InstrumentationRegistry.getContext();
        try (final InputStream inputStream = context.getAssets().open("WalletKitTestsConfig.json");
             final BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {
            return TestConfiguration.loadFrom(reader);
        }
        catch (IOException e) {
            throw new RuntimeException (e);
        }
    }
}

// Another implementation, directly accessing .json
//public class TestConfigurationLoader {
//    public static TestConfiguration getTestConfiguration() {
//        String filename = java.lang.System.getProperty("user.home") + File.separator + ".brdWalletKitTestsConfig.json";
//        return TestConfiguration.loadFrom(filename);
//    }
//}
