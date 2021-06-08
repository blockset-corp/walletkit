/*
 * System
 *
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 6/02/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit;

import android.support.annotation.Nullable;

import com.blockset.walletkit.errors.QueryError;
import com.blockset.walletkit.systemclient.Block;
import com.blockset.walletkit.systemclient.Blockchain;
import com.blockset.walletkit.systemclient.Currency;
import com.blockset.walletkit.systemclient.HederaAccount;
import com.blockset.walletkit.systemclient.Subscription;
import com.blockset.walletkit.systemclient.SubscriptionCurrency;
import com.blockset.walletkit.systemclient.SubscriptionEndpoint;
import com.blockset.walletkit.systemclient.Transaction;
import com.blockset.walletkit.systemclient.TransactionFee;
import com.blockset.walletkit.systemclient.TransactionIdentifier;
import com.blockset.walletkit.systemclient.Transfer;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.primitives.UnsignedLong;

import java.util.List;

public interface SystemClient {

    /**
     * Cancel all client requests that are currently enqueued or executing
     */
    public void cancelAll();

    /**
     * Gets a list of blockchains. Results are directed to the specified completion handler
     *
     * @param isMainnet Indicates preference of main or testnet
     * @param handler   The handler for receiving retrieved {@link com.blockset.walletkit.systemclient.Blockchain}'s
     */
    public void getBlockchains(boolean isMainnet,
                               CompletionHandler<List<Blockchain>, QueryError> handler);

    /**
     * Gets a specific blockchain as identified by the particular id.
     * Results are directed to the specified completion handler
     *
     * @param blockchainId The identification for the blockchain of interest
     * @param handler      The handler for retrieved {@link com.blockset.walletkit.systemclient.Blockchain Blockchain}
     */
    public void getBlockchain(String blockchainId,
                              CompletionHandler<Blockchain, QueryError> handler);

    /**
     * Gets a list of currencies for the particular main/testnet blockchain.
     * Results are directed to the specified completion handler
     *
     * @param blockchainId Optional identification for the blockchain of interest
     * @param isMainnet Optional preference of main vs test net. If not provided, by default,
     *                  currencies for both networks are retrieved
     * @param handler      The handler for retrieved {@link com.blockset.walletkit.Currency Currency}
     */
    public void getCurrencies(@Nullable String blockchainId,
                              @Nullable Boolean isMainnet,
                              CompletionHandler<List<Currency>, QueryError> handler);

    /**
     * Gets a specific Currency designated by the currency identifier
     * Results are directed to the specified completion handler
     *
     * @param currencyId The identification for the currency of interest
     * @param handler    The handler for retrieved {@link com.blockset.walletkit.Currency Currency}
     */
    public void getCurrency(String currencyId,
                            CompletionHandler<Currency, QueryError> handler);

    /**
     * Gets a list of transfers for a particular list of addresses within a
     * particular blockchain.
     * Results are directed to the specified completion handler
     *
     * @param blockchainId     The identification for the blockchain of interest
     * @param addresses        A list of addresses which must contain one or more valid address strings
     * @param beginBlockNumber Optional starting block number
     * @param endBlockNumber   Optional ending block number
     * @param maxPageSize      Optional maximum page size
     * @param handler          The handler for retrieved {@link com.blockset.walletkit.Transfer Transfer}
     * @throws IllegalArgumentException If addresses list is empty
     */
    public void getTransfers(String blockchainId,
                             List<String> addresses,
                             @Nullable UnsignedLong beginBlockNumber,
                             @Nullable UnsignedLong endBlockNumber,
                             @Nullable Integer maxPageSize,
                             CompletionHandler<List<Transfer>, QueryError> handler);

    /**
     * Gets a specific transfer identified by the transfer identifier.
     *
     * @param transferId The transfer identifier
     * @param handler    The handler for retrieved {@link com.blockset.walletkit.Transfer Transfer}
     */
    public void getTransfer(String transferId,
                            CompletionHandler<Transfer, QueryError> handler);

    /**
     * Gets a list of transactions for the specified blockchain.
     *
     * @param blockchainId     The identifier for the blockchain of interest
     * @param addresses        A list of addresses which must contain one or more valid address strings
     * @param beginBlockNumber An optional starting height
     * @param endBlockNumber   An optional ending height
     * @param includeRaw       Indicates to raw data of transaction in the result Transaction
     * @param includeProof     Indicates to include transaction proof in the result Transactions
     * @param includeTransfers Indicates to include transfers in the result Transactions
     * @param maxPageSize      The maximum page size or maximum numer of transactions to be included in the result
     * @param handler          The handler for retrieved {@link com.blockset.walletkit.systemclient.Transaction Transaction}'s
     * @throws IllegalArgumentException If addresses list is empty
     */
    public void getTransactions(String blockchainId,
                                List<String> addresses,
                                @Nullable UnsignedLong beginBlockNumber,
                                @Nullable UnsignedLong endBlockNumber,
                                boolean includeRaw,
                                boolean includeProof,
                                boolean includeTransfers,
                                @Nullable Integer maxPageSize,
                                CompletionHandler<List<Transaction>, QueryError> handler);

    /**
     * Get a specific transaction referenced by transaction identifier.
     *
     * @param transactionId    The transaction unique identifier
     * @param includeRaw       Indicates to raw data of transaction in the result Transaction
     * @param includeProof     Indicates to include transaction proof in the result Transactions
     * @param includeTransfers Indicates to include transfers in the result Transactions
     * @param handler          The handler for retrieved {@link com.blockset.walletkit.systemclient.Transaction Transaction}
     */
    public void getTransaction(String transactionId,
                               boolean includeRaw,
                               boolean includeProof,
                               boolean includeTransfers,
                               CompletionHandler<Transaction, QueryError> handler);

    /**
     * Creates a transaction from raw data, on the specified blockchain
     *
     * @param blockchainId The identifier for blockchain to create the transaction on
     * @param data         The raw transaction bytes suitable to the underlying blockchain
     * @param identifier   The unique identifier of the transaction
     * @param handler      The handler for the created {@link com.blockset.walletkit.systemclient.TransactionIdentifier TransactionIdentifier}
     */
    public void createTransaction(String blockchainId,
                                  byte[] data,
                                  String identifier,
                                  CompletionHandler<TransactionIdentifier, QueryError> handler);

    /**
     * Estimate transaction fees for the specified blockchain
     *
     * @param blockchainId The identifier of the blockchain to get transaction fees
     * @param data         Raw transaction bytes for the particular blockchain
     * @param handler      The handler for the created {@link com.blockset.walletkit.systemclient.TransactionFee}
     */
    public void estimateTransactionFee(String blockchainId,
                                       byte[] data,
                                       CompletionHandler<TransactionFee, QueryError> handler);

    /**
     * @param blockchainId     The blockchain to get blocks within
     * @param beginBlockNumber Beginning block query height (inclusive of this block number)
     * @param endBlockNumber   Final block query height (not inclusive of this block number)
     * @param includeRaw       Indicates to include the raw data of the block in the response
     * @param includeTxRaw     Indicates to include the raw data of the transactions in the response
     * @param includeTx        Indicates to include transactions in the response
     * @param includeTxProof   Indicates to include transaction proofs in the response
     * @param maxPageSize      Maximum number of blocks to return in the response
     * @param handler          The handler for created {@link com.blockset.walletkit.systemclient.Block}
     */
    public void getBlocks(String blockchainId,
                          UnsignedLong beginBlockNumber,
                          UnsignedLong endBlockNumber,
                          boolean includeRaw,
                          boolean includeTxRaw,
                          boolean includeTx,
                          boolean includeTxProof,
                          @Nullable Integer maxPageSize,
                          CompletionHandler<List<Block>, QueryError> handler);

    /**
     *
     * @param blockId The Id of the block to fetch
     * @param includeRaw Indicates to include raw data of the block in the response
     * @param includeTx Indicates to include transactions in the block in the response
     * @param includeTxRaw Indicates to include raw data in transactions in the block
     *                     in the response
     * @param includeTxProof Indicates to include transaction proof in the block in
     *                       the response
     * @param handler The handler for created {@link com.blockset.walletkit.systemclient.Block}
     */
    public void getBlock(String blockId,
                         boolean includeRaw,
                         boolean includeTx,
                         boolean includeTxRaw,
                         boolean includeTxProof,
                         CompletionHandler<Block, QueryError> handler);

    /**
     * Fetch all subscriptions
     *
     * @param handler The handler for found {@link com.blockset.walletkit.systemclient.Subscription}
     */
    public void getSubscriptions(CompletionHandler<List<Subscription>, QueryError> handler);

    /**
     * Fetch the identified subscription
     *
     * @param subscriptionId The id of the subscription to retrieve
     * @param handler        The handler for found {@link com.blockset.walletkit.systemclient.Subscription}
     */

    public void getSubscription(String subscriptionId,
                                CompletionHandler<Subscription, QueryError> handler);

    /**
     * Gets the indicated subscription and creates a new subscription if this subscription
     * does not exist
     *
     * @param subscription The desired subscription object containing its subscriptionId
     * @param handler      The handler for found or created {@link com.blockset.walletkit.systemclient.Subscription}
     */
    public void getOrCreateSubscription(Subscription subscription,
                                        CompletionHandler<Subscription, QueryError> handler);

    /**
     * Creates a new subscription
     *
     * @param deviceId   Unique identifier of the device associated with the subscription
     * @param endpoint   The notification endpoint
     * @param currencies A list of currencies for the description
     * @param handler    The handler for the created {@link com.blockset.walletkit.systemclient.Subscription}
     */
    public void createSubscription(String deviceId,
                                   SubscriptionEndpoint endpoint,
                                   List<SubscriptionCurrency> currencies,
                                   CompletionHandler<Subscription, QueryError> handler);

    /**
     * Updates the specified subscription
     *
     * @param subscription The subscription to be updated
     * @param handler      The handler for newly updated {@link com.blockset.walletkit.systemclient.Subscription}
     */
    public void updateSubscription(Subscription subscription,
                                   CompletionHandler<Subscription, QueryError> handler);

    /**
     * Deletes the indicated subscription
     *
     * @param subscriptionId The subscription identifier for deleted subscription
     * @param handler        Handler for completion indication.
     */
    public void deleteSubscription(String subscriptionId,
                                   CompletionHandler<Void, QueryError> handler);

    /** Swift SystemClient additionally defines Address based API get/create but there is
     *  no blockset analogue for these methods
     */

    /* Hedera Experimental? Users? Does it fit the generic interface? Also in the swift...

     */

    /**
     * Retrieves the indicated Hedera account
     * @param blockchainId The blockchain identifier
     * @param publicKey The public key
     * @param handler The handler for retrieved {@link com.blockset.walletkit.systemclient.HederaAccount}
     */
    public void getHederaAccount(String blockchainId,
                                 String publicKey,
                                 CompletionHandler<List<HederaAccount>, QueryError> handler);

    /**
     * Creates a new Hedera account
     *
     * @param blockchainId The blockchain identifier
     * @param publicKey    The public key
     * @param handler      The handler for created {@link com.blockset.walletkit.systemclient.HederaAccount}
     */
    public void createHederaAccount(String blockchainId,
                                    String publicKey,
                                    CompletionHandler<List<HederaAccount>, QueryError> handler);
}