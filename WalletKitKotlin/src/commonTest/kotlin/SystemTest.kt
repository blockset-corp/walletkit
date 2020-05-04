package com.breadwallet.core

import com.breadwallet.core.common.Cipher
import com.breadwallet.core.common.Coder
import com.breadwallet.core.common.CoderAlgorithm
import com.breadwallet.core.common.Key
import kotlin.test.*


class SystemTest {
    //private var coreDataDir: File? = null
    @BeforeTest
    fun setup() {
        //coreDataDir = HelpersAIT.generateCoreDataDir()
    }

    @AfterTest
    fun teardown() {
        //HelpersAIT.deleteFile(coreDataDir)
    }

    @Ignore
    @Test
    fun testSystemMigrateBRCoreKeyCiphertext() { // Setup the expected data
        val coder = Coder.createForAlgorithm(CoderAlgorithm.HEX)
        val authenticateData = ByteArray(0)
        val nonce12 = assertNotNull(coder.decode("00000000ed41e4e70e000000"))
        val ciphertext = assertNotNull(coder.decode(buildString {
            append("1e611714327192ec8454f1e05b1437fa4e56c77ab132d31925c1834f7ed67b7b14d93bdde51b43d38")
            append("11b2f22a23ca86287ce130740633f0680207137dabae3faa778c4b45ab692eea527902237ee1cfb9")
            append("97217e9df27e8d9131609d2e96745f1dac6c54b180621bacbb00845fe4183c1192d8f45cc267de7e")
            append("4ab943dfd73080ae5a3f1dd7c2ea2cc3a009a405154544938c22972744aa62e631c32e9ea7eaa687")
            append("ccbc244c6b97d9d69644d4b74805837c5ca3caedd63")
        }))
        val expectedPlaintext = assertNotNull(coder.decode(buildString {
            append("425a6839314159265359a70ea17800004e5f80400010077ff02c2001003f679c0a200072229ea7a86")
            append("41a6d47a81a00f5340d53d348f54f4c2479266a7a9e9a9ea3432d1f1618e7e529ecd56e5203e90c3")
            append("48494fd7b98217b4b525b1c41335aee41453c8c121998a4cc2ef5856afa62e6b82358d48acd52866")
            append("cc671180b0f2f83aa5c891bb8c043bd254cfd2054b5930bd1910328ea5235866bf4ff8bb9229c284")
            append("8538750bc00")
        }))

        // Test with paper key
        run {
            val pk = "truly flame one position follow sponsor frost oval tuna swallow situate talk"
            val key = assertNotNull(Key.createForBIP32ApiAuth(pk, WORDS))
            // Migrate using the crypto address params key
            val migratedCiphertext = assertNotNull(System.migrateBRCoreKeyCiphertext(
                    key, nonce12, authenticateData, ciphertext))

            // Decrypt using the crypto address params key
            val cipher = Cipher.createForChaCha20Poly1305(key, nonce12, authenticateData)
            val decryptedPlaintext = assertNotNull(cipher.decrypt(migratedCiphertext))
            // Verify correct decryption
            assertContentEquals(expectedPlaintext, decryptedPlaintext)
        }

        // Test with a private key string encoded using CRYPTO_ADDRESS_PARAMS
        var cryptoAddressParamsMigratedCiphertext: ByteArray
        run {
            // Load the correct key encoded with the crypto address params
            val ks = "T7GNuCG4XzHaPGhmUTVGvTnHZodVTrV7KKj1K1vVwTcNcSADqnb5"
            val key = assertNotNull(Key.createFromPrivateKey(ks))
            // Migrate using the crypto address params key
            val migratedCiphertext = assertNotNull(System.migrateBRCoreKeyCiphertext(
                    key, nonce12, authenticateData, ciphertext))
            cryptoAddressParamsMigratedCiphertext = migratedCiphertext
            // Decrypt using the crypto address params key
            val cipher: Cipher = Cipher.createForChaCha20Poly1305(key, nonce12, authenticateData)
            val decryptedPlaintext = assertNotNull(cipher.decrypt(cryptoAddressParamsMigratedCiphertext))
            // Verify correct decryption
            assertContentEquals(expectedPlaintext, decryptedPlaintext)
        }
        // Test with a private key string encoded using BITCOIN_ADDRESS_PARAMS
        var mainnetAddressParamsMigratedCiphertext: ByteArray
        run {
            // Load the correct key encoded with the bitcoin address params
            val ks = "cRo6vMxjZg1EmsYAKEMY5RjyFBHb4DZua9yDZdkTsc6DMHhv8Unr"
            val key = assertNotNull(Key.createFromPrivateKey(ks))
            // Migrate using the crypto address params key
            val migratedCiphertext = assertNotNull(System.migrateBRCoreKeyCiphertext(
                    key, nonce12, authenticateData, ciphertext))
            mainnetAddressParamsMigratedCiphertext = migratedCiphertext
            // Decrypt using the crypto address params key
            val cipher: Cipher = Cipher.createForChaCha20Poly1305(key, nonce12, authenticateData)
            val decryptedPlaintext = assertNotNull(cipher.decrypt(mainnetAddressParamsMigratedCiphertext))
            // Verify correct decryption
            assertContentEquals(expectedPlaintext, decryptedPlaintext)
        }

        // Confirm that migrated ciphertext is the same, regardless of private key encoding
        assertContentEquals(cryptoAddressParamsMigratedCiphertext, mainnetAddressParamsMigratedCiphertext)
        run {
            // Load the incorrect uncompressed key
            val ks = "5Kb8kLf9zgWQnogidDA76MzPL6TsZZY36hWXMssSzNydYXYB9KF"
            val key = assertNotNull(Key.createFromPrivateKey(ks))
            // Migrate and fail
            assertNull(System.migrateBRCoreKeyCiphertext(key, nonce12, authenticateData, ciphertext))
        }
        run {
            // Load the incorrect uncompressed key
            val ks = "KyvGbxRUoofdw3TNydWn2Z78dBHSy2odn1d3wXWN2o3SAtccFNJL"
            val key = assertNotNull(Key.createFromPrivateKey(ks))
            // Migrate and fail
            assertNull(System.migrateBRCoreKeyCiphertext(key, nonce12, authenticateData, ciphertext))
        }
    }

/*
  @Test
  fun testSystemBtc() {
    testSystemForCurrency("btc", WalletManagerMode.API_ONLY, AddressScheme.BTC_LEGACY)
  }

  @Test
  fun testSystemBch() {
    testSystemForCurrency("bch", WalletManagerMode.P2P_ONLY, AddressScheme.BTC_LEGACY)
  }

  @Test
  fun testSystemEth() {
    testSystemForCurrency("eth", WalletManagerMode.API_ONLY, AddressScheme.ETH_DEFAULT)
  }

  @Test
  fun testSystemEthMigrationNotRequired() {
    testSystemMigrationNotRequiredForCurrency("eth")
  }

  @Test
  fun testSystemBtcMigrationSuccess() {
    testSystemMigrationSuccessForBitcoinCurrency("btc")
  }

  @Test
  fun testSystemBchMigrationSuccess() {
    testSystemMigrationSuccessForBitcoinCurrency("bch")
  }

  @Test
  fun testSystemBtcMigrationFailureOnTransaction() {
    testSystemMigrationFailureOnTransactionForBitcoinCurrency("btc")
  }

  @Test
  fun testSystemBchMigrationFailureOnTransaction() {
    testSystemMigrationFailureOnTransactionForBitcoinCurrency("bch")
  }

  @Test
  fun testSystemBtcMigrationFailureOnBlock() {
    testSystemMigrationFailureOnBlockForBitcoinCurrency("btc")
  }

  @Test
  fun testSystemBchMigrationFailureOnBlock() {
    testSystemMigrationFailureOnBlockForBitcoinCurrency("bch")
  }
  private fun testSystemForCurrency(currencyCode: String, mode: WalletManagerMode, scheme: AddressScheme) {
    val recorder: RecordingSystemListener = HelpersAIT.createRecordingListener()
    val system: java.lang.System = HelpersAIT.createAndConfigureSystemWithListener(coreDataDir, recorder)
    // networks
    val networks: Collection<Network> = recorder.getAddedNetworks()
    org.junit.Assert.assertNotEquals(0, networks.size.toLong())
    val maybeNetwork: com.google.common.base.Optional<Network> = HelpersAIT.getNetworkByCurrencyCode(networks, currencyCode)
    org.junit.Assert.assertTrue(maybeNetwork.isPresent())
    val network: Network = maybeNetwork.get()
    system.createWalletManager(network, mode, scheme, emptySet())
    Uninterruptibles.sleepUninterruptibly(5, TimeUnit.SECONDS)
    // managers
    val managers: Collection<WalletManager> = recorder.getAddedManagers()
    org.junit.Assert.assertEquals(1, managers.size.toLong())
    val maybeManager: com.google.common.base.Optional<WalletManager> = HelpersAIT.getManagerByCode(managers, currencyCode)
    org.junit.Assert.assertTrue(maybeManager.isPresent())
    val manager: WalletManager = maybeManager.get()
    assertEquals(system, manager.getSystem())
    assertEquals(network, manager.getNetwork())
    // wallets
    val wallets: Collection<Wallet> = recorder.getAddedWallets()
    org.junit.Assert.assertEquals(1, wallets.size.toLong())
    val maybeWallet: com.google.common.base.Optional<Wallet> = HelpersAIT.getWalletByCode(wallets, currencyCode)
    org.junit.Assert.assertTrue(maybeWallet.isPresent())
    val wallet: Wallet = maybeWallet.get()
    assertEquals(manager, wallet.getWalletManager())
    assertEquals(manager.getCurrency(), wallet.getCurrency())
    assertEquals(network.getCurrency(), wallet.getCurrency())
    assertEquals(Amount.create(0, manager.getBaseUnit()), wallet.getBalance())
    assertEquals(WalletState.CREATED, wallet.getState())
    assertEquals(0, wallet.getTransfers().size())
  }
  private fun testSystemMigrationNotRequiredForCurrency(currencyCode: String) {
    val recorder: RecordingSystemListener = HelpersAIT.createRecordingListener()
    val system: java.lang.System = HelpersAIT.createAndConfigureSystemWithListener(coreDataDir, recorder)
    val networks: Collection<Network> = recorder.getAddedNetworks()
    val network: Network = HelpersAIT.getNetworkByCurrencyCode(networks, currencyCode).get()
    org.junit.Assert.assertFalse(system.migrateRequired(network))
  }

  private fun testSystemMigrationSuccessForBitcoinCurrency(currencyCode: String) {
    val recorder: RecordingSystemListener = HelpersAIT.createRecordingListener()
    val system: java.lang.System = HelpersAIT.createAndConfigureSystemWithListener(coreDataDir, recorder)
    val networks: Collection<Network> = recorder.getAddedNetworks()
    val network: Network = HelpersAIT.getNetworkByCurrencyCode(networks, currencyCode).get()
    // transaction blob
    val transactionBlobs: List<TransactionBlob> = listOf(
        TransactionBlob.BTC(
            Coder.createForAlgorithm(com.breadwallet.crypto.Coder.Algorithm.HEX).decode(
                "010000000001017b032f6a651c7dcbcfb78d817b303be8d20afa22901618b517f21755a7cd8d4801" +
                    "00000023220020e0627b64745905646f276f355502a4053058b64edbf277119249611c98da4169ff" +
                    "ffffff020cf962010000000017a914243157d578bd928a92e039e8d4dbbb294416935c87f3be2a00" +
                    "000000001976a91448380bc7605e91a38f8d7ba01a2795416bf92dde88ac040047304402205f5de6" +
                    "8896ca3edf97e3ea1fd3513903537fd5f2e0b3661d6c617b1c48fc69e102200e0f2059513be93183" +
                    "929c7d3e2de0e9c7085706a88e8f746e8f5aa713d27a5201473044022050d8ecb9cd7fdacb6d6351" +
                    "dec2bc5b3716328ef2c4466db44bdd34a657292b8c022068501bf81812ad8e3ed9df24354c371923" +
                    "a07dc966a6e41463594774d009169e0169522103b8e138ed70232c9cbd1b9028121064236af12dbe" +
                    "98641c3f74fa13166f272f582103f66ee7c87817d324921edc3f7d7726de5a18cfed057e5a50e7c7" +
                    "4e2ae7e05ad72102a7bf21582d71e5da5c3bc43e84c88fdf32803aa4720e1c1a9d08aab541a4f331" +
                    "53ae00000000").get(),
            UnsignedInteger.ZERO,
            UnsignedInteger.ZERO)
    )
    // block blob
    val blockBlobs: List<BlockBlob> = listOf(
        BlockBlob.BTC(
            Coder.createForAlgorithm(com.breadwallet.crypto.Coder.Algorithm.HEX).decode(
                "0100000006e533fd1ada86391f3f6c343204b0d278d4aaec1c0b20aa27ba0300000000006abbb3eb" +
                    "3d733a9fe18967fd7d4c117e4ccbbac5bec4d910d900b3ae0793e77f54241b4d4c86041b4089cc9b" +
                    "0c000000084c30b63cfcdc2d35e3329421b9805ef0c6565d35381ca857762ea0b3a5a128bbca5065" +
                    "ff9617cbcba45eb23726df6498a9b9cafed4f54cbab9d227b0035ddefbbb15ac1d57d0182aaee61c" +
                    "74743a9c4f785895e563909bafec45c9a2b0ff3181d77706be8b1dcc91112eada86d424e2d0a8907" +
                    "c3488b6e44fda5a74a25cbc7d6bb4fa04245f4ac8a1a571d5537eac24adca1454d65eda446055479" +
                    "af6c6d4dd3c9ab658448c10b6921b7a4ce3021eb22ed6bb6a7fde1e5bcc4b1db6615c6abc5ca0421" +
                    "27bfaf9f44ebce29cb29c6df9d05b47f35b2edff4f0064b578ab741fa78276222651209fe1a2c4c0" +
                    "fa1c58510aec8b090dd1eb1f82f9d261b8273b525b02ff1a").get(),
            UnsignedInteger.ZERO)
    )
    // peer blob
    val peerBlobs: List<PeerBlob> = listOf(
        PeerBlob.BTC(
            UnsignedInteger.ZERO,
            UnsignedInteger.ZERO,
            com.google.common.primitives.UnsignedLong.ZERO,
            UnsignedInteger.ZERO
        )
    )
    org.junit.Assert.assertTrue(system.migrateRequired(network))
    try {
      system.migrateStorage(network, transactionBlobs, blockBlobs, peerBlobs)
    } catch (e: MigrateError) {
      org.junit.Assert.fail()
    }
  }

  private fun testSystemMigrationFailureOnTransactionForBitcoinCurrency(currencyCode: String) {
    val recorder: RecordingSystemListener = HelpersAIT.createRecordingListener()
    val system: java.lang.System = HelpersAIT.createAndConfigureSystemWithListener(coreDataDir, recorder)
    val networks: Collection<Network> = recorder.getAddedNetworks()
    val network: Network = HelpersAIT.getNetworkByCurrencyCode(networks, currencyCode).get()
    // transaction blob
    val transactionBlobs: List<TransactionBlob> = listOf(
        TransactionBlob.BTC(
            Coder.createForAlgorithm(com.breadwallet.crypto.Coder.Algorithm.HEX).decode(
                "BAAD").get(),
            UnsignedInteger.ZERO,
            UnsignedInteger.ZERO)
    )
    org.junit.Assert.assertTrue(system.migrateRequired(network))
    var error: MigrateError? = null
    try {
      system.migrateStorage(network, transactionBlobs, emptyList(), emptyList())
    } catch (e: MigrateTransactionError) {
      error = e
    } catch (e: MigrateError) {
      org.junit.Assert.fail()
    }
    org.junit.Assert.assertNotNull(error)
  }

  private fun testSystemMigrationFailureOnBlockForBitcoinCurrency(currencyCode: String) {
    val recorder: RecordingSystemListener = HelpersAIT.createRecordingListener()
    val system: java.lang.System = HelpersAIT.createAndConfigureSystemWithListener(coreDataDir, recorder)
    val networks: Collection<Network> = recorder.getAddedNetworks()
    val network: Network = HelpersAIT.getNetworkByCurrencyCode(networks, currencyCode).get()
    // block blob
    val blockBlobs: List<BlockBlob> = listOf(
        BlockBlob.BTC(
            Coder.createForAlgorithm(com.breadwallet.crypto.Coder.Algorithm.HEX).decode(
                "BAAD").get(),
            UnsignedInteger.ZERO)
    )
    org.junit.Assert.assertTrue(system.migrateRequired(network))
    var error: MigrateError? = null
    try {
      system.migrateStorage(network, emptyList(), blockBlobs, emptyList())
    } catch (e: MigrateBlockError) {
      error = e
    } catch (e: MigrateError) {
      org.junit.Assert.fail()
    }
    org.junit.Assert.assertNotNull(error)
  }*/
}
