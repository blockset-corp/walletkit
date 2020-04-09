package com.breadwallet.crypto.errors;

import com.breadwallet.crypto.blockchaindb.models.bdb.HederaAccount;

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
