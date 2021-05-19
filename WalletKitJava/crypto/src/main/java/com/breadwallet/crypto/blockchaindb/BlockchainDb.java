/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.crypto.blockchaindb;

import androidx.annotation.Nullable;

import com.breadwallet.crypto.blockchaindb.apis.bdb.BlockApi;
import com.breadwallet.crypto.blockchaindb.apis.bdb.BlockchainApi;
import com.breadwallet.crypto.blockchaindb.apis.bdb.CurrencyApi;
import com.breadwallet.crypto.blockchaindb.apis.bdb.BdbApiClient;
import com.breadwallet.crypto.blockchaindb.apis.bdb.ExperimentalApi;
import com.breadwallet.crypto.blockchaindb.apis.bdb.SubscriptionApi;
import com.breadwallet.crypto.blockchaindb.apis.bdb.TransactionApi;
import com.breadwallet.crypto.blockchaindb.apis.bdb.TransferApi;
import com.breadwallet.crypto.blockchaindb.apis.brd.BrdApiClient;
import com.breadwallet.crypto.blockchaindb.errors.QueryError;
import com.breadwallet.crypto.blockchaindb.models.bdb.Block;
import com.breadwallet.crypto.blockchaindb.models.bdb.Blockchain;
import com.breadwallet.crypto.blockchaindb.models.bdb.Currency;
import com.breadwallet.crypto.blockchaindb.models.bdb.HederaAccount;
import com.breadwallet.crypto.blockchaindb.models.bdb.Subscription;
import com.breadwallet.crypto.blockchaindb.models.bdb.SubscriptionCurrency;
import com.breadwallet.crypto.blockchaindb.models.bdb.SubscriptionEndpoint;
import com.breadwallet.crypto.blockchaindb.models.bdb.Transaction;
import com.breadwallet.crypto.blockchaindb.models.bdb.TransactionFee;
import com.breadwallet.crypto.blockchaindb.models.bdb.TransactionIdentifier;
import com.breadwallet.crypto.blockchaindb.models.bdb.Transfer;
import com.breadwallet.crypto.utility.CompletionHandler;
import com.google.common.primitives.UnsignedLong;

import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

import okhttp3.OkHttpClient;
import okhttp3.Request;

public class BlockchainDb {

    private static final String DEFAULT_BDB_BASE_URL = "https://api.blockset.com";
    private static final String DEFAULT_API_BASE_URL = "https://api.breadwallet.com";
    private static final DataTask DEFAULT_DATA_TASK = (cli, request, callback) -> cli.newCall(request).enqueue(callback);

    private final AtomicInteger ridGenerator;

    private final OkHttpClient client;
    private final BlockApi blockApi;
    private final BlockchainApi blockchainApi;
    private final CurrencyApi currencyApi;
    private final SubscriptionApi subscriptionApi;
    private final TransferApi transferApi;
    private final TransactionApi transactionApi;
    private final ExperimentalApi experimentalApi;

    public BlockchainDb(OkHttpClient client) {
        this(client, null, null, null, null);
    }

    public BlockchainDb(OkHttpClient client, String bdbBaseURL, String apiBaseURL) {
        this(client, bdbBaseURL, null, apiBaseURL, null);
    }

    public BlockchainDb(OkHttpClient client,
                        @Nullable String bdbBaseURL,
                        @Nullable DataTask bdbDataTask,
                        @Nullable String apiBaseURL,
                        @Nullable DataTask apiDataTask) {
        bdbBaseURL = bdbBaseURL == null ? DEFAULT_BDB_BASE_URL : bdbBaseURL;
        apiBaseURL = apiBaseURL == null ? DEFAULT_API_BASE_URL : apiBaseURL;

        bdbDataTask = bdbDataTask == null ? DEFAULT_DATA_TASK : bdbDataTask;
        apiDataTask = apiDataTask == null ? DEFAULT_DATA_TASK : apiDataTask;

        ObjectCoder coder = ObjectCoder.createObjectCoderWithFailOnUnknownProperties();
        BdbApiClient bdbClient = new BdbApiClient(client, bdbBaseURL, bdbDataTask, coder);
        BrdApiClient brdClient = new BrdApiClient(client, apiBaseURL, apiDataTask, coder);

        ExecutorService executorService = Executors.newCachedThreadPool();
        ScheduledExecutorService scheduledExecutorService = Executors.newSingleThreadScheduledExecutor();

        this.ridGenerator = new AtomicInteger(0);

        this.client = client;
        this.blockApi = new BlockApi(bdbClient, executorService);
        this.blockchainApi = new BlockchainApi(bdbClient);
        this.currencyApi = new CurrencyApi(bdbClient, executorService);
        this.subscriptionApi = new SubscriptionApi(bdbClient);
        this.transferApi = new TransferApi(bdbClient, executorService);
        this.transactionApi = new TransactionApi(bdbClient, executorService);
        this.experimentalApi = new ExperimentalApi(bdbClient, scheduledExecutorService);
    }

    public static BlockchainDb createForTest (OkHttpClient client,
                                              String bdbAuthToken) {
        return createForTest(client, bdbAuthToken, null, null);
    }

    public static BlockchainDb createForTest (OkHttpClient client,
                                              String bdbAuthToken,
                                              @Nullable String bdbBaseURL,
                                              @Nullable String apiBaseURL) {
        DataTask brdDataTask = (cli, request, callback) -> {
            Request decoratedRequest = request.newBuilder()
                    .header("Authorization", "Bearer " + bdbAuthToken)
                    .build();
            cli.newCall(decoratedRequest).enqueue(callback);
        };
        return new BlockchainDb (client, bdbBaseURL, brdDataTask, apiBaseURL, null);
    }

    /**
     * Cancel all BlockchainDb requests that are currently enqueued or executing
     */
    public void cancelAll () {
        client.dispatcher().cancelAll();
        // In a race, any Callable on any Executor might run NOW, causing a `client` request.
        // That is okay; we'll have some more data.  That is, it is no different from if the 
        // request had completed just before the `cancelAll()` call.
    }

    // Blockchain

    public void getBlockchains(CompletionHandler<List<Blockchain>, QueryError> handler) {
        blockchainApi.getBlockchains(
                true,
                handler
        );
    }

    public void getBlockchains(boolean isMainnet,
                               CompletionHandler<List<Blockchain>, QueryError> handler) {
        blockchainApi.getBlockchains(
                isMainnet,
                handler
        );
    }

    public void getBlockchain(String id,
                              CompletionHandler<Blockchain, QueryError> handler) {
        blockchainApi.getBlockchain(
                id,
                handler
        );
    }

    // Currency

    public void getCurrencies(CompletionHandler<List<Currency>, QueryError> handler) {
        currencyApi.getCurrencies(
                handler
        );
    }

    public void getCurrencies(@Nullable String id,
                              CompletionHandler<List<Currency>, QueryError> handler) {
        currencyApi.getCurrencies(
                id,
                handler
        );
    }

    public void getCurrencies(boolean mainnet,
                              CompletionHandler<List<Currency>, QueryError> handler) {
        currencyApi.getCurrencies(
                mainnet,
                handler
        );
    }

    public void getCurrency(String id,
                            CompletionHandler<Currency, QueryError> handler) {
        currencyApi.getCurrency(
                id,
                handler
        );
    }

    // Subscription

    public void getOrCreateSubscription(Subscription subscription,
                                        CompletionHandler<Subscription, QueryError> handler) {
        subscriptionApi.getOrCreateSubscription(
                subscription,
                handler
        );
    }

    public void getSubscription(String id,
                                CompletionHandler<Subscription, QueryError> handler) {
        subscriptionApi.getSubscription(
                id,
                handler
        );
    }

    public void getSubscriptions(CompletionHandler<List<Subscription>, QueryError> handler) {
        subscriptionApi.getSubscriptions(
                handler
        );
    }

    public void createSubscription(String deviceId,
                                   SubscriptionEndpoint endpoint,
                                   List<SubscriptionCurrency> currencies,
                                   CompletionHandler<Subscription, QueryError> handler) {
        subscriptionApi.createSubscription(
                deviceId,
                endpoint,
                currencies,
                handler
        );
    }

    public void updateSubscription(Subscription subscription,
                                   CompletionHandler<Subscription, QueryError> handler) {
        subscriptionApi.updateSubscription(
                subscription,
                handler
        );
    }

    public void deleteSubscription(String id,
                                   CompletionHandler<Void, QueryError> handler) {
        subscriptionApi.deleteSubscription(
                id,
                handler
        );
    }

    // Transfer

    /* Throws 'IllegalArgumentException' if `addresses` is empty. */
    public void getTransfers(String id,
                             List<String> addresses,
                             UnsignedLong beginBlockNumber,
                             UnsignedLong endBlockNumber,
                             CompletionHandler<List<Transfer>, QueryError> handler) {
        getTransfers(
                id,
                addresses,
                beginBlockNumber,
                endBlockNumber,
                null,
                handler
        );
    }

    /* Throws 'IllegalArgumentException' if `addresses` is empty. */
    public void getTransfers(String id,
                             List<String> addresses,
                             @Nullable UnsignedLong beginBlockNumber,
                             @Nullable UnsignedLong endBlockNumber,
                             @Nullable Integer maxPageSize,
                             CompletionHandler<List<Transfer>, QueryError> handler) {
        transferApi.getTransfers(
                id,
                addresses,
                beginBlockNumber,
                endBlockNumber,
                maxPageSize,
                handler
        );
    }

    public void getTransfer(String id,
                            CompletionHandler<Transfer, QueryError> handler) {
        transferApi.getTransfer(
                id,
                handler
        );
    }

    // Transactions

    /* Throws 'IllegalArgumentException' if `addresses` is empty. */
    public void getTransactions(String id,
                                List<String> addresses,
                                @Nullable UnsignedLong beginBlockNumber,
                                @Nullable UnsignedLong endBlockNumber,
                                boolean includeRaw,
                                boolean includeProof,
                                boolean includeTransfers,
                                CompletionHandler<List<Transaction>, QueryError> handler) {
        getTransactions(
                id,
                addresses,
                beginBlockNumber,
                endBlockNumber,
                includeRaw,
                includeProof,
                includeTransfers,
                null,
                handler
        );
    }

    /* Throws 'IllegalArgumentException' if `addresses` is empty. */
    public void getTransactions(String id,
                                List<String> addresses,
                                @Nullable UnsignedLong beginBlockNumber,
                                @Nullable UnsignedLong endBlockNumber,
                                boolean includeRaw,
                                boolean includeProof,
                                boolean includeTransfers,
                                @Nullable Integer maxPageSize,
                                CompletionHandler<List<Transaction>, QueryError> handler) {
        transactionApi.getTransactions(
                id,
                addresses,
                beginBlockNumber,
                endBlockNumber,
                includeRaw,
                includeProof,
                includeTransfers,
                maxPageSize,
                handler
        );
    }

    public void getTransaction(String id,
                               boolean includeRaw,
                               boolean includeProof,
                               boolean includeTransfers,
                               CompletionHandler<Transaction, QueryError> handler) {
        transactionApi.getTransaction(
                id,
                includeRaw,
                includeProof,
                includeTransfers,
                handler
        );
    }

    public void createTransaction(String id,
                                  byte[] tx,
                                  String identifier,
                                  CompletionHandler<TransactionIdentifier, QueryError> handler) {
        transactionApi.createTransaction(
                id,
                tx,
                identifier,
                handler
        );
    }

    public void estimateTransactionFee(String id,
                                       byte[] tx,
                                       CompletionHandler<TransactionFee, QueryError> handler) {
        transactionApi.estimateTransactionFee(
                id,
                tx,
                handler
        );
    }

    // Blocks

    public void getBlocks(String id,
                          UnsignedLong beginBlockNumber,
                          UnsignedLong endBlockNumber,
                          boolean includeTx,
                          boolean includeTxRaw,
                          boolean includeTxProof,
                          CompletionHandler<List<Block>, QueryError> handler) {
        getBlocks(
                id,
                beginBlockNumber,
                endBlockNumber,
                includeTx,
                includeTxRaw,
                includeTxProof,
                null,
                handler)
        ;
    }

    public void getBlocks(String id,
                          UnsignedLong beginBlockNumber,
                          UnsignedLong endBlockNumber,
                          boolean includeTx,
                          boolean includeTxRaw,
                          boolean includeTxProof,
                          @Nullable Integer maxPageSize,
                          CompletionHandler<List<Block>, QueryError> handler) {
        blockApi.getBlocks(
                id,
                beginBlockNumber,
                endBlockNumber,
                false,
                includeTx,
                includeTxRaw,
                includeTxProof,
                maxPageSize,
                handler);
    }

    public void getBlocksWithRaw(String id,
                                 UnsignedLong beginBlockNumber,
                                 UnsignedLong endBlockNumber,
                                 @Nullable Integer maxPageSize,
                                 CompletionHandler<List<Block>, QueryError> handler) {
        blockApi.getBlocks(
                id,
                beginBlockNumber,
                endBlockNumber,
                true,
                false,
                false,
                false,
                maxPageSize,
                handler
        );
    }

    public void getBlock(String id,
                         boolean includeTx,
                         boolean includeTxRaw,
                         boolean includeTxProof,
                         CompletionHandler<Block, QueryError> handler) {
        blockApi.getBlock(
                id,
                false,
                includeTx,
                includeTxRaw,
                includeTxProof,
                handler
        );
    }

    public void getBlockWithRaw(String id,
                                CompletionHandler<Block, QueryError> handler) {
        blockApi.getBlock(
                id,
                true,
                false,
                false,
                false,
                handler
        );
    }

    // Addresses

    // Experimental - Hedera Account Creation

    public void getHederaAccount(String id,
                                 String publicKey,
                                 CompletionHandler<List<HederaAccount>, QueryError> handler) {
        experimentalApi.getHederaAccount(
                id,
                publicKey,
                handler
        );
    }

    public void createHederaAccount(String id,
                                    String publicKey,
                                    CompletionHandler<List<HederaAccount>, QueryError> handler) {
        experimentalApi.createHederaAccount(
                id,
                publicKey,
                handler
        );
    }
}
