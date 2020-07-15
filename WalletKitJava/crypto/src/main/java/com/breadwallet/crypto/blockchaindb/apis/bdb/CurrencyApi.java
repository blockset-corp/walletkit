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
import com.breadwallet.crypto.blockchaindb.models.bdb.Currency;
import com.breadwallet.crypto.utility.CompletionHandler;
import com.google.common.base.Optional;
import com.google.common.collect.ImmutableListMultimap;
import com.google.common.collect.ImmutableMultimap;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;

public class CurrencyApi {

    private final BdbApiClient jsonClient;
    private final ExecutorService executorService;

    public CurrencyApi(BdbApiClient jsonClient,
                       ExecutorService executorService) {
        this.jsonClient = jsonClient;
        this.executorService = executorService;
    }

    public void getCurrencies(CompletionHandler<List<Currency>, QueryError> handler) {
        getCurrencies(null, handler);
    }

    public void getCurrencies(@Nullable String id,
                              CompletionHandler<List<Currency>, QueryError> handler) {
        ImmutableListMultimap.Builder<String, String> paramsBuilder = ImmutableListMultimap.builder();
        if (id != null) paramsBuilder.put("blockchain_id", id);
        paramsBuilder.put("verified", "true");
        ImmutableMultimap<String, String> params = paramsBuilder.build();

        CompletionHandler<PagedData<Currency>, QueryError> pagedHandler = createPagedResultsHandler(handler);
        jsonClient.sendGetForArrayWithPaging("currencies", params, Currency.class, pagedHandler);
    }

    public void getCurrencies(boolean mainnet,
                              CompletionHandler<List<Currency>, QueryError> handler) {
        ImmutableListMultimap.Builder<String, String> paramsBuilder = ImmutableListMultimap.builder();
        paramsBuilder.put("testnet", (mainnet ? "false" : "true"));
        paramsBuilder.put("verified", "true");
        ImmutableMultimap<String, String> params = paramsBuilder.build();

        jsonClient.sendGetForArray("currencies", params, Currency.class, handler);
    }

    public void getCurrency(String id,
                            CompletionHandler<Currency, QueryError> handler) {
        jsonClient.sendGetWithId("currencies", id, ImmutableMultimap.of(), Currency.class, handler);
    }

    private void submitGetNextBlocks(String nextUrl, CompletionHandler<PagedData<Currency>, QueryError> handler) {
        executorService.submit(() -> getNextBlocks(nextUrl, handler));
    }

    private void getNextBlocks(String nextUrl, CompletionHandler<PagedData<Currency>, QueryError> handler) {
        jsonClient.sendGetForArrayWithPaging("blocks", nextUrl, Currency.class, handler);
    }

    private CompletionHandler<PagedData<Currency>, QueryError> createPagedResultsHandler(CompletionHandler<List<Currency>, QueryError> handler) {
        List<Currency> allResults = new ArrayList<>();
        return new CompletionHandler<PagedData<Currency>, QueryError>() {
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
}
