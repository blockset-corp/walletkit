/*
 * Created by Michael Carrara.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.errors;

import com.blockset.walletkit.errors.QueryError;

public final class AccountInitializationQueryError extends AccountInitializationError {
    QueryError queryError;

    public AccountInitializationQueryError(QueryError queryError) {
        this.queryError = queryError;
    }

    public QueryError getQueryError() {
        return queryError;
    }
}
