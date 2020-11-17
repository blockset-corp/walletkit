/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.crypto;

import com.breadwallet.crypto.blockchaindb.DataTask;
import com.breadwallet.crypto.blockchaindb.BlockchainDb;
import com.breadwallet.crypto.blockchaindb.errors.QueryError;
import com.breadwallet.crypto.blockchaindb.models.bdb.Block;
import com.breadwallet.crypto.blockchaindb.models.bdb.Blockchain;
import com.breadwallet.crypto.blockchaindb.models.bdb.Currency;
import com.breadwallet.crypto.blockchaindb.models.bdb.Subscription;
import com.breadwallet.crypto.blockchaindb.models.bdb.SubscriptionCurrency;
import com.breadwallet.crypto.blockchaindb.models.bdb.SubscriptionEndpoint;
import com.breadwallet.crypto.blockchaindb.models.bdb.SubscriptionEvent;
import com.breadwallet.crypto.blockchaindb.models.bdb.Transaction;
import com.breadwallet.crypto.blockchaindb.models.bdb.Transfer;
import com.breadwallet.crypto.utility.CompletionHandler;
import com.breadwallet.crypto.utility.TestConfiguration;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInteger;
import com.google.common.primitives.UnsignedLong;

import org.junit.Before;
import org.junit.Test;

import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.UUID;
import java.util.concurrent.Semaphore;

import okhttp3.Call;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

import static org.junit.Assert.*;

public class BlockchainDbIT {

    private static final String API_BASE_URL = "https://api.breadwallet.com";

    private BlockchainDb blockchainDb;

    @Before
    public void setup() {
        TestConfiguration configuration = TestConfigurationLoader.getTestConfiguration();
        DataTask decoratedSynchronousDataTask = (client, request, callback) -> {
            Request decoratedRequest = request.newBuilder()
                    .header("Authorization", "Bearer " + configuration.getBlocksetAccess().getToken())
                    .build();
            synchronousDataTask.execute(client, decoratedRequest, callback);
        };
        blockchainDb = new BlockchainDb(new OkHttpClient(),
                configuration.getBlocksetAccess().getBaseURL(), decoratedSynchronousDataTask,
                API_BASE_URL, synchronousDataTask);
    }

    // BDB

    @Test
    public void testGetBlocks() {
        SynchronousCompletionHandler<List<Block>> handler = new SynchronousCompletionHandler<>();

        blockchainDb.getBlocksWithRaw("bitcoin-mainnet", UnsignedLong.ZERO, UnsignedLong.valueOf(10),
                null, handler);
        List<Block> blocksWithRaw = handler.dat().orNull();
        assertNotNull(blocksWithRaw);
        assertNotEquals(0, blocksWithRaw.size());

        blockchainDb.getBlocks("bitcoin-mainnet", UnsignedLong.ZERO, UnsignedLong.valueOf(10),
                true, true, true, null, handler);
        List<Block> blocksWithParts = handler.dat().orNull();
        assertNotNull(blocksWithParts);
        assertNotEquals(0, blocksWithParts.size());

        blockchainDb.getBlocks("bitcoin-mainnet", UnsignedLong.ZERO, UnsignedLong.valueOf(10),
                false, false, false, null, handler);
        List<Block> allBlocks = handler.dat().orNull();
        assertNotNull(allBlocks);
        assertNotEquals(0, allBlocks.size());

        blockchainDb.getBlocks("bitcoin-mainnet", UnsignedLong.ZERO, UnsignedLong.valueOf(10),
                false, false, false, 1, handler);
        List<Block> pagedBlocks = handler.dat().orNull();
        assertNotNull(pagedBlocks);
        assertNotEquals(0, pagedBlocks.size());

        assertEquals(allBlocks.size(), pagedBlocks.size());
    }

    @Test
    public void testGetBlock() {
        SynchronousCompletionHandler<Block> handler = new SynchronousCompletionHandler<>();

        blockchainDb.getBlock("bitcoin-mainnet:000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f",
                true, true, true, handler);
        Block block = handler.dat().orNull();
        assertNotNull(block);
        assertEquals(block.getId(), "bitcoin-mainnet:000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");

        blockchainDb.getBlock("bitcoin-mainnet:000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f",
                false, false, false, handler);
        block = handler.dat().orNull();
        assertNotNull(block);
        assertEquals(block.getId(), "bitcoin-mainnet:000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");

        blockchainDb.getBlockWithRaw("bitcoin-mainnet:000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f",
                handler);
        block = handler.dat().orNull();
        assertNotNull(block);
        assertEquals(block.getId(), "bitcoin-mainnet:000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
    }

    @Test
    public void testGetBlockchains() {
        SynchronousCompletionHandler<List<Blockchain>> handler = new SynchronousCompletionHandler<>();

        blockchainDb.getBlockchains(true, handler);
        List<Blockchain> blockchains = handler.dat().orNull();
        assertNotNull(blockchains);
        assertNotEquals(0, blockchains.size());

        blockchainDb.getBlockchains(false, handler);
        blockchains = handler.dat().orNull();
        assertNotNull(blockchains);
        assertNotEquals(0, blockchains.size());

        blockchainDb.getBlockchains(handler);
        blockchains = handler.dat().orNull();
        assertNotNull(blockchains);
        assertNotEquals(0, blockchains.size());
    }

    @Test
    public void testGetBlockchain() {
        SynchronousCompletionHandler<Blockchain> handler = new SynchronousCompletionHandler<>();

        blockchainDb.getBlockchain("bitcoin-mainnet", handler);
        Blockchain blockchain = handler.dat().orNull();
        assertNotNull(blockchain);
        assertEquals(blockchain.getId(), "bitcoin-mainnet");
        assertEquals(blockchain.getConfirmationsUntilFinal(), UnsignedInteger.valueOf(6));
    }

    @Test
    public void testGetCurrencies() {
        SynchronousCompletionHandler<List<Currency>> handler = new SynchronousCompletionHandler<>();
        List<Currency> currencies;

        blockchainDb.getCurrencies("bitcoin-mainnet", handler);
        currencies = handler.dat().orNull();
        assertNotNull(currencies);
        assertNotEquals(0, currencies.size());

        blockchainDb.getCurrencies(handler);
        currencies = handler.dat().orNull();
        assertNotNull(currencies);
        assertNotEquals(0, currencies.size());
    }

    @Test
    public void testGetCurrency() {
        SynchronousCompletionHandler<Currency> handler = new SynchronousCompletionHandler<>();

        blockchainDb.getCurrency("bitcoin-mainnet:__native__", handler);
        Currency currency = handler.dat().orNull();
        assertNotNull(currency);
        assertEquals(currency.getCode(), "btc");
    }

    @Test
    public void testGetTransfers() {
        SynchronousCompletionHandler<List<Transfer>> handler = new SynchronousCompletionHandler<>();

        blockchainDb.getTransfers("bitcoin-mainnet",
                Collections.singletonList("1JfbZRwdDHKZmuiZgYArJZhcuuzuw2HuMu"),
                UnsignedLong.ZERO,
                UnsignedLong.valueOf(500000),
                null,
                handler);
        List<Transfer> allTransfers = handler.dat().orNull();
        assertNotNull(allTransfers);
        assertNotEquals(0, allTransfers.size());

        blockchainDb.getTransfers("bitcoin-mainnet",
                Collections.singletonList("1JfbZRwdDHKZmuiZgYArJZhcuuzuw2HuMu"),
                UnsignedLong.ZERO,
                UnsignedLong.valueOf(500000),
                1,
                handler);
        List<Transfer> pagedTransfers = handler.dat().orNull();
        assertNotNull(pagedTransfers);
        assertNotEquals(0, pagedTransfers.size());

        // TODO(fix): This test fails; see CORE-741 for more details
        assertEquals(allTransfers.size(), pagedTransfers.size());

        boolean caughtException = false;
        try {
            blockchainDb.getTransfers("bitcoin-mainnet",
                    Collections.EMPTY_LIST,
                    UnsignedLong.ZERO,
                    UnsignedLong.valueOf(500000),
                    1,
                    handler);

        }
        catch (Exception ex) {
            caughtException = true;
        }
        assertTrue (caughtException);
    }

    @Test
    public void testGetTransfer() {
        SynchronousCompletionHandler<Transfer> handler = new SynchronousCompletionHandler<>();

        String transferId = "bitcoin-mainnet:63522845d294ee9b0188ae5cac91bf389a0c3723f084ca1025e7d9cdfe481ce1:1";
        blockchainDb.getTransfer(transferId, handler);
        Transfer transfer = handler.dat().orNull();
        assertNotNull(transfer);
        assertEquals(transferId, transfer.getId());
    }

    @Test
    public void testGetTransactions() {
        SynchronousCompletionHandler<List<Transaction>> handler = new SynchronousCompletionHandler<>();

        blockchainDb.getTransactions("bitcoin-mainnet",
                Collections.singletonList("1JfbZRwdDHKZmuiZgYArJZhcuuzuw2HuMu"),
                UnsignedLong.ZERO,
                UnsignedLong.valueOf(500000),
                true,
                false,
                false,
                null,
                handler);
        List<Transaction> allTransactions = handler.dat().orNull();
        assertNotNull(allTransactions);
        assertNotEquals(0, allTransactions.size());

        blockchainDb.getTransactions("bitcoin-mainnet",
                Collections.singletonList("1JfbZRwdDHKZmuiZgYArJZhcuuzuw2HuMu"),
                UnsignedLong.ZERO,
                UnsignedLong.valueOf(500000),
                true,
                true,
                true,
                1,
                handler);
        List<Transaction> pagedTransactions = handler.dat().orNull();
        assertNotNull(pagedTransactions);
        assertNotEquals(0, pagedTransactions.size());

        // TODO(fix): This test fails; see CORE-741 for more details
        assertEquals(allTransactions.size(), pagedTransactions.size());

        boolean caughtException = false;
        try {
            blockchainDb.getTransactions("bitcoin-mainnet",
                    Collections.EMPTY_LIST,
                    UnsignedLong.ZERO,
                    UnsignedLong.valueOf(500000),
                    true,
                    false,
                    false,
                    null,
                    handler);
        }
        catch (Exception ex) {
            caughtException = true;
        }
        assertTrue (caughtException);
    }

    @Test
    public void testGetTransaction() {
        SynchronousCompletionHandler<Transaction> handler = new SynchronousCompletionHandler<>();

        String transactionId = "bitcoin-mainnet:06d7d63d6c1966a378bbbd234a27a5b583f37d3bdf9fb9ef50f4724c86b4559b";

        blockchainDb.getTransaction(transactionId, true, true, false, handler);
        Transaction transaction = handler.dat().orNull();
        assertNotNull(transaction);
        assertEquals(transactionId, transaction.getId());

        blockchainDb.getTransaction(transactionId, false, false, false, handler);
        transaction = handler.dat().orNull();
        assertNotNull(transaction);
        assertEquals(transactionId, transaction.getId());
    }

    @Test
    public void testSubscriptions() {
        SynchronousCompletionHandler<Void> delHandler;
        SynchronousCompletionHandler<Subscription> subHandler;
        SynchronousCompletionHandler<List<Subscription>> subsHandler;

        String deviceId = UUID.randomUUID().toString();

        SubscriptionEndpoint endpoint = SubscriptionEndpoint.create(
                "fcm",
                "development",
                "fcm registration token");

        List<SubscriptionCurrency> currencies = Collections.singletonList(
                SubscriptionCurrency.create(
                        "bitcoin-testnet:__native__",
                        Arrays.asList(
                                "2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT",
                                "mvnSpXB1Vizfg3uodBx418APVK1jQXScvW"),
                        Collections.singletonList(
                                SubscriptionEvent.create(
                                        "confirmed",
                                        Collections.singletonList(UnsignedInteger.ONE)
                                )
                        )
                )
        );

        // subscription get all

        subsHandler = new SynchronousCompletionHandler<>();
        blockchainDb.getSubscriptions(subsHandler);
        List<Subscription> initialGetSubscriptions = subsHandler.dat().orNull();
        assertNotNull(initialGetSubscriptions);

        // subscription create

        subHandler = new SynchronousCompletionHandler<>();
        blockchainDb.createSubscription(deviceId, endpoint, currencies, subHandler);
        Subscription createSubscription = subHandler.dat().orNull();
        assertNotNull(createSubscription);

        // subscription get

        subHandler = new SynchronousCompletionHandler<>();
        blockchainDb.getSubscription(createSubscription.getId(), subHandler);
        Subscription getSubscription = subHandler.dat().orNull();
        assertNotNull(getSubscription);
        assertEquals(createSubscription.getId(), getSubscription.getId());
        assertEquals(createSubscription.getDevice(), getSubscription.getDevice());

        // subscription get all too

        subsHandler = new SynchronousCompletionHandler<>();
        blockchainDb.getSubscriptions(subsHandler);
        List<Subscription> updatedGetSubscriptions = subsHandler.dat().orNull();
        assertNotNull(updatedGetSubscriptions);
        assertNotEquals(0, updatedGetSubscriptions.size());
        assertEquals(initialGetSubscriptions.size() + 1, updatedGetSubscriptions.size());

        // subscription update

        List<SubscriptionCurrency> updatedCurrencies = Collections.singletonList(
                SubscriptionCurrency.create(
                        "bitcoin-testnet:__native__",
                        Collections.singletonList(
                                "2NEpHgLvBJqGFVwQPUA3AQPjpE5gNWhETfT"),
                        Collections.singletonList(
                                SubscriptionEvent.create(
                                        "confirmed",
                                        Collections.singletonList(UnsignedInteger.ONE)
                                )
                        )
                )
        );

        Subscription updatedSubscription = Subscription.create(
                createSubscription.getId(),
                createSubscription.getDevice(),
                createSubscription.getEndpoint(),
                updatedCurrencies
        );

        subHandler = new SynchronousCompletionHandler<>();
        blockchainDb.updateSubscription(updatedSubscription, subHandler);
        Subscription updateSubscription = subHandler.dat().orNull();
        assertNotNull(updateSubscription);
        assertEquals(updatedSubscription.getCurrencies().size(), updateSubscription.getCurrencies().size());

        // subscription delete

        delHandler = new SynchronousCompletionHandler<>();
        blockchainDb.deleteSubscription(createSubscription.getId(), delHandler);
        QueryError error = delHandler.err().orNull();
        assertNull(error);

        // subscription get all tre

        subsHandler = new SynchronousCompletionHandler<>();
        blockchainDb.getSubscriptions(subsHandler);
        List<Subscription> updatedAgainGetSubscriptions = subsHandler.dat().orNull();
        assertNotNull(updatedAgainGetSubscriptions);
        assertEquals(initialGetSubscriptions.size(), updatedAgainGetSubscriptions.size());
    }

    // Helpers

    private static class SynchronousCompletionHandler<T> implements CompletionHandler<T, QueryError> {

        private final Semaphore sema = new Semaphore(0);

        private T data;
        private QueryError error;

        Optional<T> dat() {
            sema.acquireUninterruptibly();
            return Optional.fromNullable(data);
        }

        Optional<QueryError> err() {
            sema.acquireUninterruptibly();
            return Optional.fromNullable(error);
        }

        @Override
        public void handleData(T data) {
            this.data = data;
            this.error = null;
            sema.release();
        }

        @Override
        public void handleError(QueryError error) {
            this.data = null;
            this.error = error;
            sema.release();
        }
    }

    private static final DataTask synchronousDataTask = (client, request, callback) -> {
        Call call = client.newCall(request);
        try (Response r = call.execute()) {
            callback.onResponse(call, r);

        } catch (IOException e) {
            callback.onFailure(call, e);
        }
    };
}
