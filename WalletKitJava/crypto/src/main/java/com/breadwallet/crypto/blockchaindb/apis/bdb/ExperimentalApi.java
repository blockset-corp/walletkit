package com.breadwallet.crypto.blockchaindb.apis.bdb;

import com.breadwallet.crypto.blockchaindb.errors.QueryError;
import com.breadwallet.crypto.blockchaindb.errors.QueryNoDataError;
import com.breadwallet.crypto.blockchaindb.errors.QueryResponseError;
import com.breadwallet.crypto.blockchaindb.models.bdb.HederaAccount;
import com.breadwallet.crypto.utility.CompletionHandler;
import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.google.common.collect.ImmutableListMultimap;
import com.google.common.collect.ImmutableMultimap;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import static com.google.common.base.Preconditions.checkNotNull;

public class ExperimentalApi {

    private final BdbApiClient jsonClient;
    private final ScheduledExecutorService executorService;

    private static final List<String> resourcePathAccounts =
            Arrays.asList("_experimental", "hedera", "accounts");
    private static final List<String> resourcePathAccountTransactions =
            Arrays.asList("_experimental", "hedera", "account_transactions");

    public ExperimentalApi(BdbApiClient jsonClient,
                           ScheduledExecutorService executorService) {
        this.jsonClient = jsonClient;
        this.executorService = executorService;
    }

     private void getHederaAccountForTransaction(String id,
                                                String publicKey,
                                                HederaTransaction transaction,
                                                CompletionHandler<List<HederaAccount>, QueryError> handler) {
        final long retryPeriodInSeconds = 5;
        final long retryDurationInSeconds = 4 * 60;
        final long[] retriesRemaining = { (retryDurationInSeconds / retryPeriodInSeconds) - 1 };

        final String transactionId = id + ":" + transaction.getTransactionId();

        final List<CompletionHandler<HederaTransaction, QueryError>> retryHandler = new ArrayList<>();
        retryHandler.add (new CompletionHandler<HederaTransaction, QueryError>() {
            @Override
            public void handleData(HederaTransaction newTransaction) {
                switch (newTransaction.getTransactionStatus()) {
                    case "success":
                        getHederaAccount(id, publicKey, handler);
                        break;

                    case "pending":
                        if (retriesRemaining[0] == 0)
                            handler.handleError(new QueryNoDataError());
                        else {
                            retriesRemaining[0] -= 1;
                            executorService.schedule(
                                    new Runnable() {
                                        @Override
                                        public void run() {
                                            jsonClient.sendGetWithId(
                                                    resourcePathAccountTransactions,
                                                    transactionId,
                                                    ImmutableMultimap.of(),
                                                    HederaTransaction.class,
                                                    retryHandler.get(0));  // recursive-ish
                                        }
                                    },
                                    retryDurationInSeconds,
                                    TimeUnit.SECONDS);
                        }
                        break;

                    default:
                        handler.handleError(new QueryNoDataError());
                        break;
                }
            }

            @Override
            public void handleError(QueryError error) {
                handler.handleError(new QueryNoDataError());
            }
        });

        jsonClient.sendGetWithId(
                resourcePathAccountTransactions,
                transactionId,
                ImmutableMultimap.of(),
                HederaTransaction.class,
                retryHandler.get(0));
   }

    public void getHederaAccount(String id,
                                 String publicKey,
                                 CompletionHandler<List<HederaAccount>, QueryError> handler) {
         jsonClient.sendGetForArray(
                 resourcePathAccounts,
                "accounts",
                ImmutableListMultimap.of(
                        "blockchain_id", id,
                        "pub_key", publicKey),
                HederaAccount.class,
                handler);
    }

    public void createHederaAccount(String id,
                                    String publicKey,
                                    CompletionHandler<List<HederaAccount>, QueryError> handler) {
        jsonClient.sendPost(
                resourcePathAccounts,
                ImmutableMultimap.of(),
                NewHederaAccount.create(id, publicKey),
                HederaTransaction.class,
                new CompletionHandler<HederaTransaction, QueryError>() {
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

        private static class NewHederaAccount {
        @JsonCreator
        public static NewHederaAccount create(@JsonProperty("blockchain_id") String blockchainId,
                                              @JsonProperty("pub_key") String publicKey) {
            return new NewHederaAccount(
                    checkNotNull(blockchainId),
                    checkNotNull(publicKey)
            );
        }

        private final String blockchainId;
        private final String publicKey;

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

    // Combines 'POST accounts/' and 'GET account_transactions/' - because, why not.
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
}
