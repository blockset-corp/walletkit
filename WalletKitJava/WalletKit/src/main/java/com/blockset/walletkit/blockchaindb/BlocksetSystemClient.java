/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.blockchaindb;

import android.support.annotation.Nullable;

import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.blockchaindb.apis.bdb.BlockApi;
import com.blockset.walletkit.blockchaindb.apis.bdb.BlockchainApi;
import com.blockset.walletkit.blockchaindb.apis.bdb.CurrencyApi;
import com.blockset.walletkit.blockchaindb.apis.bdb.BdbApiClient;
import com.blockset.walletkit.blockchaindb.apis.bdb.ExperimentalApi;
import com.blockset.walletkit.blockchaindb.apis.bdb.SubscriptionApi;
import com.blockset.walletkit.blockchaindb.apis.bdb.TransactionApi;
import com.blockset.walletkit.blockchaindb.apis.bdb.TransferApi;
import com.blockset.walletkit.blockchaindb.errors.QueryError;
import com.blockset.walletkit.blockchaindb.models.bdb.Block;
import com.blockset.walletkit.blockchaindb.models.bdb.Blockchain;
import com.blockset.walletkit.blockchaindb.models.bdb.Currency;
import com.blockset.walletkit.blockchaindb.models.bdb.HederaAccount;
import com.blockset.walletkit.blockchaindb.models.bdb.Subscription;
import com.blockset.walletkit.blockchaindb.models.bdb.SubscriptionCurrency;
import com.blockset.walletkit.blockchaindb.models.bdb.SubscriptionEndpoint;
import com.blockset.walletkit.blockchaindb.models.bdb.Transaction;
import com.blockset.walletkit.blockchaindb.models.bdb.TransactionFee;
import com.blockset.walletkit.blockchaindb.models.bdb.TransactionIdentifier;
import com.blockset.walletkit.blockchaindb.models.bdb.Transfer;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.primitives.UnsignedLong;

import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

import okhttp3.OkHttpClient;
import okhttp3.Request;

public class BlocksetSystemClient implements SystemClient {

    private static final String DEFAULT_BDB_BASE_URL = "https://api.blockset.com";
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

    public BlocksetSystemClient(OkHttpClient client) {
        this(client, null, null);
    }

    public BlocksetSystemClient(OkHttpClient client, String bdbBaseURL) {
        this(client, bdbBaseURL, null);
    }

    public BlocksetSystemClient(OkHttpClient client,
                        @Nullable String bdbBaseURL,
                        @Nullable DataTask bdbDataTask) {
        bdbBaseURL = bdbBaseURL == null ? DEFAULT_BDB_BASE_URL : bdbBaseURL;

        bdbDataTask = bdbDataTask == null ? DEFAULT_DATA_TASK : bdbDataTask;

        ObjectCoder coder = ObjectCoder.createObjectCoderWithFailOnUnknownProperties();
        BdbApiClient bdbClient = new BdbApiClient(client, bdbBaseURL, bdbDataTask, coder);

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

    public static BlocksetSystemClient createForTest (OkHttpClient client,
                                              String bdbAuthToken) {
        return createForTest(client, bdbAuthToken, null);
    }

    public static BlocksetSystemClient createForTest (OkHttpClient client,
                                              String bdbAuthToken,
                                              @Nullable String bdbBaseURL) {
        DataTask brdDataTask = (cli, request, callback) -> {
            Request decoratedRequest = request.newBuilder()
                    .header("Authorization", "Bearer " + bdbAuthToken)
                    .build();
            cli.newCall(decoratedRequest).enqueue(callback);
        };
        return new BlocksetSystemClient (client, bdbBaseURL, brdDataTask);
    }

    /**
     * Cancel all client requests that are currently enqueued or executing
     */
    public void cancelAll () {
        client.dispatcher().cancelAll();
        // In a race, any Callable on any Executor might run NOW, causing a `client` request.
        // That is okay; we'll have some more data.  That is, it is no different from if the 
        // request had completed just before the `cancelAll()` call.
    }

    // Blockchain

    private void getBlockchains(CompletionHandler<List<Blockchain>, QueryError> handler) {
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

    public void getBlockchain(String blockchainId,
                              CompletionHandler<Blockchain, QueryError> handler) {
        blockchainApi.getBlockchain(
                blockchainId,
                handler
        );
    }

    // Currency

    private void getCurrencies(CompletionHandler<List<Currency>, QueryError> handler) {
        currencyApi.getCurrencies(
                handler
        );
    }

    private void getCurrencies(@Nullable String id,
                              CompletionHandler<List<Currency>, QueryError> handler) {
        currencyApi.getCurrencies(
                id,
                handler
        );
    }

    public void getCurrencies(@Nullable String blockchainId,
                              @Nullable Boolean isMainnet,
                              CompletionHandler<List<Currency>, QueryError> handler) {

        if (isMainnet != null) {
            currencyApi.getCurrencies(
                    blockchainId,
                    handler
            );
        } else {
            currencyApi.getCurrencies(
                    blockchainId,
                    isMainnet.booleanValue(),
                    handler
            );
        }
    }

    public void getCurrency(String currencyId,
                            CompletionHandler<Currency, QueryError> handler) {
        currencyApi.getCurrency(
                currencyId,
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

    public void getSubscription(String subscriptionId,
                                CompletionHandler<Subscription, QueryError> handler) {
        subscriptionApi.getSubscription(
                subscriptionId,
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
    private void getTransfers(String id,
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
    public void getTransfers(String blockchainId,
                             List<String> addresses,
                             @Nullable UnsignedLong beginBlockNumber,
                             @Nullable UnsignedLong endBlockNumber,
                             @Nullable Integer maxPageSize,
                             CompletionHandler<List<Transfer>, QueryError> handler) {
        transferApi.getTransfers(
                blockchainId,
                addresses,
                beginBlockNumber,
                endBlockNumber,
                maxPageSize,
                handler
        );
    }

    public void getTransfer(String transferId,
                            CompletionHandler<Transfer, QueryError> handler) {
        transferApi.getTransfer(
                transferId,
                handler
        );
    }

    // Transactions

    /* Throws 'IllegalArgumentException' if `addresses` is empty. */
    private void getTransactions(String id,
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
    public void getTransactions(String blockchainId,
                                List<String> addresses,
                                @Nullable UnsignedLong beginBlockNumber,
                                @Nullable UnsignedLong endBlockNumber,
                                boolean includeRaw,
                                boolean includeProof,
                                boolean includeTransfers,
                                @Nullable Integer maxPageSize,
                                CompletionHandler<List<Transaction>, QueryError> handler) {
        transactionApi.getTransactions(
                blockchainId,
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

    public void getTransaction(String transactionId,
                               boolean includeRaw,
                               boolean includeProof,
                               boolean includeTransfers,
                               CompletionHandler<Transaction, QueryError> handler) {
        transactionApi.getTransaction(
                transactionId,
                includeRaw,
                includeProof,
                includeTransfers,
                handler
        );
    }

    public void createTransaction(String blockchainId,
                                  byte[] tx,
                                  String identifier,
                                  CompletionHandler<TransactionIdentifier, QueryError> handler) {
        transactionApi.createTransaction(
                blockchainId,
                tx,
                identifier,
                handler
        );
    }

    public void estimateTransactionFee(String blockchainId,
                                       byte[] data,
                                       CompletionHandler<TransactionFee, QueryError> handler) {
        transactionApi.estimateTransactionFee(
                blockchainId,
                data,
                handler
        );
    }

    // Blocks

    public void getBlocks(String id,
                          UnsignedLong beginBlockNumber,
                          UnsignedLong endBlockNumber,
                          boolean includeRaw,
                          boolean includeTxRaw,
                          boolean includeTx,
                          boolean includeTxProof,
                          int maxPageSize,
                          CompletionHandler<List<Block>, QueryError> handler) {

        blockApi.getBlocks(
                id,
                beginBlockNumber,
                endBlockNumber,
                includeRaw,
                includeTx,
                includeTxRaw,
                includeTxProof,
                maxPageSize,
                handler);
    }

   /* public void getBlocks(String id,
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
    }*/

    /* NO USAGES ??
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
    } */

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

    /* NO USAGES ??
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
    } */

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
