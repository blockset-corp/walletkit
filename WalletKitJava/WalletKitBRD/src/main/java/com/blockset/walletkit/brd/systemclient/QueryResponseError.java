/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd.systemclient;

import com.blockset.walletkit.errors.QueryError;

import java.util.Map;

// HTTP response unexpected (typically not 200/OK)
public class QueryResponseError extends QueryError {

    private final int statusCode;
    private final Map<String,Object> json;
    private final boolean jsonError;

    public QueryResponseError(int statusCode, Map<String,Object> json, boolean jsonError) {
        super("Status code " + statusCode);
        this.statusCode = statusCode;
        this.json = json;
        this.jsonError = jsonError;
    }

    public int getStatusCode() {
        return statusCode;
    }

    public Map<String, Object> getJson() {
        return json;
    }

    public boolean isJsonError() {
        return jsonError;
    }

    @Override
    public String getMessage() {
        return super.getMessage() + ", Message: " + json.toString();
    }
}
