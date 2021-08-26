/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.Key;
import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.errors.QueryError;
import com.blockset.walletkit.nativex.WKWalletConnectorError;
import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.errors.WalletConnectorError;
import com.blockset.walletkit.nativex.WKWalletConnector;
import com.blockset.walletkit.utility.CompletionHandler;

import com.google.common.base.Optional;
import com.google.common.io.BaseEncoding;

import java.util.Arrays;
import java.util.Map;

final class WalletConnector implements com.blockset.walletkit.WalletConnector {

    /** @brief The core reference
     */
    private final WKWalletConnector     core;

    /** @brief The manager for this connector
     */
    private final WalletManager         manager;

    private WalletConnector(    WKWalletConnector   core,
                                WalletManager       manager ) {
        this.core = core;
        this.manager = manager;
    }

    /** Create the WalletConnector with the valid WalletKit core connector object
     *  and the WalletManager
     * @param coreConnector The WalletKit core WalletConnect object
     * @param manager The WalletKit manager
     * @return A newly instantiated WalletConnector
     */
    private static WalletConnector create( WKWalletConnector    coreConnector,
                                           WalletManager        manager ) {
        WalletConnector connector = new WalletConnector(coreConnector, manager);
        ReferenceCleaner.register(connector, coreConnector::give);
        return connector;
    }

    /**
     * Create a WalletConnector is supported for the manager's network
     *
     * @param manager: The `WalletManager` for this connector
     *
     * @return On success, a WalletConnector for `manager`.
     * @throws {@link WalletConnectorError.UnsupportedConnector}
     *   if `manager` does not support the WalletConnect 1.0 specification
     */
    static WalletConnector create(WalletManager manager) throws WalletConnectorError {

        Optional<WKWalletConnector> walletConnector =
                WKWalletConnector.create(manager.getCoreBRCryptoWalletManager());

        if (walletConnector.isPresent())
            return WalletConnector.create(walletConnector.get(), manager);
        else
            throw new WalletConnectorError.UnsupportedConnector();
    }

    @Override
    public DigestAndSignaturePair sign( byte[]      message,
                                        Key         key,
                                        boolean     prefix  ) throws WalletConnectorError {
        if (!key.hasSecret())
            throw new WalletConnectorError.InvalidKeyForSigning("Key object does not have a private key");

        return null;
    }

    @Override
    public Key recover (Digest      digest,
                        Signature   signature   ) throws WalletConnectorError {

        // Check objects validity
        if (!(digest instanceof BRDDigest) ||
            !(signature instanceof BRDSignature) ||
            ( ((BRDDigest)digest).core.getPointer() != core.getPointer() ||
              ((BRDSignature)signature).core.getPointer() != core.getPointer()))
            throw new WalletConnectorError.UnknownEntity();

        return null;
    }

    @Override
    public Serialization createSerialization(byte[] data) {
        return new BRDSerialization(core, data);
    }

    @Override
    public Transaction createTransaction(Map<String, String> arguments) throws WalletConnectorError {

        // *may* throw new WalletConnectorError.InvalidTransactionArguments();

        return null;
    }

    @Override
    public Transaction createTransaction( Serialization serialization ) throws WalletConnectorError {
        if (!(serialization instanceof BRDSerialization) ||
             ((BRDSerialization)serialization).core.getPointer() != core.getPointer() )
            throw new WalletConnectorError.UnknownEntity();

        return null;
    }

    @Override
    public Transaction sign(   Transaction     transaction,
                               Key             key          ) throws WalletConnectorError {
        if (!(transaction instanceof BRDTransaction) ||
             ((BRDTransaction)transaction).core.getPointer() != core.getPointer() )
            throw new WalletConnectorError.UnknownEntity();
        if (!key.hasSecret())
            throw new WalletConnectorError.InvalidKeyForSigning("Key object does not have a private key");

        /// Do signing...
        return transaction;
    }

    @Override
    public void submit(
            Transaction                                             transaction,
            CompletionHandler<Transaction, WalletConnectorError>    completion  ) throws WalletConnectorError {

        if (!(transaction instanceof BRDTransaction) ||
             ((BRDTransaction)transaction).core.getPointer() != core.getPointer() )
            throw new WalletConnectorError.UnknownEntity();
        if (!transaction.isSigned())
            throw new WalletConnectorError.UnsignedTransaction();

        byte[] data = transaction.getSerialization().getData();
        String base64 = BaseEncoding.base64().encode(
                            Arrays.copyOfRange(data, 0, data.length < 10 ? data.length : 10));
        String identifier = String.format("WalletConnect: %s:%s",
                                          manager.getNetwork().getUids(),
                                          base64);
        manager.getSystem().getSystemClient().createTransaction(
                manager.getNetwork().getUids(),
                data,
                identifier,
                new CompletionHandler<SystemClient.TransactionIdentifier, QueryError>() {
                    @Override
                    public void handleData(SystemClient.TransactionIdentifier tid) {
                        completion.handleData(transaction);
                    }

                    @Override
                    public void handleError(QueryError error) {
                        completion.handleError(new WalletConnectorError.SubmitFailed());
                    }
                });
    }

    /** Concrete Digest with WKWalletConnector core object
     *
     */
    class BRDDigest implements Digest {

        private final byte[]        data32;
        final WKWalletConnector     core;

        BRDDigest(   WKWalletConnector   core,
                     byte[]              data32) {

            this.core = core;
            this.data32 = data32;
        }

        @Override
        public byte[] getData32() { return data32; }
    }

    /** Concrete Signature with WKWalletConnector core object
     *
     */
    class BRDSignature implements Signature {

        private final byte[]        data;
        final WKWalletConnector     core;

        BRDSignature(   WKWalletConnector   core,
                        byte[]              data) {

            this.core = core;
            this.data = data;
        }

        @Override
        public byte[] getData() { return data; }
    }

    /** Concrete Serialization with WKWalletConnector core object
     *
     */
    class BRDSerialization implements Serialization {

        private final byte[]        data;
        final WKWalletConnector     core;

        BRDSerialization(   WKWalletConnector   core,
                            byte[]              data) {

            this.core = core;
            this.data = data;
        }

        @Override
        public byte[] getData() { return data; }
    }

    /** Concrete Transaction with WKWalletConnector core object
     *
     */
    class BRDTransaction implements Transaction {

        private final Serialization serialization;
        final WKWalletConnector     core;

        BRDTransaction(   WKWalletConnector   core,
                          Serialization       serialization) {

            this.core = core;
            this.serialization = serialization;
        }

        @Override
        public Serialization getSerialization() { return serialization; }

        @Override
        public boolean isSigned() { return false; }
    }

    /** Translate native error enumeration to Walletkit Java class equivalents
     *  Importantly -- returns null when the status code indicates no error
     * @param error The native enumeration of the error
     * @return A com.blockset.walletkit.errors error class or null if there was no error
     */
    private static WalletConnectorError wkErrorToError(WKWalletConnectorError error) {

        // Currently defined native errors...
        switch (error) {
            case SUCCESS: return null;

            case ERROR_UNSUPPORTED_CONNECTOR:
                return new WalletConnectorError.UnsupportedConnector();
        }
        return null;
    }

    /* package */
 //   WKWalletConnector getWKWalletConnector() { return core; }
}
