/*
 * Created by Bryan Goring <bryan.goring@brd.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit;

import com.blockset.walletkit.errors.WalletConnectorError;
import com.blockset.walletkit.utility.CompletionHandler;

import java.util.Map;

public interface WalletConnector {

    /** Signature
     *
     *  A signature holds the signature bytes in the form
     *  specific to the WalletConnector.
     */
    interface Signature {
        /**
         * Get signature data
         * @return The signature bytes
         */
        byte[] getData();
    }

    /** Digest
     *
     *  A Digest holds '32 hash bytes'
     */
    interface Digest {
        /**
         * Get digest data, typically 32 bytes
         * @return The digest data
         */
        byte[] getData32();
    }

    /** Transaction
     *
     */
    interface Transaction {

        /** Check if the transaction is signed
         *
         * @return True if signed
         */
        boolean isSigned();

        /** Gets the transaction serialization.
         *  This could be unsigned or unsigned according to the
         *  status indicated by ({@link #isSigned}
         *
         */
        Serialization getSerialization();
    }

    /** Serialization
     *
     *  A serialization is a byte sequence representing an
     *  unsigned or signed transaction.
     *
     */
    interface Serialization {

        /**
         *  Get serialization data
         * @return the serialization bytes
         */
        byte[] getData();
    }

    /** Composite wrapper of a {@link Digest} and {@link Signature}
     *  to allow returning them together.
     */
    class DigestAndSignaturePair {
        public final Digest digest;
        public final Signature signature;
        public DigestAndSignaturePair(Digest    digest,
                                      Signature signature) {
            this.digest = digest;
            this.signature = signature;
        }
    }

    /**
     * Sign arbitrary data
     *
     * @param message Arbitrary data to be signed
     * @param key: Private signing {@link Key} key
     * @param prefix Indicates to include optional prefix in the signature (TBD: may not need
     *               to be mandated)
     * @return The pair of the {@link Digest} and {@link Signature}
     * @throws {@link WalletConnectorError.InvalidKeyForSigning}
     */
    DigestAndSignaturePair sign(   byte[]       message,
                                   Key          key,
                                   boolean      prefix  ) throws WalletConnectorError;

    /**
     * Recover the public key
     *
     * @param digest The {@link Digest} digest
     * @param signature The corresponding {@link Signature} signature
     * @return On success, a public key.
     * @throws {@link WalletConnectorError.UnknownEntity}
     *         to indicate the 'digest' or 'signature' are not from 'self'
     *
     */
     Key recover (  Digest     digest,
                    Signature  signature   ) throws WalletConnectorError;

    /**
     *  Create a serialization from arbitrary data, typically the data should
     *  be signed or unsigned transaction data, but no checks are performed.
     * @param data The data
     * @return A {@link Serialization} serialization
     */
    Serialization createSerialization(byte[] data);

    /**
     * Create a Transaction from a wallet-connect-specific dictionary of arguments applicable to
     * the connector's network.  For ETH the Dictionary keys are: {...}
     * @param arguments Connector networks arguments for transaction, in the form of key/value pairs
     * @result An unsigned {@link Transaction}
     * @throws {@link WalletConnectorError.InvalidTransactionArguments}
     */
    Transaction createTransaction ( Map<String, String> arguments ) throws WalletConnectorError;

    /**
     * Create a Transaction from a signed or unsigned serialization. Creation of a
     * Transaction from the Serialization object implies that this Serialization data
     * conforms to the Network's conventions regarding serialization (i.e. it may
     * not be just arbitrary data).
     *
     * @param serialization A transaction serialization, signed or unsigned
     * @return On success, an unsigned or signed {@link Transaction}. On failure
     * @throws {@link WalletConnectorError.UnknownEntity}
     *         if the serialization is not from 'self'
     */
    Transaction createTransaction ( Serialization serialization ) throws WalletConnectorError;

    /**
     * Sign a transaction
     *
     * This function is the 'sign' part of the ETH JSON-RPC `eth_sendTransaction` and
     * `eth_sendRawTransaction`.
     *
     * @param transaction The input transaction to be signed
     * @param key A private key
     * @return On success, a signed {@link Transaction} which will be distinct from the provided
     *         'transaction' argument.
     * @throws {@link WalletConnectorError.UnknownEntity}
     *            if the 'transaction' is not from 'self'
     * @throws {@link WalletConnectorError.InvalidKeyForSigning}
     *            if 'key' is not private
     */
    Transaction sign ( Transaction     transaction,
                       Key             key         ) throws WalletConnectorError;

    /**
     * Send a transaction to the connector's network. As implied by the presence of a
     * CompletionHandler, this method executes asynchronously.
     *
     *
     * This function is the 'submit' part of the ETH JSON-RPC `eth_sendTransaction` and
     * `eth_sendRawTransaction`.
     *
     * @param transaction The transaction to be submitted
     * @param completion The handler to which {@link Transaction} result and potential errors
     *                   are directed.
     * @return On success, a submitted transaction which may be distinct from the provided
     *         transaction argument.
     * @throws {@link WalletConnectorError.UnknownEntity}
     *           if `transaction` is not from `self`
     * @throws {@link WalletConnectorError.UnsignedTransaction}
     *           if `transaaction` is not signed
     * May also indicate {@link WalletConnectorError.SubmitFailed}
     * to the completion if the `transaction` was not submitted
     */
    void submit ( Transaction                                           transaction,
                  CompletionHandler<Transaction, WalletConnectorError>  completion  ) throws WalletConnectorError;
}
