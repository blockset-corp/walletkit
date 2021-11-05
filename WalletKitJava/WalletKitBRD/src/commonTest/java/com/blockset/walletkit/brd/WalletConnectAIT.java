/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;

import java.io.File;
import java.nio.charset.StandardCharsets;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.TimeUnit;

import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.brd.HelpersAIT.RecordingSystemListener;
import com.blockset.walletkit.AddressScheme;
import com.blockset.walletkit.Coder;
import com.blockset.walletkit.Cipher;
import com.blockset.walletkit.Currency;
import com.blockset.walletkit.Key;
import com.blockset.walletkit.Network;
import com.blockset.walletkit.NetworkType;
import com.blockset.walletkit.System;
import com.blockset.walletkit.Unit;
import com.blockset.walletkit.Wallet;
import com.blockset.walletkit.WalletManager;
import com.blockset.walletkit.WalletManagerMode;
import com.blockset.walletkit.WalletState;
import com.blockset.walletkit.brd.systemclient.BlocksetSystemClient;
import com.blockset.walletkit.errors.WalletConnectorError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;
import com.google.common.util.concurrent.Uninterruptibles;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import java.util.logging.Level;
import java.util.logging.Logger;

public class WalletConnectAIT {

    static private final Logger Log = Logger.getLogger(WalletConnectAIT.class.getName());

    private static File         coreDataDir;
    private static System       system;
    private com.blockset.walletkit.WalletConnector wc;

    @BeforeClass
    public static void setup() {
        HelpersAIT.registerCryptoApiProvider();

        coreDataDir = HelpersAIT.generateCoreDataDir();
        HelpersAIT.createOrOverwriteDirectory(coreDataDir);

        BlocksetSystemClient query = HelpersAIT.createDefaultBlockchainDbWithoutToken();

        system = HelpersAIT.createAndConfigureSystemWithBlockchainDbAndCurrencies(coreDataDir, query, null);
        Log.log(Level.FINE, String.format("WalletConnectTest: Got %d networks", system.getNetworks().size()));
        assertTrue(system.getNetworks().size() >= 1);
    }

    @AfterClass
    public static void teardown() {
        HelpersAIT.deleteFile(coreDataDir);
    }

    @Test
    public void testCreateWalletConnectorOnEth() {

        Network network = null;
        for (Network n: system.getNetworks()) {
            if (NetworkType.ETH == n.getType() && !n.isMainnet())
                network = n;
        }
        Log.log(Level.FINE, String.format("WalletConnectTest: Have the ETH network? %s", (network != null ? "YES" : "NO")));
        assertNotNull(network);

        List<? extends WalletManager> mgrs = system.getWalletManagers();
        Log.log(Level.FINE, String.format("WalletConnectTest: There are %d managers already", mgrs.size()));
        AddressScheme as = network.getDefaultAddressScheme();
        boolean success = system.createWalletManager(network,
                                                     WalletManagerMode.API_ONLY,
                                                     as,
                                                     Collections.emptySet());
        assertTrue("Created ETH wallet manager", success);

        WalletManager ethMgr = system.getWalletManagers().get(0);
        Log.log(Level.FINE, String.format("WalletConnectTest: %s", ethMgr.toString()));

        ethMgr.createWalletConnector(new CompletionHandler<com.blockset.walletkit.WalletConnector, WalletConnectorError>() {

            @Override
            public void handleData(com.blockset.walletkit.WalletConnector data) {
                wc = data;
            }

            @Override
            public void handleError(WalletConnectorError error) {
                assertTrue("No wallet connector! " + error.toString(), false);
            }
        });
    }
}
