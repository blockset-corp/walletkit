/*
 * Created by Michael Carrara.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.errors;

import com.blockset.walletkit.SystemClient.HederaAccount;

import java.util.List;

public final class AccountInitializationMultipleHederaAccountsError extends AccountInitializationError {
    List<HederaAccount> accounts;

    public AccountInitializationMultipleHederaAccountsError (List<HederaAccount> accounts) {
        this.accounts = accounts;
    }

    public List<HederaAccount> getAccounts() {
        return accounts;
    }
}
