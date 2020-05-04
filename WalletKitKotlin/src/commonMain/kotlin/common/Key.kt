package com.breadwallet.core.common

import com.breadwallet.core.Secret
import kotlinx.io.core.Closeable

expect class Key : Closeable {

    /**
     * Initialize based on `secret` to produce a private+public key pair
     *
     * @param secret the secret
     */
    internal constructor(secret: Secret)

    public val hasSecret: Boolean

    /** Return the WIF-encoded private key */
    public val encodeAsPrivate: ByteArray

    /** Return the hex-encoded, DER-encoded public key */
    public val encodeAsPublic: ByteArray

    public val secret: Secret

    /**
     * Check if `self` and `that` have an identical public key
     *
     * @param that the other CryptoKey
     *
     * @return If identical `true`; otherwise `false`
     */
    public fun publicKeyMatch(that: Key): Boolean

    internal fun privateKeyMatch(that: Key): Boolean

    companion object {
        public var wordList: List<String>?

        /**
         * Check if a private key `string` is a valid passphrase-protected private key.
         * The string must be BIP38 format.
         */
        public fun isProtectedPrivateKey(privateKey: String): Boolean

        /**
         * Create `Key` from a BIP-39 phrase
         *
         * @param phrase A 12 word phrase (aka paper key)
         * @param words Official BIP-39 list of words, with 2048 entries, in the language for `phrase`
         *
         * @return A Key, if the phrase if valid
         */
        public fun createFromPhrase(phrase: String, words: List<String>? = wordList): Key?

        /**
         * Create `Key` from `string` using the passphrase to decrypt it.
         *
         * The string must be BIP38 format. Different crypto currencies have
         * different implementations; this function will look for a valid string
         * using BITCOIN mainnet and BITCOIN testnet.
         *
         * @return A Key if one exists
         */
        public fun createFromProtectedPrivateKey(privateKey: String, passphrase: String): Key?

        /**
         * Create `Key` from `string`.  The string must be wallet import format (WIF), mini private
         * key format, or hex string for example: 5HxWvvfubhXpYYpS3tJkw6fq9jE9j18THftkZjHHfmFiWtmAbrj
         * Different crypto currencies have different formats; this function will look for a valid
         * string using BITCOIN mainnet and BITCOIN testnet.
         *
         * @return A Key if one exists
         */
        public fun createFromPrivateKey(privateKey: String): Key?

        /**
         * Create `Key`, as a public key, from `string`.
         *
         * The string must be the hex-encoded DER-encoded public key that is produced
         * by `encodeAsPublic`.
         *
         * @return A Key, if one exists
         */
        public fun createFromPublicKey(string: String): Key?

        public fun createForPigeonFromKey(key: Key, nonce: ByteArray): Key?

        public fun createForBIP32ApiAuth(phrase: String, words: List<String>? = wordList): Key?

        public fun createForBIP32BitID(
                phrase: String,
                index: Int,
                uri: String,
                words: List<String>? = wordList
        ): Key?
    }
}
