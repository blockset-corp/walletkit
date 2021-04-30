/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.blockchaindb.apis.bdb;

import com.blockset.walletkit.blockchaindb.errors.QueryError;
import com.blockset.walletkit.blockchaindb.models.bdb.Blockchain;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.collect.ImmutableListMultimap;
import com.google.common.collect.ImmutableMultimap;
import com.google.common.collect.Multimap;

import java.util.List;

public class BlockchainApi {

    private final BdbApiClient jsonClient;

    public BlockchainApi(BdbApiClient jsonClient) {
        this.jsonClient = jsonClient;
    }

    public void getBlockchains(boolean isMainnet,
                               CompletionHandler<List<Blockchain>, QueryError> handler) {
        ImmutableListMultimap.Builder<String, String> paramsBuilder = ImmutableListMultimap.builder();
        paramsBuilder.put("testnet", Boolean.valueOf(!isMainnet).toString());
        paramsBuilder.put("verified", "true");
        ImmutableMultimap<String, String> params = paramsBuilder.build();

        jsonClient.sendGetForArray("blockchains", params, Blockchain.class, handler);
    }

    public void getBlockchain(String id,
                              CompletionHandler<Blockchain, QueryError> handler) {
        Multimap<String, String> params = ImmutableListMultimap.of("verified", "true");
        jsonClient.sendGetWithId("blockchains", id, params, Blockchain.class, handler);
    }
}
