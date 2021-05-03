/*
 * Created by Michael Carrara.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibrary;
import com.google.common.io.Files;
import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Library;
import com.sun.jna.Native;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.nio.charset.StandardCharsets;
import java.util.Calendar;
import java.util.TimeZone;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.*;

public class WKNativeLibraryIT {

    private String paperKey;
    private String uids;
    private File coreDataDir;
    private int epoch;

    @Before
    public void setup() {
        // this is a compromised testnet paperkey
        paperKey = "ginger settle marine tissue robot crane night number ramp coast roast critic";
        uids = "5766b9fa-e9aa-4b6d-9b77-b5f1136e5e96";
        coreDataDir = Files.createTempDir();
        Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
        calendar.set(2017, /* September */ 8, 6);
        epoch = (int) TimeUnit.MILLISECONDS.toSeconds(calendar.getTimeInMillis());

        coreDirClear();
        coreDirCreate();
    }

    @After
    public void teardown() {
        coreDirClear();
    }

    @Test
    public void testLoad() {
        assertNotNull(TestWKNativeLibrary.INSTANCE);
    }

    // Bitcoin

    @Test
    public void testBitcoin() {
        assertEquals(1, TestWKNativeLibrary.INSTANCE.BRRunTests());
    }

    @Test
    public void testBitcoinSyncOne() {
        int success = 0;

        success = TestWKNativeLibrary.INSTANCE.BRRunTestsSync (paperKey, 1, 1);
        assertEquals(1, success);
    }
    
// REFACTOR
//    @Test
//    public void testBitcoinWalletManagerSync () {
//        coreDirClear();
//        int success = 0;
//
//        success = TestCryptoLibrary.INSTANCE.BRRunTestWalletManagerSync (paperKey, coreDataDir.getAbsolutePath(), 1, 1);
//        assertEquals(1, success);
//
//        success = TestCryptoLibrary.INSTANCE.BRRunTestWalletManagerSync (paperKey, coreDataDir.getAbsolutePath(), 1, 1);
//        assertEquals(1, success);
//    }

    @Test
    public void testBitcoinWalletManagerSyncStressBtc() {
        int success;

        coreDirClear();
        success = TestWKNativeLibrary.INSTANCE.BRRunTestWalletManagerSyncStress(
                paperKey,
                coreDataDir.getAbsolutePath(),
                epoch,
                500000,
                1,
                1
        );
        assertEquals(1, success);

        coreDirClear();
        success = TestWKNativeLibrary.INSTANCE.BRRunTestWalletManagerSyncStress(
                paperKey,
                coreDataDir.getAbsolutePath(),
                epoch,
                1500000,
                1,
                0
        );
        assertEquals(1, success);
    }

    @Test
    public void testBitcoinWalletManagerSyncStressBch() {
        int success;

        coreDirClear();
        success = TestWKNativeLibrary.INSTANCE.BRRunTestWalletManagerSyncStress(
                paperKey,
                coreDataDir.getAbsolutePath(),
                epoch,
                500000,
                0,
                1
        );
        assertEquals(1, success);

        coreDirClear();
        success = TestWKNativeLibrary.INSTANCE.BRRunTestWalletManagerSyncStress(
                paperKey,
                coreDataDir.getAbsolutePath(),
                epoch,
                1500000,
                0,
                0
        );
        assertEquals(1, success);
    }

    // Support

    @Test
    public void testBitcoinSupport() {
        assertEquals(1, TestWKNativeLibrary.INSTANCE.BRRunSupTests());
    }

    // Crypto

// REFACTOR
//    @Test
//    public void testCrypto() {
//        TestCryptoLibrary.INSTANCE.runCryptoTests();
//    }

    @Test
    public void testCryptoWithAccountAndNetworkBtc() {
        WKAccount account = WKAccount.createFromPhrase(
                paperKey.getBytes(StandardCharsets.UTF_8),
                UnsignedLong.valueOf(epoch),
                uids
        ).get();

        try {
            WKNetwork network;

            network = createBitcoinNetwork(true, 500000);
            try {
                coreDirClear();
                int success = TestWKNativeLibrary.INSTANCE.runCryptoTestsWithAccountAndNetwork(
                        account,
                        network,
                        coreDataDir.getAbsolutePath());
                assertEquals(1, success);
            } finally {
                network.give();
            }

            network = createBitcoinNetwork(false, 1500000);
            try {
                coreDirClear();
                int success = TestWKNativeLibrary.INSTANCE.runCryptoTestsWithAccountAndNetwork(
                        account,
                        network,
                        coreDataDir.getAbsolutePath());
                assertEquals(1, success);
            } finally {
                network.give();
            }

        } finally {
            account.give();
        }
    }

    @Test
    public void testCryptoWithAccountAndNetworkBch() {
        WKAccount account = WKAccount.createFromPhrase(
                paperKey.getBytes(StandardCharsets.UTF_8),
                UnsignedLong.valueOf(epoch),
                uids
        ).get();

        try {
            WKNetwork network;

            network = createBitcoinCashNetwork(true, 500000);
            try {
                coreDirClear();
                int success = TestWKNativeLibrary.INSTANCE.runCryptoTestsWithAccountAndNetwork(
                        account,
                        network,
                        coreDataDir.getAbsolutePath());
                assertEquals(1, success);
            } finally {
                network.give();
            }

            network = createBitcoinCashNetwork(false, 1500000);
            try {
                coreDirClear();
                int success = TestWKNativeLibrary.INSTANCE.runCryptoTestsWithAccountAndNetwork(
                        account,
                        network,
                        coreDataDir.getAbsolutePath());
                assertEquals(1, success);
            } finally {
                network.give();
            }
        } finally {
            account.give();
        }
    }

    @Test
    public void testCryptoWithAccountAndNetworkEth() {
        WKAccount account = WKAccount.createFromPhrase(
                paperKey.getBytes(StandardCharsets.UTF_8),
                UnsignedLong.valueOf(epoch),
                uids
        ).get();

        try {
            WKNetwork network;

            network = createEthereumNetwork(true, 8000000);
            try {
                coreDirClear();
                int success = TestWKNativeLibrary.INSTANCE.runCryptoTestsWithAccountAndNetwork(
                        account,
                        network,
                        coreDataDir.getAbsolutePath());
                assertEquals(1, success);

                network.give();
            } finally {
                network.give();
            }

            network = createEthereumNetwork(false, 4500000);
            try {
                coreDirClear();
                int success = TestWKNativeLibrary.INSTANCE.runCryptoTestsWithAccountAndNetwork(
                        account,
                        network,
                        coreDataDir.getAbsolutePath());
                assertEquals(1, success);
            } finally {
                network.give();
            }

        } finally {
            account.give();
        }
    }

    // Ethereum

    @Test
    public void testEthereumRlp () {
        TestWKNativeLibrary.INSTANCE.runRlpTests();
    }

    @Test
    public void testEthereumUtil () {
        TestWKNativeLibrary.INSTANCE.runUtilTests();
    }

    @Test
    public void testEthereumEvent () {
        TestWKNativeLibrary.INSTANCE.runEventTests ();
    }

    @Test
    public void testEthereumBase () {
        TestWKNativeLibrary.INSTANCE.runBaseTests();
    }

    @Test
    public void testEthereumBlockChain () {
        TestWKNativeLibrary.INSTANCE.runBcTests();
    }

    @Test
    public void testEthereumContract () {
        TestWKNativeLibrary.INSTANCE.runContractTests ();
    }

    @Test
    public void testEthereumBasics() {
        TestWKNativeLibrary.INSTANCE.runTests(0);
    }

// REFACTOR
//    @Test
//    public void testEWM () {
//        TestCryptoLibrary.INSTANCE.runEWMTests(paperKey, coreDataDir.getAbsolutePath());
//    }

    @Test
    public void testLES () {
        TestWKNativeLibrary.INSTANCE.runLESTests(paperKey);
        TestWKNativeLibrary.INSTANCE.runNodeTests();
    }

    // Ripple

    @Test
    public void testRipple() {
        TestWKNativeLibrary.INSTANCE.runRippleTest();
    }

    // Test Bits

    private void coreDirCreate() {
        coreDataDir.mkdirs();
    }

    private void coreDirClear() {
        deleteRecursively(coreDataDir);
    }

    private static void deleteRecursively (File file) {
        if (file.isDirectory()) {
            for (File child : file.listFiles()) {
                deleteRecursively(child);
            }
        }
        file.delete();
    }

    private static WKNetwork createBitcoinNetwork(boolean isMainnet, long blockHeight) {
        WKCurrency btc = null;
        WKUnit satoshis = null;
        WKUnit bitcoins = null;
        WKAmount factor = null;
        WKNetworkFee fee = null;
        WKNetwork network = null;

        try {

            network = WKNetwork.findBuiltin("bitcoin-" + (isMainnet ? "mainnet" : "testnet")).get();

            btc = WKCurrency
                    .create("bitcoin", "bitcoin", "btc", "native", null);

            satoshis = WKUnit
                    .createAsBase(btc, "sat", "satoshi", "SAT");

            bitcoins = WKUnit
                    .create(btc, "btc", "bitcoin", "B", satoshis, UnsignedInteger.valueOf(8));

            factor = WKAmount
                    .create(1000, satoshis);

            fee = WKNetworkFee
                    .create(UnsignedLong.valueOf(30 * 1000), factor, satoshis);

            network.setHeight(UnsignedLong.valueOf(blockHeight));

//            network.setCurrency(btc);
            network.addCurrency(btc, satoshis, bitcoins);

            network.addCurrencyUnit(btc, satoshis);
            network.addCurrencyUnit(btc, bitcoins);

            network.addFee(fee);

            return network.take();

        } finally {
            if (null != btc) btc.give();
            if (null != satoshis) satoshis.give();
            if (null != bitcoins) bitcoins.give();
            if (null != factor) factor.give();
            if (null != fee) fee.give();
            if (null != network) network.give();
        }
    }

    private static WKNetwork createBitcoinCashNetwork(boolean isMainnet, long blockHeight) {
        WKCurrency btc = null;
        WKUnit satoshis = null;
        WKUnit bitcoins = null;
        WKAmount factor = null;
        WKNetworkFee fee = null;
        WKNetwork network = null;

        try {

            network = WKNetwork.findBuiltin("bitcoincash-" + (isMainnet ? "mainnet" : "testnet")).get();

            btc = WKCurrency
                    .create("bitcoin-cash", "bitcoin cash", "bch", "native", null);

            satoshis = WKUnit
                    .createAsBase(btc, "sat", "satoshi", "SAT");

            bitcoins = WKUnit
                    .create(btc, "btc", "bitcoin", "B", satoshis, UnsignedInteger.valueOf(8));

            factor = WKAmount
                    .create(1000, satoshis);

            fee = WKNetworkFee
                    .create(UnsignedLong.valueOf(30 * 1000), factor, satoshis);

            network.setHeight(UnsignedLong.valueOf(blockHeight));

//            network.setCurrency(btc);
            network.addCurrency(btc, satoshis, bitcoins);

            network.addCurrencyUnit(btc, satoshis);
            network.addCurrencyUnit(btc, bitcoins);

            network.addFee(fee);

            return network.take();

        } finally {
            if (null != btc) btc.give();
            if (null != satoshis) satoshis.give();
            if (null != bitcoins) bitcoins.give();
            if (null != factor) factor.give();
            if (null != fee) fee.give();
            if (null != network) network.give();
        }
    }

    private static WKNetwork createEthereumNetwork(boolean isMainnet, long blockHeight) {
        WKCurrency eth = null;
        WKUnit wei = null;
        WKUnit gwei = null;
        WKUnit ether = null;
        WKAmount factor = null;
        WKNetworkFee fee = null;
        WKNetwork network = null;

        try {

            network = WKNetwork.findBuiltin("ethereum-" + (isMainnet ? "mainnet" : "ropsten")).get();

            eth = WKCurrency
                    .create("ethereum", "ethereum", "eth", "native", null);

            wei = WKUnit
                    .createAsBase(eth, "wei", "wei", "wei");

            gwei = WKUnit
                    .create(eth, "gwei", "gwei", "gwei", wei, UnsignedInteger.valueOf(9));

            ether = WKUnit
                    .create(eth, "ether", "eth", "E", wei, UnsignedInteger.valueOf(18));

            factor = WKAmount
                    .create(2.0, gwei);

            fee = WKNetworkFee
                    .create(UnsignedLong.valueOf(1000), factor, gwei);

            network.setHeight(UnsignedLong.valueOf(blockHeight));

//            network.setCurrency(eth);

            network.addCurrency(eth, wei, ether);

            network.addCurrencyUnit(eth, wei);
            network.addCurrencyUnit(eth, gwei);
            network.addCurrencyUnit(eth, ether);

            network.addFee(fee);

            return network.take();

        } finally {
            if (null != eth) eth.give();
            if (null != wei) wei.give();
            if (null != gwei) gwei.give();
            if (null != ether) ether.give();
            if (null != factor) factor.give();
            if (null != fee) fee.give();
            if (null != network) network.give();
        }
    }

    public interface TestWKNativeLibrary extends Library {
        TestWKNativeLibrary INSTANCE = Native.load(WKNativeLibrary.LIBRARY_NAME, TestWKNativeLibrary.class);

        int BRRunTests();
        int BRRunSupTests();
        int BRRunTestsSync (String paperKey, int isBTC, int isMainnet);
        int BRRunTestWalletManagerSync (String paperKey, String storagePath, int isBTC, int isMainnet);
        int BRRunTestWalletManagerSyncStress(String paperKey, String storagePath, int epoch, long blockHeight, int isBTC, int isMainnet);

        void runCryptoTests();
        int runCryptoTestsWithAccountAndNetwork(WKAccount account, WKNetwork network, String storagePath);

        void runUtilTests();
        void runRlpTests();
        void runEventTests();
        void runBaseTests();
        void runBcTests();
        void runContractTests();
        void runTests(int reallySend);
        void runEWMTests(String paperKey, String storagePath);
        void runLESTests(String paperKey);
        void runNodeTests();

        void runRippleTest();
    }
}
