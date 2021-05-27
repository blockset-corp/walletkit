/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corecrypto;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import java.io.File;
import java.nio.charset.StandardCharsets;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.TimeUnit;

import com.breadwallet.corecrypto.HelpersAIT.RecordingSystemListener;
import com.breadwallet.crypto.AddressScheme;
import com.breadwallet.crypto.Coder;
import com.breadwallet.crypto.Cipher;
import com.breadwallet.crypto.Currency;
import com.breadwallet.crypto.Key;
import com.breadwallet.crypto.Network;
import com.breadwallet.crypto.NetworkType;
import com.breadwallet.crypto.System;
import com.breadwallet.crypto.Unit;
import com.breadwallet.crypto.Wallet;
import com.breadwallet.crypto.WalletManager;
import com.breadwallet.crypto.WalletManagerMode;
import com.breadwallet.crypto.WalletState;
import com.breadwallet.crypto.blockchaindb.BlockchainDb;
import com.breadwallet.crypto.errors.MigrateBlockError;
import com.breadwallet.crypto.errors.MigrateError;
import com.breadwallet.crypto.errors.MigrateTransactionError;
import com.breadwallet.crypto.migration.BlockBlob;
import com.breadwallet.crypto.migration.PeerBlob;
import com.breadwallet.crypto.migration.TransactionBlob;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedLong;
import com.google.common.util.concurrent.Uninterruptibles;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

public class SystemAIT {

    private File coreDataDir;

    @Before
    public void setup() {
        HelpersAIT.registerCryptoApiProvider();

        coreDataDir = HelpersAIT.generateCoreDataDir();
        HelpersAIT.createOrOverwriteDirectory(coreDataDir);
    }

    @After
    public void teardown() {
        HelpersAIT.deleteFile(coreDataDir);
    }

    // @Test
    public void ignoreTestSystemAppCurrencies() {
        // Create a query that fails (no authentication)
        BlockchainDb query = HelpersAIT.createDefaultBlockchainDbWithoutToken();

        // We need the UIDS to contain a valid ETH address BUT not be a default.  Since we are
        // using `isMainnet = false` use a mainnet address.
        List<com.breadwallet.crypto.blockchaindb.models.bdb.Currency> currencies = Collections.singletonList(
                System.asBlockChainDBModelCurrency(
                        "ethereum-ropsten" + ":" + Blockchains.ADDRESS_BRD_MAINNET,
                        "FOO Token",
                        "foo",
                        "ERC20",
                        UnsignedInteger.valueOf(10)
                ).get()
        );

        System system = HelpersAIT.createAndConfigureSystemWithBlockchainDbAndCurrencies(coreDataDir, query, currencies);
        assertTrue(system.getNetworks().size() >= 1);

        Network network = null;
        for (Network n: system.getNetworks()) if (NetworkType.ETH == n.getType() && !n.isMainnet()) network = n;
        assertNotNull(network);

        assertTrue(network.getCurrencyByCode("eth").isPresent());
        // assertTrue(network.getCurrencyByCode("foo").isPresent());
        // assertFalse(network.getCurrencyByCode("FOO").isPresent());

        if (!network.getCurrencyByCode("foo").isPresent()) {
            assertTrue(false);
            return;
        }

        Currency fooCurrency = network.getCurrencyByCode("foo").get();
        assertEquals("erc20", fooCurrency.getType());

        Optional<? extends Unit> fooDefault = network.defaultUnitFor(fooCurrency);
        assertTrue(fooDefault.isPresent());
        assertEquals(UnsignedInteger.valueOf(10), fooDefault.get().getDecimals());
        assertEquals("foo", fooDefault.get().getSymbol());

        Optional<? extends Unit> fooBase = network.baseUnitFor(fooCurrency);
        assertTrue(fooBase.isPresent());
        assertEquals(UnsignedInteger.ZERO, fooBase.get().getDecimals());
        assertEquals("fooi", fooBase.get().getSymbol());
    }

    @Test
    public void testSystemMigrateBRCoreKeyCiphertext() {
        // Setup the expected data

        Coder coder = Coder.createForAlgorithm(Coder.Algorithm.HEX);

        byte[] authenticateData = new byte[0];

        Optional<byte[]> maybeNonce12 = coder.decode(
                "00000000ed41e4e70e000000");
        assertTrue(maybeNonce12.isPresent());
        byte[] nonce12 = maybeNonce12.get();

        Optional<byte[]> maybeCiphertext = coder.decode(
                "1e611714327192ec8454f1e05b1437fa4e56c77ab132d31925c1834f7ed67b7b14d93bdde51b43d38" +
                        "11b2f22a23ca86287ce130740633f0680207137dabae3faa778c4b45ab692eea527902237ee1cfb9" +
                        "97217e9df27e8d9131609d2e96745f1dac6c54b180621bacbb00845fe4183c1192d8f45cc267de7e" +
                        "4ab943dfd73080ae5a3f1dd7c2ea2cc3a009a405154544938c22972744aa62e631c32e9ea7eaa687" +
                        "ccbc244c6b97d9d69644d4b74805837c5ca3caedd63");
        assertTrue(maybeCiphertext.isPresent());
        byte[] ciphertext = maybeCiphertext.get();

        Optional<byte[]> maybeExpectedPlaintext = coder.decode(
                "425a6839314159265359a70ea17800004e5f80400010077ff02c2001003f679c0a200072229ea7a86" +
                        "41a6d47a81a00f5340d53d348f54f4c2479266a7a9e9a9ea3432d1f1618e7e529ecd56e5203e90c3" +
                        "48494fd7b98217b4b525b1c41335aee41453c8c121998a4cc2ef5856afa62e6b82358d48acd52866" +
                        "cc671180b0f2f83aa5c891bb8c043bd254cfd2054b5930bd1910328ea5235866bf4ff8bb9229c284" +
                        "8538750bc00");
        assertTrue(maybeExpectedPlaintext.isPresent());
        byte[] expectedPlaintext = maybeExpectedPlaintext.get();

        // Test with paper key

        {
            byte[] pk = "truly flame one position follow sponsor frost oval tuna swallow situate talk".getBytes(StandardCharsets.UTF_8);
            Optional<? extends Key> maybeCryptoEncodedKey = Key.createForBIP32ApiAuth(pk, HelpersAIT.BIP39_WORDS_EN);
            assertTrue(maybeCryptoEncodedKey.isPresent());
            Key key = maybeCryptoEncodedKey.get();

            // Migrate using the crypto address params key

            Optional<byte[]> maybeMigratedCiphertext = System.migrateBRCoreKeyCiphertext(key, nonce12, authenticateData, ciphertext);
            assertTrue(maybeMigratedCiphertext.isPresent());
            byte[] migratedCiphertext = maybeMigratedCiphertext.get();

            // Decrypt using the crypto address params key

            Cipher cipher = Cipher.createForChaCha20Poly1305(key, nonce12, authenticateData);
            Optional<byte[]> maybeDecryptedPlaintext = cipher.decrypt(migratedCiphertext);
            assertTrue(maybeDecryptedPlaintext.isPresent());
            byte[] decryptedPlaintext = maybeDecryptedPlaintext.get();

            // Verify correct decryption

            assertArrayEquals(expectedPlaintext, decryptedPlaintext);
        }

        // Test with a private key string encoded using CRYPTO_ADDRESS_PARAMS

        byte[] cryptoAddressParamsMigratedCiphertext;
        {
            // Load the correct key encoded with the crypto address params

            byte[] ks = "T7GNuCG4XzHaPGhmUTVGvTnHZodVTrV7KKj1K1vVwTcNcSADqnb5".getBytes(StandardCharsets.UTF_8);
            Optional<? extends Key> maybeCryptoEncodedKey = Key.createFromPrivateKeyString(ks);
            assertTrue(maybeCryptoEncodedKey.isPresent());
            Key key = maybeCryptoEncodedKey.get();

            // Migrate using the crypto address params key

            Optional<byte[]> maybeMigratedCiphertext = System.migrateBRCoreKeyCiphertext(key, nonce12, authenticateData, ciphertext);
            assertTrue(maybeMigratedCiphertext.isPresent());
            cryptoAddressParamsMigratedCiphertext = maybeMigratedCiphertext.get();

            // Decrypt using the crypto address params key

            Cipher cipher = Cipher.createForChaCha20Poly1305(key, nonce12, authenticateData);
            Optional<byte[]> maybeDecryptedPlaintext = cipher.decrypt(cryptoAddressParamsMigratedCiphertext);
            assertTrue(maybeDecryptedPlaintext.isPresent());
            byte[] decryptedPlaintext = maybeDecryptedPlaintext.get();

            // Verify correct decryption

            assertArrayEquals(expectedPlaintext, decryptedPlaintext);
        }

        // Test with a private key string encoded using BITCOIN_ADDRESS_PARAMS

        byte[] mainnetAddressParamsMigratedCiphertext;
        {
            // Load the correct key encoded with the bitcoin address params

            byte[] ks = "cRo6vMxjZg1EmsYAKEMY5RjyFBHb4DZua9yDZdkTsc6DMHhv8Unr".getBytes(StandardCharsets.UTF_8);
            Optional<? extends Key> maybeCryptoEncodedKey = Key.createFromPrivateKeyString(ks);
            assertTrue(maybeCryptoEncodedKey.isPresent());
            Key key = maybeCryptoEncodedKey.get();

            // Migrate using the crypto address params key

            Optional<byte[]> maybeMigratedCiphertext = System.migrateBRCoreKeyCiphertext(key, nonce12, authenticateData, ciphertext);
            assertTrue(maybeMigratedCiphertext.isPresent());
            mainnetAddressParamsMigratedCiphertext = maybeMigratedCiphertext.get();

            // Decrypt using the crypto address params key

            Cipher cipher = Cipher.createForChaCha20Poly1305(key, nonce12, authenticateData);
            Optional<byte[]> maybeDecryptedPlaintext = cipher.decrypt(mainnetAddressParamsMigratedCiphertext);
            assertTrue(maybeDecryptedPlaintext.isPresent());
            byte[] decryptedPlaintext = maybeDecryptedPlaintext.get();

            // Verify correct decryption

            assertArrayEquals(expectedPlaintext, decryptedPlaintext);
        }

        // Confirm that migrated ciphertext is the same, regardless of private key encoding

        assertArrayEquals(cryptoAddressParamsMigratedCiphertext, mainnetAddressParamsMigratedCiphertext);

        {
            // Load the incorrect uncompressed key

            byte[] ks = "5Kb8kLf9zgWQnogidDA76MzPL6TsZZY36hWXMssSzNydYXYB9KF".getBytes(StandardCharsets.UTF_8);
            Optional<? extends Key> maybeCryptoEncodedKey = Key.createFromPrivateKeyString(ks);
            assertTrue(maybeCryptoEncodedKey.isPresent());
            Key key = maybeCryptoEncodedKey.get();

            // Migrate and fail

            Optional<byte[]> maybeMigratedCiphertext = System.migrateBRCoreKeyCiphertext(key, nonce12, authenticateData, ciphertext);
            assertFalse(maybeMigratedCiphertext.isPresent());
        }

        {
            // Load the incorrect uncompressed key

            byte[] ks = "KyvGbxRUoofdw3TNydWn2Z78dBHSy2odn1d3wXWN2o3SAtccFNJL".getBytes(StandardCharsets.UTF_8);
            Optional<? extends Key> maybeCryptoEncodedKey = Key.createFromPrivateKeyString(ks);
            assertTrue(maybeCryptoEncodedKey.isPresent());
            Key key = maybeCryptoEncodedKey.get();

            // Migrate and fail

            Optional<byte[]> maybeMigratedCiphertext = System.migrateBRCoreKeyCiphertext(key, nonce12, authenticateData, ciphertext);
            assertFalse(maybeMigratedCiphertext.isPresent());
        }
    }

    @Test
    public void testSystemBtc() {
        testSystemForCurrency("btc", false, WalletManagerMode.API_ONLY, AddressScheme.BTC_LEGACY, 0);
    }

    @Test
    public void testSystemBch() {
        testSystemForCurrency("bch", false, WalletManagerMode.P2P_ONLY, AddressScheme.BTC_LEGACY, 0);
    }

    @Test
    public void testSystemBsv() {
        testSystemForCurrency("bsv", false, WalletManagerMode.API_ONLY, AddressScheme.BTC_LEGACY, 0);
    }

    @Test
    public void testSystemEth() {
        testSystemForCurrency("eth", true, WalletManagerMode.API_ONLY, AddressScheme.NATIVE, 0);
    }

    @Test
    public void testSystemXrp() {
        testSystemForCurrency("xrp", true, WalletManagerMode.API_ONLY, AddressScheme.NATIVE, 20);
    }

//    @Test
//    public void testSystemHbar() {
//        testSystemForCurrency("hbar", true, WalletManagerMode.API_ONLY, AddressScheme.GEN_DEFAULT, 0);
//    }

    @Test
    public void testSystemXtz() {
        testSystemForCurrency("xtz", true, WalletManagerMode.API_ONLY, AddressScheme.NATIVE, 0);
    }

    private void testSystemForCurrency(String currencyCode, boolean mainnet,  WalletManagerMode mode, AddressScheme scheme, long balanceMinimum) {
        RecordingSystemListener recorder = HelpersAIT.createRecordingListener();
        System system = HelpersAIT.createAndConfigureSystemWithListener(coreDataDir, recorder, mainnet);

        // networks

        Collection<Network> networks = recorder.getAddedNetworks();
        assertNotEquals(0, networks.size());

        Optional<Network> maybeNetwork = HelpersAIT.getNetworkByCurrencyCode(networks, currencyCode);
        assertTrue(maybeNetwork.isPresent());

        Network network = maybeNetwork.get();
        system.createWalletManager(network, mode, scheme, Collections.emptySet());
        Uninterruptibles.sleepUninterruptibly(5, TimeUnit.SECONDS);

        // managers

        Collection<WalletManager> managers = recorder.getAddedManagers();
        assertEquals(1, managers.size());

        Optional<WalletManager> maybeManager = HelpersAIT.getManagerByCode(managers, currencyCode);
        assertTrue(maybeManager.isPresent());

        WalletManager manager = maybeManager.get();
        assertEquals(system, manager.getSystem());
        assertEquals(network, manager.getNetwork());

        // wallets

        Collection<Wallet> wallets = recorder.getAddedWallets();
        assertEquals(1, wallets.size());

        Optional<Wallet> maybeWallet = HelpersAIT.getWalletByCode(wallets, currencyCode);
        assertTrue(maybeWallet.isPresent());

        Wallet wallet = maybeWallet.get();
        assertEquals(manager, wallet.getWalletManager());
        assertEquals(manager.getCurrency(), wallet.getCurrency());
        assertEquals(network.getCurrency(), wallet.getCurrency());
        assertEquals(Amount.create(0, manager.getBaseUnit()), wallet.getBalance());
        assertEquals(Optional.of(Amount.create(balanceMinimum, manager.getDefaultUnit())), wallet.getBalanceMinimum());
        assertTrue(!wallet.getBalanceMaximum().isPresent());
        assertEquals(WalletState.CREATED, wallet.getState());
        assertEquals(0, wallet.getTransfers().size());
    }
}
