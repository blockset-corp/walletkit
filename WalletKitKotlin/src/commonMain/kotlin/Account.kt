package com.breadwallet.core

import kotlinx.io.core.Closeable

expect class Account : Closeable {

    /**
     * A 'globally unique' ID String for account.
     *
     * For BlockchainDB this will be the 'walletId'
     */
    public val uids: String

    public val timestamp: Long

    public val filesystemIdentifier: String

    /**
     * Serialize an account.  The serialization is *always* in the current, default format
     */
    public val serialize: ByteArray

    public fun validate(serialization: ByteArray): Boolean

    public fun isInitialized(network: Network): Boolean

    public fun getInitializationData(network: Network): ByteArray

    public fun initialize(network: Network, data: ByteArray)

    override fun close()

    companion object {
        /**
         * Recover an account from a BIP-39 'paper key'
         *
         * @param phrase the 12 word paper key
         *
         * @return the paperKey's corresponding account, or NIL if the paperKey is invalid.
         */
        public fun createFromPhrase(phrase: ByteArray, timestamp: Long, uids: String): Account?

        /**
         * Create an account based on an account serialization
         *
         * @param serialization The result of a prior call to [Account.serialize]
         *
         * @return The serialization's corresponding account or NIL if the serialization is invalid.
         *    If the serialization is invalid then the account *must be recreated* from the `phrase`
         *    (aka 'Paper Key').  A serialization will be invalid when the serialization format changes
         *    which will *always occur* when a new blockchain is added.  For example, when XRP is added
         *    the XRP public key must be serialized; the old serialization w/o the XRP public key will
         *    be invalid and the `phrase` is *required* in order to produce the XRP public key.
         */
        public fun createFromSerialization(serialization: ByteArray, uids: String): Account?

        /**
         * Generate a BIP-39 'paper Key'.
         *
         * Use Account.createFrom(paperKey:) to get the account.  The wordList is the locale-specific
         * BIP-39-defined array of BIP39_WORDLIST_COUNT words.  This function has a precondition on
         * the size of the wordList.
         *
         * @param words A local-specific BIP-39-defined array of BIP39_WORDLIST_COUNT words.
         *
         * @return A 12 word 'paper key'
         */
        public fun generatePhrase(words: List<String>): ByteArray?

        /**
         * Validate a phrase as a BIP-39 'paper key'; returns true if validated, false otherwise
         *
         * @param phrase the candidate paper key
         * @param words A local-specific BIP-39-defined array of BIP39_WORDLIST_COUNT words.
         *
         * @return true is a valid paper key; false otherwise
         */
        public fun validatePhrase(phrase: ByteArray, words: List<String>): Boolean
    }
}
