/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.crypto.blockchaindb.apis.bdb;

import android.support.annotation.Nullable;

import com.breadwallet.crypto.blockchaindb.apis.PagedData;
import com.breadwallet.crypto.blockchaindb.errors.QueryError;
import com.breadwallet.crypto.blockchaindb.models.bdb.Transaction;
import com.breadwallet.crypto.blockchaindb.models.bdb.TransactionFee;
import com.breadwallet.crypto.blockchaindb.models.bdb.TransactionIdentifier;
import com.breadwallet.crypto.utility.CompletionHandler;
import com.google.common.base.Optional;
import com.google.common.collect.ImmutableListMultimap;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableMultimap;
import com.google.common.collect.Lists;
import com.google.common.collect.Multimap;
import com.google.common.io.BaseEncoding;
import com.google.common.primitives.UnsignedLong;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;

public class TransactionApi {

    private static final int ADDRESS_COUNT = 50;
    private static final int DEFAULT_MAX_PAGE_SIZE = 20;

    private final BdbApiClient jsonClient;
    private final ExecutorService executorService;

    public TransactionApi(BdbApiClient jsonClient,
                          ExecutorService executorService) {
        this.jsonClient = jsonClient;
        this.executorService = executorService;
    }

    public void getTransactions(String id,
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
            paramsBuilder.put("blockchain_id", id);
            paramsBuilder.put("include_proof", String.valueOf(includeProof));
            paramsBuilder.put("include_raw", String.valueOf(includeRaw));
            paramsBuilder.put("include_transfers", String.valueOf(includeTransfers));
            paramsBuilder.put("include_calls", "false");
            if (beginBlockNumber != null) paramsBuilder.put("start_height", beginBlockNumber.toString());
            if (endBlockNumber != null) paramsBuilder.put("end_height", endBlockNumber.toString());
            paramsBuilder.put("max_page_size", maxPageSize.toString());
            for (String address : chunkedAddresses) paramsBuilder.put("address", address);
            ImmutableMultimap<String, String> params = paramsBuilder.build();

            CompletionHandler<PagedData<Transaction>, QueryError> pagedHandler = createPagedResultsHandler(coordinator, chunkedAddresses);
            jsonClient.sendGetForArrayWithPaging("transactions", params, Transaction.class, pagedHandler);
        }
    }

    public void getTransaction(String id,
                               boolean includeRaw,
                               boolean includeProof,
                               boolean includeTransfers,
                               CompletionHandler<Transaction, QueryError> handler) {
        Multimap<String, String> params = ImmutableListMultimap.of(
                "include_proof", String.valueOf(includeProof),
                "include_raw", String.valueOf(includeRaw),
                "include_transfers", String.valueOf(includeTransfers),
                "include_calls", "false");

        jsonClient.sendGetWithId("transactions", id, params, Transaction.class, handler);
    }

    public void createTransaction(String id,
                                  String hashAsHex,
                                  byte[] tx,
                                  CompletionHandler<TransactionIdentifier, QueryError> handler) {
        Map json = ImmutableMap.of(
                "blockchain_id", id,
                "transaction_id", hashAsHex,
                "data", BaseEncoding.base64().encode(tx));

        jsonClient.sendPost("transactions", ImmutableMultimap.of(), json, TransactionIdentifier.class, handler);
    }

    public void estimateFee(String id,
                                  String hashAsHex,
                                  byte[] tx,
                                  CompletionHandler<TransactionFee, QueryError> handler) {
        Multimap<String, String> params = ImmutableListMultimap.of(
                "estimate_fee", "true");
        Map json = ImmutableMap.of(
                "blockchain_id", id,
                "transaction_id", hashAsHex,
                "data", BaseEncoding.base64().encode(tx));

        jsonClient.sendPost("transactions", params, json, TransactionFee.class, handler);
    }

    private CompletionHandler<PagedData<Transaction>, QueryError> createPagedResultsHandler(GetChunkedCoordinator<String, Transaction> coordinator,
                                                                                       List<String> chunkedAddresses) {
        List<Transaction> allResults = new ArrayList<>();
        return new CompletionHandler<PagedData<Transaction>, QueryError>() {
            @Override
            public void handleData(PagedData<Transaction> results) {
                Optional<String> nextUrl = results.getNextUrl();
                allResults.addAll(results.getData());

                if (nextUrl.isPresent()) {
                    submitGetNextTransactions(nextUrl.get(), this);

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

    private void submitGetNextTransactions(String nextUrl,
                                           CompletionHandler<PagedData<Transaction>, QueryError> handler) {
        executorService.submit(() -> getNextTransactions(nextUrl, handler));
    }

    private void getNextTransactions(String nextUrl,
                                     CompletionHandler<PagedData<Transaction>, QueryError> handler) {
        jsonClient.sendGetForArrayWithPaging("transactions", nextUrl, Transaction.class, handler);
    }
}
