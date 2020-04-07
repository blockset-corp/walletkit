/*
 * Account
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 1/22/18.
 * Copyright (c) 2018-2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.crypto;

import com.google.common.base.Optional;

import java.util.Date;
import java.util.List;
import java.lang.String;

public interface Account {

    /**
     * Recover an account from a BIP-39 'paper key'.
     *
     * @apiNote The caller should take appropriate security measures, like enclosing this method's call in a
     * try-finally block that wipes the phraseUtf8 value, to ensure that it is purged from memory
     * upon completion.
     *
     * @param phraseUtf8 The UTF-8 NFKD normalized BIP-39 paper key
     * @param timestamp The timestamp of when this account was first created
     * @param uids The unique identifier of this account
     */
    static Optional<Account> createFromPhrase(byte[] phraseUtf8, Date timestamp, String uids) {
        return CryptoApi.getProvider().accountProvider().createFromPhrase(phraseUtf8, timestamp, uids);
    }

    /**
     * Create an account based on an account serialization.
     *
     * @param serialization The result of a prior call to {@link Account#serialize()}
     * @param uids The unique identifier of this account
     *
     * @return The serialization's corresponding account or {@link Optional#absent()} if the serialization is invalid.
     *         If the serialization is invalid then the account <b>must be recreated</b> from the `phrase`
     *         (aka 'Paper Key').  A serialization will be invald when the serialization format changes
     *         which will <b>always occur</b> when a new blockchain is added.  For example, when XRP is added
     *         the XRP public key must be serialized; the old serialization w/o the XRP public key will
     *         be invalid and the `phrase` is <b>required</b> in order to produce the XRP public key.
     */
    static Optional<Account> createFromSerialization(byte[] serialization, String uids) {
        return CryptoApi.getProvider().accountProvider().createFromSerialization(serialization, uids);
    }

    /**
     * Generate a BIP-39 'paper Key'
     *
     * Use {@link Account#createFromPhrase(byte[], Date, String)} to get the account
     *
     * @return A UTF-8 NFKD normalized BIP-39 paper key
     */
    static byte[] generatePhrase(List<String> words) {
        return CryptoApi.getProvider().accountProvider().generatePhrase(words);
    }

    /**
     * Validate a phrase as a BIP-39 'paper key'
     *
     * @param phraseUtf8 The UTF-8 NFKD normalized BIP-39 paper key
     * @param words A locale-specific BIP-39-defined array of 2048 words.
     *
     * @return true is a valid paper key; false otherwise
     */
    static boolean validatePhrase(byte[] phraseUtf8, List<String> words) {
        return CryptoApi.getProvider().accountProvider().validatePhrase(phraseUtf8, words);
    }

    Date getTimestamp();

    /**
     * A 'globally unique' ID String for account.
     *
     * For BlockchainDB this will be the 'walletId'.
     */
    String getUids();

    /**
     * Serialize an account.  The serialization is <b>always</b> in the current, default format.
     */
    byte[] serialize();

    /**
     * Validate a serialized account contains the same seed material
     *
     * @param serialization a serialized account from {@link #serialize()}
     *
     * @return true contains the same material; false otherwise
     */
    boolean validate(byte[] serialization);

    /**
     * Check if `account` is initialized for `network`.  Some networks require that accounts
     * be initialized before they can be used; Hedera is one such network.
     *
     * @param network the network
     *
     * @return `true` if initialized; `false` otherwise
     */
    boolean isInitialized (Network network);

    /**
     * Initialize `account` on `network` using `data`.  The provided data is network specific and
     * thus an opaque sequence of bytes.
     *
     * @param network the network
     * @param data the data
     *
     * @return The account serialization or `nil` if the account was already initialized.  This
     *            serialization must be saved otherwise the initialization will be lost upon the
     *            next System start.
     */
    byte[] initialize (Network network, byte[] data);

    /**
     * Get the data needed to initialize `account` on `network`.  This data is network specfic and
     * thus an opaqe sequence of bytes.  The bytes are provided to some 'initialization provider'
     * in a network specific manner; the provider's result is passed back using the
     * `accountInitialize` function.
     *
     * @param network the network
     *
     * @return Opaque data to be provided to the 'initialization provider' or null is no
     *         initialization is required.
     */
    byte[] getInitializationData (Network network);
}
