package com.breadwallet.crypto;

import com.breadwallet.crypto.utility.TestConfiguration;

import java.io.File;

public class TestConfigurationLoader {
    public static TestConfiguration getTestConfiguration() {
        String filename = java.lang.System.getProperty("user.home") + File.separator + ".brdWalletKitTestsConfig.json";
        return TestConfiguration.loadFrom(filename);
    }
}
