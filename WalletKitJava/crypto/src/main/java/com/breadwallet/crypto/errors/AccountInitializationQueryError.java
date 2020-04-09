package com.breadwallet.crypto.errors;

import com.breadwallet.crypto.blockchaindb.errors.QueryError;

public final class AccountInitializationQueryError extends AccountInitializationError {
    QueryError queryError;

    public AccountInitializationQueryError(QueryError queryError) {
        this.queryError = queryError;
    }

    public QueryError getQueryError() {
        return queryError;
    }
}
