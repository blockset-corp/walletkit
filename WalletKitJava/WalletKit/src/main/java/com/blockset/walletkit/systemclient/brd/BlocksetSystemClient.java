/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.systemclient.brd;

import android.support.annotation.Nullable;

import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.errors.QueryNoDataError;
import com.blockset.walletkit.errors.QueryError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.base.Optional;
import com.google.common.collect.ImmutableListMultimap;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableMultimap;
import com.google.common.collect.Lists;
import com.google.common.collect.Multimap;
import com.google.common.io.BaseEncoding;
import com.google.common.primitives.UnsignedLong;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import okhttp3.OkHttpClient;
import okhttp3.Request;

import static com.google.common.base.Preconditions.checkNotNull;

public class BlocksetSystemClient implements SystemClient {

    private static final int ADDRESS_COUNT = 50;
    private static final int DEFAULT_MAX_PAGE_SIZE = 20;
    private static final String DEFAULT_BDB_BASE_URL = "https://api.blockset.com";
    private static final DataTask DEFAULT_DATA_TASK = (cli, request, callback) -> cli.newCall(request).enqueue(callback);
    private static final List<String> resourcePathAccounts =
            Arrays.asList("_experimental", "hedera", "accounts");

    private final AtomicInteger ridGenerator;

    private final OkHttpClient client;
    private final BdbApiClient bdbClient;
    private final ExecutorService apiExecutor;
    private final ScheduledExecutorService scheduledApiExecutor;

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
        bdbClient = new BdbApiClient(client, bdbBaseURL, bdbDataTask, coder);

        apiExecutor = Executors.newCachedThreadPool();
        scheduledApiExecutor = Executors.newSingleThreadScheduledExecutor();

        this.ridGenerator = new AtomicInteger(0);

        this.client = client;
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
    @Override
    public void cancelAll () {
        client.dispatcher().cancelAll();
        // In a race, any Callable on any Executor might run NOW, causing a `client` request.
        // That is okay; we'll have some more data.  That is, it is no different from if the 
        // request had completed just before the `cancelAll()` call.
    }

    // Blockchain
    @Override
    public void getBlockchains(boolean isMainnet,
                               CompletionHandler<List<Blockchain>, QueryError> handler) {
        ImmutableListMultimap.Builder<String, String> paramsBuilder = ImmutableListMultimap.builder();
        paramsBuilder.put("testnet", Boolean.valueOf(!isMainnet).toString());
        paramsBuilder.put("verified", "true");
        ImmutableMultimap<String, String> params = paramsBuilder.build();

        bdbClient.sendGetForArray("blockchains", params, BlocksetBlockchain.class, handler);
    }

    @Override
    public void getBlockchain(String blockchainId,
                              CompletionHandler<Blockchain, QueryError> handler) {
        Multimap<String, String> params = ImmutableListMultimap.of("verified", "true");
        bdbClient.sendGetWithId("blockchains", blockchainId, params, BlocksetBlockchain.class, handler);
    }

    // Currency

    private CompletionHandler<PagedData<Currency>, QueryError> createPagedResultsHandler(CompletionHandler<List<Currency>, QueryError> handler) {
        List<Currency> allResults = new ArrayList<>();
        return new CompletionHandler<PagedData<Currency>, QueryError>() {

            private void submitGetNextBlocks(String nextUrl, CompletionHandler<PagedData<Currency>, QueryError> handler) {
                apiExecutor.submit(() -> getNextBlocks(nextUrl, handler));
            }

            private void getNextBlocks(String nextUrl, CompletionHandler<PagedData<Currency>, QueryError> handler) {
                bdbClient.sendGetForArrayWithPaging("blocks", nextUrl, BlocksetCurrency.class, handler);
            }

            @Override
            public void handleData(PagedData<Currency> results) {
                Optional<String> nextUrl = results.getNextUrl();
                allResults.addAll(results.getData());

                if (nextUrl.isPresent()) {
                    submitGetNextBlocks(nextUrl.get(), this);

                } else {
                    handler.handleData(allResults);
                }
            }

            @Override
            public void handleError(QueryError error) {
                handler.handleError(error);
            }
        };
    }

    @Override
    public void getCurrencies(@Nullable String blockchainId,
                              @Nullable Boolean isMainnet,
                              CompletionHandler<List<Currency>, QueryError> handler) {

        ImmutableListMultimap.Builder<String, String> paramsBuilder = ImmutableListMultimap.builder();
        if (blockchainId != null)
            paramsBuilder.put("blockchain_id", blockchainId);
        if (isMainnet != null)
            paramsBuilder.put("testnet", (isMainnet ? "false" : "true"));
        paramsBuilder.put("verified", "true");
        ImmutableMultimap<String, String> params = paramsBuilder.build();

        CompletionHandler<PagedData<Currency>, QueryError> pagedHandler = createPagedResultsHandler(handler);
        bdbClient.sendGetForArrayWithPaging("currencies", params, BlocksetCurrency.class, pagedHandler);
    }

    @Override
    public void getCurrency(String currencyId,
                            CompletionHandler<Currency, QueryError> handler) {
        bdbClient.sendGetWithId("currencies", currencyId, ImmutableMultimap.of(), BlocksetCurrency.class, handler);
    }

    // Subscription
    @Override
    public void getOrCreateSubscription(Subscription subscription,
                                        CompletionHandler<Subscription, QueryError> handler) {
        getSubscription(subscription.getId(), new CompletionHandler<Subscription, QueryError>() {
            @Override
            public void handleData(Subscription data) {
                handler.handleData(data);
            }

            @Override
            public void handleError(QueryError error) {
                createSubscription(subscription.getDevice(), subscription.getEndpoint(),
                                   subscription.getCurrencies(), handler);
            }
        });
    }

    @Override
    public void getSubscription(String subscriptionId,
                                CompletionHandler<Subscription, QueryError> handler) {
        bdbClient.sendGetWithId("subscriptions", subscriptionId, ImmutableMultimap.of(),
                                 BlocksetSubscription.class, handler);
    }

    @Override
    public void getSubscriptions(CompletionHandler<List<Subscription>, QueryError> handler) {
        bdbClient.sendGetForArray("subscriptions", ImmutableMultimap.of(),
                                   BlocksetSubscription.class, handler);
    }

    @Override
    public void createSubscription(String deviceId,
                                   SubscriptionEndpoint endpoint,
                                   List<SubscriptionCurrency> currencies,
                                   CompletionHandler<Subscription, QueryError> handler) {
        bdbClient.sendPost("subscriptions", ImmutableMultimap.of(),
                            NewSubscription.create(deviceId, endpoint, currencies),
                            BlocksetSubscription.class, handler);
    }

    @Override
    public void updateSubscription(Subscription subscription,
                                   CompletionHandler<Subscription, QueryError> handler) {
        bdbClient.sendPutWithId("subscriptions", subscription.getId(), ImmutableMultimap.of(),
                                 subscription, BlocksetSubscription.class, handler);
    }

    @Override
    public void deleteSubscription(String id,
                                   CompletionHandler<Void, QueryError> handler) {
        bdbClient.sendDeleteWithId("subscriptions", id, ImmutableMultimap.of(), handler);
    }

    // Transfer

    private CompletionHandler<PagedData<Transfer>, QueryError> createPagedTransferResultsHandler(
            GetChunkedCoordinator<String, Transfer> coordinator,
            List<String> chunkedAddresses) {

        List<Transfer> allResults = new ArrayList<>();
        return new CompletionHandler<PagedData<Transfer>, QueryError>() {

            private void getTransfer(String id,
                                    CompletionHandler<Transfer, QueryError> handler) {
                bdbClient.sendGetWithId("transfers", id, ImmutableMultimap.of(), BlocksetTransfer.class, handler);
            }

            private void getNextTransfers(String nextUrl,
                                          CompletionHandler<PagedData<Transfer>, QueryError> handler) {
                bdbClient.sendGetForArrayWithPaging("transfers", nextUrl, BlocksetTransfer.class, handler);
            }

            private void submitGetNextTransfers(String nextUrl,
                                                CompletionHandler<PagedData<Transfer>, QueryError> handler) {
                apiExecutor.submit(() -> getNextTransfers(nextUrl, handler));
            }

            @Override
            public void handleData(PagedData<Transfer> results) {
                Optional<String> nextUrl = results.getNextUrl();
                allResults.addAll(results.getData());

                if (nextUrl.isPresent()) {
                    submitGetNextTransfers(nextUrl.get(), this);

                } else {
                    coordinator.handleChunkData(chunkedAddresses, allResults);
                }
            }

            @Override
            public void handleError(QueryError error) {
                coordinator.handleError(error);
            }
        };
    }

    /* Throws 'IllegalArgumentException' if `addresses` is empty. */
    @Override
    public void getTransfers(String blockchainId,
                             List<String> addresses,
                             @Nullable UnsignedLong beginBlockNumber,
                             @Nullable UnsignedLong endBlockNumber,
                             @Nullable Integer maxPageSize,
                             CompletionHandler<List<Transfer>, QueryError> handler) {
        if (addresses.isEmpty())
            throw new IllegalArgumentException("Empty `addresses`");

        List<List<String>> chunkedAddressesList = Lists.partition(addresses, ADDRESS_COUNT);
        GetChunkedCoordinator<String, Transfer> coordinator = new GetChunkedCoordinator<>(chunkedAddressesList, handler);

        if (null == maxPageSize) maxPageSize = DEFAULT_MAX_PAGE_SIZE;

        for (int i = 0; i < chunkedAddressesList.size(); i++) {
            List<String> chunkedAddresses = chunkedAddressesList.get(i);

            ImmutableListMultimap.Builder<String, String> paramsBuilder = ImmutableListMultimap.builder();
            paramsBuilder.put("blockchain_id", blockchainId);
            if (beginBlockNumber != null) paramsBuilder.put("start_height", beginBlockNumber.toString());
            if (endBlockNumber   != null) paramsBuilder.put("end_height",   endBlockNumber.toString());
            paramsBuilder.put("max_page_size", maxPageSize.toString());
            for (String address : chunkedAddresses) paramsBuilder.put("address", address);
            ImmutableMultimap<String, String> params = paramsBuilder.build();

            CompletionHandler<PagedData<Transfer>, QueryError> pagedHandler = createPagedTransferResultsHandler(coordinator, chunkedAddresses);
            bdbClient.sendGetForArrayWithPaging("transfers", params, BlocksetTransfer.class, pagedHandler);
        }
    }

    @Override
    public void getTransfer(String transferId,
                            CompletionHandler<Transfer, QueryError> handler) {

        bdbClient.sendGetWithId("transfers", transferId, ImmutableMultimap.of(), BlocksetTransfer.class, handler);
    }

    // Transactions

    private CompletionHandler<PagedData<Transaction>, QueryError> createPagedTransactionResultsHandler(GetChunkedCoordinator<String, Transaction> coordinator,
                                                                                                       List<String> chunkedAddresses) {
        List<Transaction> allResults = new ArrayList<>();
        return new CompletionHandler<PagedData<Transaction>, QueryError>() {

            private void getNextTransactions(String nextUrl,
                                             CompletionHandler<PagedData<Transaction>, QueryError> handler) {
                bdbClient.sendGetForArrayWithPaging("transactions", nextUrl, BlocksetTransaction.class, handler);
            }

            private void submitGetNextTransactions(String nextUrl,
                                                   CompletionHandler<PagedData<Transaction>, QueryError> handler) {
                apiExecutor.submit(() -> getNextTransactions(nextUrl, handler));
            }

            boolean transactionStatusIsValid(Transaction transaction) {
                switch (transaction.getStatus()) {
                    case "confirmed":
                    case "submitted":
                    case "failed":
                        return true;
                    case "reverted":
                        return bdbClient.capabilities.hasCapabilities (BdbApiClient.Capabilities.transferStatusRevert);
                    case "rejected":
                        return bdbClient.capabilities.hasCapabilities (BdbApiClient.Capabilities.transferStatusReject);
                    default:
                        return false;
                }
            }

            boolean transactionsAreAllValid (List<Transaction> transactions) {
                for (Transaction transaction : transactions) {
                    if (!transactionStatusIsValid(transaction))
                        return false;
                }
                return true;
            }

            @Override
            public void handleData(PagedData<Transaction> results) {
                Optional<String> nextUrl = results.getNextUrl();
                allResults.addAll(results.getData());

                if (nextUrl.isPresent()) {
                    submitGetNextTransactions(nextUrl.get(), this);
                } else if (!transactionsAreAllValid(allResults)) {
                    coordinator.handleError(new QueryJsonParseError());
                } else {
                    coordinator.handleChunkData(chunkedAddresses, allResults);
                }
            }

            @Override
            public void handleError(QueryError error) {
                coordinator.handleError(error);
            }
        };
    }

    /* Throws 'IllegalArgumentException' if `addresses` is empty. */
    @Override
    public void getTransactions(String blockchainId,
                                List<String> addresses,
                                @Nullable UnsignedLong beginBlockNumber,
                                @Nullable UnsignedLong endBlockNumber,
                                boolean includeRaw,
                                boolean includeProof,
                                boolean includeTransfers,
                                @Nullable Integer maxPageSize,
                                CompletionHandler<List<Transaction>, QueryError> handler) {
        if (addresses.isEmpty())
            throw new IllegalArgumentException("Empty `addresses`");

        List<List<String>> chunkedAddressesList = Lists.partition(addresses, ADDRESS_COUNT);
        GetChunkedCoordinator<String, Transaction> coordinator = new GetChunkedCoordinator<>(chunkedAddressesList, handler);

        if (null == maxPageSize) maxPageSize = (includeTransfers ? 1 : 3) * DEFAULT_MAX_PAGE_SIZE;

        for (int i = 0; i < chunkedAddressesList.size(); i++) {
            List<String> chunkedAddresses = chunkedAddressesList.get(i);

            ImmutableListMultimap.Builder<String, String> paramsBuilder = ImmutableListMultimap.builder();
            paramsBuilder.put("blockchain_id", blockchainId);
            paramsBuilder.put("include_proof", String.valueOf(includeProof));
            paramsBuilder.put("include_raw", String.valueOf(includeRaw));
            paramsBuilder.put("include_transfers", String.valueOf(includeTransfers));
            paramsBuilder.put("include_calls", "false");
            if (beginBlockNumber != null) paramsBuilder.put("start_height", beginBlockNumber.toString());
            if (endBlockNumber != null) paramsBuilder.put("end_height", endBlockNumber.toString());
            paramsBuilder.put("max_page_size", maxPageSize.toString());
            for (String address : chunkedAddresses) paramsBuilder.put("address", address);
            ImmutableMultimap<String, String> params = paramsBuilder.build();

            CompletionHandler<PagedData<Transaction>, QueryError> pagedHandler = createPagedTransactionResultsHandler(coordinator, chunkedAddresses);
            bdbClient.sendGetForArrayWithPaging("transactions", params, BlocksetTransaction.class, pagedHandler);
        }
    }

    @Override
    public void getTransaction(String transactionId,
                               boolean includeRaw,
                               boolean includeProof,
                               boolean includeTransfers,
                               CompletionHandler<Transaction, QueryError> handler) {
        Multimap<String, String> params = ImmutableListMultimap.of(
                "include_proof", String.valueOf(includeProof),
                "include_raw", String.valueOf(includeRaw),
                "include_transfers", String.valueOf(includeTransfers),
                "include_calls", "false");

        bdbClient.sendGetWithId("transactions", transactionId, params, BlocksetTransaction.class, handler);
    }

    @Override
    public void createTransaction(String blockchainId,
                                  byte[] tx,
                                  String identifier,
                                  CompletionHandler<TransactionIdentifier, QueryError> handler) {
        String data = BaseEncoding.base64().encode(tx);
        Map json = ImmutableMap.of(
                "blockchain_id", blockchainId,
                "submit_context", String.format ("WalletKit:%s:%s", blockchainId, (null != identifier ? identifier : ("Data:" + data.substring(0,20)))),
                "data", data);

        bdbClient.sendPost("transactions", ImmutableMultimap.of(), json, BlocksetTransactionIdentifier.class, handler);
    }

    @Override
    public void estimateTransactionFee(String blockchainId,
                                       byte[] data,
                                       CompletionHandler<TransactionFee, QueryError> handler) {
        Multimap<String, String> params = ImmutableListMultimap.of(
                "estimate_fee", "true");

        String sdata = BaseEncoding.base64().encode(data);
        Map json = ImmutableMap.of(
                "blockchain_id", blockchainId,
                "submit_context", String.format("WalletKit:%s:Data:%s (FeeEstimate)", blockchainId, sdata.substring(0, 20)),
                "data", data);

        bdbClient.sendPost("transactions", params, json, BlocksetTransactionFee.class, handler);
    }

    // Blocks

    private CompletionHandler<PagedData<BlocksetBlock>, QueryError> createPagedBlockResultsHandler(CompletionHandler<List<Block>, QueryError> handler) {
        List<Block> allResults = new ArrayList<>();
        return new CompletionHandler<PagedData<BlocksetBlock>, QueryError>() {

            private void getNextBlocks(String nextUrl, CompletionHandler<PagedData<BlocksetBlock>, QueryError> handler) {
                bdbClient.sendGetForArrayWithPaging("blocks", nextUrl, BlocksetBlock.class, handler);
            }

            private void submitGetNextBlocks(String nextUrl, CompletionHandler<PagedData<BlocksetBlock>, QueryError> handler) {
                apiExecutor.submit(() -> getNextBlocks(nextUrl, handler));
            }

            @Override
            public void handleData(PagedData<BlocksetBlock> results) {
                Optional<String> nextUrl = results.getNextUrl();
                allResults.addAll(results.getData());

                if (nextUrl.isPresent()) {
                    submitGetNextBlocks(nextUrl.get(), this);

                } else {
                    handler.handleData(allResults);
                }
            }

            @Override
            public void handleError(QueryError error) {
                handler.handleError(error);
            }
        };
    }

    @Override
    public void getBlocks(String blockchainId,
                          UnsignedLong beginBlockNumber,
                          UnsignedLong endBlockNumber,
                          boolean includeRaw,
                          boolean includeTxRaw,
                          boolean includeTx,
                          boolean includeTxProof,
                          @Nullable Integer maxPageSize,
                          CompletionHandler<List<Block>, QueryError> handler) {

        ImmutableListMultimap.Builder<String, String> paramsBuilder = ImmutableListMultimap.builder();
        paramsBuilder.put("blockchain_id", blockchainId);
        paramsBuilder.put("include_raw", String.valueOf(includeRaw));
        paramsBuilder.put("include_tx", String.valueOf(includeTx));
        paramsBuilder.put("include_tx_raw", String.valueOf(includeTxRaw));
        paramsBuilder.put("include_tx_proof", String.valueOf(includeTxProof));
        paramsBuilder.put("start_height", beginBlockNumber.toString());
        paramsBuilder.put("end_height", endBlockNumber.toString());
        if (null != maxPageSize)
            paramsBuilder.put("max_page_size", maxPageSize.toString());
        ImmutableMultimap<String, String> params = paramsBuilder.build();

        CompletionHandler<PagedData<BlocksetBlock>, QueryError> pagedHandler = createPagedBlockResultsHandler(handler);
        bdbClient.sendGetForArrayWithPaging("blocks", params, BlocksetBlock.class, pagedHandler);
    }

    @Override
    public void getBlock(String id,
                         boolean includeRaw,
                         boolean includeTx,
                         boolean includeTxRaw,
                         boolean includeTxProof,
                         CompletionHandler<Block, QueryError> handler) {
        Multimap<String, String> params = ImmutableListMultimap.of(
                "include_raw", String.valueOf(includeRaw),
                "include_tx", String.valueOf(includeTx),
                "include_tx_raw", String.valueOf(includeTxRaw),
                "include_tx_proof", String.valueOf(includeTxProof));

        bdbClient.sendGetWithId("blocks", id, params, BlocksetBlock.class, handler);
    }

    // Addresses

    // Experimental - Hedera Account Creation
    private static class NewHederaAccount {

        private final String blockchainId;
        private final String publicKey;

        @JsonCreator
        public static NewHederaAccount create(@JsonProperty("blockchain_id") String blockchainId,
                                              @JsonProperty("pub_key") String publicKey) {
            return new NewHederaAccount(
                    checkNotNull(blockchainId),
                    checkNotNull(publicKey)
            );
        }

        private NewHederaAccount(String blockchainId,
                                 String publicKey) {
            this.blockchainId = blockchainId;
            this.publicKey = publicKey;
        }

        @JsonProperty("blockchain_id")
        public String getBlockchainId() {
            return blockchainId;
        }

        @JsonProperty("pub_key")
        public String getPublicKey() {
            return publicKey;
        }
    }

    private static class HederaTransaction {
        @JsonCreator
        public static HederaTransaction create(@JsonProperty("account_id") String accountId,
                                               @JsonProperty("transaction_id") String transactionId,
                                               @JsonProperty("transaction_status") String transactionStatus) {
            return new HederaTransaction(
                    accountId,
                    transactionId,
                    transactionStatus
            );
        }

        private final String accountId;
        private final String transactionId;
        private final String transactionStatus;

        public HederaTransaction(String accountId,
                                 String transactionId,
                                 String transactionStatus) {
            this.accountId = accountId;
            this.transactionId = transactionId;
            this.transactionStatus = transactionStatus;
        }

        @JsonProperty("account_id")
        public String getAccountId() {
            return accountId;
        }

        public String getTransactionId() {
            return transactionId;
        }

        @JsonProperty("transaction_status")
        public String getTransactionStatus() {
            return transactionStatus;
        }
    }

    private class HederaRetryCompletionHandler implements CompletionHandler<List<HederaAccount>, QueryError> {
        final long retryPeriodInSeconds = 5;
        final long retryDurationInSeconds = 4 * 60;
        long retriesRemaining = (retryDurationInSeconds / retryPeriodInSeconds) - 1;

        String id;
        String publicKey;
        CompletionHandler<List<HederaAccount>, QueryError> handler;

        HederaRetryCompletionHandler (String id, String publicKey, CompletionHandler<List<HederaAccount>, QueryError> handler) {
            this.id = id;
            this.publicKey = publicKey;
            this.handler = handler;
        }

        private void retryIfAppropriate () {
            if (0 == retriesRemaining) handler.handleError(new QueryNoDataError());
            else {
                retriesRemaining -= 1;
                scheduledApiExecutor.schedule(
                        () -> getHederaAccount (id, publicKey, this),
                        retryPeriodInSeconds,
                        TimeUnit.SECONDS);
            }
        }

        @Override
        public void handleData(List<HederaAccount> accounts) {
            if (accounts.isEmpty()) retryIfAppropriate();
            else {
                handler.handleData(accounts);
            }
        }

        @Override
        public void handleError(QueryError error) {
            // Ignore the error and try again.  The rationale being: the POST to create the
            // Hedera Account succeeded (because we are here in the first place), so we might as
            // well just keep trying to get the actual Hedera Account.
            retryIfAppropriate();
        }
    }

    @Override
    public void getHederaAccount(String blockchainId,
                                 String publicKey,
                                 CompletionHandler<List<HederaAccount>, QueryError> handler) {
        bdbClient.sendGetForArray(
                resourcePathAccounts,
                "accounts",
                ImmutableListMultimap.of(
                        "blockchain_id", blockchainId,
                        "pub_key", publicKey),
                BlocksetHederaAccount.class,
                handler);
    }

    @Override
    public void createHederaAccount(String id,
                                    String publicKey,
                                    CompletionHandler<List<HederaAccount>, QueryError> handler) {
        bdbClient.sendPost(
                resourcePathAccounts,
                ImmutableMultimap.of(),
                NewHederaAccount.create(id, publicKey),
                HederaTransaction.class,
                new CompletionHandler<HederaTransaction, QueryError>() {

                    private void getHederaAccountForTransaction(String id,
                                                                String publicKey,
                                                                HederaTransaction transaction,
                                                                CompletionHandler<List<HederaAccount>, QueryError> handler) {
                        // We don't actually use the `transactionID` through the `GET .../account_transactions`
                        // endpoint.  It is more direct to just repeatedly "GET .../accounts"
                        // final String transactionId = id + ":" + transaction.getTransactionId();

                        final long initialDelayInSeconds = 2;

                        scheduledApiExecutor.schedule(
                                () -> getHederaAccount(id, publicKey, new HederaRetryCompletionHandler(id, publicKey, handler)),
                                initialDelayInSeconds,
                                TimeUnit.SECONDS);
                    }

                    @Override
                    public void handleData(HederaTransaction transaction) {
                        getHederaAccountForTransaction(id, publicKey, transaction, handler);
                    }

                    @Override
                    public void handleError(QueryError error) {
                        if (error instanceof QueryResponseError && 422 == ((QueryResponseError) error).getStatusCode())
                            getHederaAccount(id, publicKey, handler);
                        else
                            handler.handleError(error);
                    }
                });
    }
}
