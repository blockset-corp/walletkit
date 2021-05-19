package com.breadwallet.corecrypto;

import android.content.Context;
import androidx.test.InstrumentationRegistry;

import com.breadwallet.crypto.utility.TestConfiguration;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class TestConfigurationLoader {
    public static TestConfiguration getTestConfiguration() {
        final Context context = InstrumentationRegistry.getContext();
        try (final InputStream inputStream = context.getAssets().open("WalletKitTestConfig.json");
             final BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream))) {
            return TestConfiguration.loadFrom(reader);
        }
        catch (IOException e) {
            throw new RuntimeException (e);
        }
    }
}
