/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.utility.TestConfiguration;
import java.io.File;

public class TestConfigurationLoader {
    public static TestConfiguration getTestConfiguration() {
        String filename = java.lang.System.getProperty("user.home") + File.separator + ".brdWalletKitTestsConfig.json";
        return TestConfiguration.loadFrom(filename);
    }
}
