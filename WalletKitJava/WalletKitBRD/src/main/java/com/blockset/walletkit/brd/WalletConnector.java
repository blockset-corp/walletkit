/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.SystemClient;
import com.blockset.walletkit.errors.QueryError;
import com.blockset.walletkit.nativex.WKKey;
import com.blockset.walletkit.nativex.WKWalletConnectorError;
import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.errors.WalletConnectorError;
import com.blockset.walletkit.nativex.WKWalletConnector;
import com.blockset.walletkit.nativex.support.WKResult;
import com.blockset.walletkit.utility.CompletionHandler;

import com.google.common.base.Optional;
import com.google.common.io.BaseEncoding;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
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
     * @param completion A completion handler for the result and potential
     *                  {@link WalletConnectorError.UnsupportedConnector} error
     *                  if `manager` does not support the WalletConnect 1.0 specification
     */
    static void create(  WalletManager manager,
                         CompletionHandler<com.blockset.walletkit.WalletConnector, WalletConnectorError> completion) {

        Optional<WKWalletConnector> walletConnector =
                WKWalletConnector.create(manager.getCoreBRCryptoWalletManager());

        if (walletConnector.isPresent())
            completion.handleData(WalletConnector.create(walletConnector.get(), manager));
        else
            completion.handleError(new WalletConnectorError.UnsupportedConnector());
    }

    @Override
    public Result<DigestAndSignaturePair, WalletConnectorError>
    sign( byte[]                      message,
          com.blockset.walletkit.Key  key,
          boolean                     prefix  ) {

        if (!key.hasSecret()) {

            // Opportunity to Logger.log something here...
            Result.failure(new WalletConnectorError.InvalidKeyForSigning("Key object does not have a private key"));
        }

        // Digest algorithm may be different per network type and optional prefix must be included
        // per network definition prior to doing the hash
        byte[] digestData = null;
        byte[] signatureData = null;
        
        WKResult<byte[], WKWalletConnectorError> digestResult = core.getDigest(message, prefix);
        if (digestResult.isSuccess()) {
            
            digestData = digestResult.getSuccess();
            Key cryptoKey = Key.from(key);
            WKResult<byte[], WKWalletConnectorError> sigResult = core.sign(digestData,
                                                                           cryptoKey.getBRCryptoKey());
            
            if (sigResult.isSuccess()) {
                signatureData = sigResult.getSuccess();
            } else {
                return Result.failure(wkErrorToError(sigResult.getFailure()));
            }
        } else {
            return Result.failure(wkErrorToError(digestResult.getFailure()));
        }

        Digest digest = new Digest(this.core, digestData);
        Signature signature = new Signature(this.core, signatureData);
        return Result.success(new DigestAndSignaturePair(digest, signature));
    }

    @Override
    public Result<com.blockset.walletkit.Key, WalletConnectorError>
    recover (com.blockset.walletkit.WalletConnector.Digest      digest,
             com.blockset.walletkit.WalletConnector.Signature   signature   ) {

        // Check objects validity
        if (!(digest instanceof Digest) ||
            !(signature instanceof Signature) ||
            ( ((Digest)digest).core.getPointer() != core.getPointer() ||
              ((Signature)signature).core.getPointer() != core.getPointer())) {
            return Result.failure(new WalletConnectorError.UnknownEntity());
        }
        WKResult<WKKey,WKWalletConnectorError> recoverResult = core.recover(((Digest) digest).data32,
                                                                            ((Signature) signature).data);
        if (recoverResult.isSuccess()) {
            // The returned key will own the native WKKey core memory
            return Result.success(Key.create(recoverResult.getSuccess()));
        }
        return Result.failure(new WalletConnectorError.UnrecoverableKey());
    }

    @Override
    public Serialization createSerialization(byte[] data) {
        return new Serialization(core, data);
    }

    @Override
    public Result<com.blockset.walletkit.WalletConnector.Transaction, WalletConnectorError>
    createTransaction(Map<String, String> arguments) {

        List<String> keys = new ArrayList<String>();
        List<String> vals = new ArrayList<String>();
        
        keys.addAll(arguments.keySet());
        vals.addAll(arguments.values());
        
        WKResult<byte[], WKWalletConnectorError> serializationRes =
                core.createTransactionFromArguments(keys, vals, arguments.size());

        if (serializationRes.isFailure()) {
            return Result.failure(wkErrorToError(serializationRes.getFailure()));
        }

        Serialization serialization = new Serialization(this.core,
                                                        serializationRes.getSuccess());
        return Result.success(new Transaction(this.core, serialization, false));
    }

    @Override
    public Result<com.blockset.walletkit.WalletConnector.Transaction, WalletConnectorError>
    createTransaction(com.blockset.walletkit.WalletConnector.Serialization serialization ) {
        if (!(serialization instanceof Serialization) ||
             ((Serialization)serialization).core.getPointer() != core.getPointer()) {

            // Opportunity to Logger.log something here...

            return Result.failure(new WalletConnectorError.UnknownEntity());
        }

        WKResult<WKWalletConnector.CreateTransactionFromSerializationResult,
                 WKWalletConnectorError> serializationRes = core.createTransactionFromSerialization(serialization.getData());
        if (!serializationRes.isFailure()) {
            return Result.failure(wkErrorToError(serializationRes.getFailure()));
        }
        
        return Result.success(new Transaction(this.core,
                              new Serialization(this.core, serializationRes.getSuccess().serialization),
                              serializationRes.getSuccess().isSigned));
    }

    @Override
    public Result<com.blockset.walletkit.WalletConnector.Transaction, WalletConnectorError>
    sign(   com.blockset.walletkit.WalletConnector.Transaction  transaction,
            com.blockset.walletkit.Key                          key          ) {
    
        if (!(transaction instanceof Transaction) ||
             ((Transaction)transaction).core.getPointer() != core.getPointer() ) {

            // perhaps log an error?
            return Result.failure(new WalletConnectorError.UnknownEntity());
        }
        if (!key.hasSecret()) {
            // Log a log?
            return Result.failure(new WalletConnectorError.InvalidKeyForSigning("Key object does not have a private key"));
        }
        if (transaction.isSigned()) {
            return Result.failure(new WalletConnectorError.PreviouslySignedTransaction());
        }
        
        Key cryptoKey = Key.from(key);

        // Returns the RLP serialization of a signed transaction
        WKResult<byte[], WKWalletConnectorError> signedTransactionRlpSerializationResult =
                core.signTransaction(transaction.getSerialization().getData(),
                                     cryptoKey.getBRCryptoKey());
        
        if (signedTransactionRlpSerializationResult.isFailure()) {
            return Result.failure(wkErrorToError(signedTransactionRlpSerializationResult.getFailure()));
        }
        
        // Return fresh signed transaction
        return Result.success(new Transaction(this.core,
                              new Serialization(this.core, signedTransactionRlpSerializationResult.getSuccess()),
                              true));
    }

    @Override
    public void submit(
            com.blockset.walletkit.WalletConnector.Transaction transaction,
            CompletionHandler<com.blockset.walletkit.WalletConnector.Transaction, WalletConnectorError> completion  ) {

        if (!(transaction instanceof Transaction) ||
             ((Transaction)transaction).core.getPointer() != core.getPointer() ) {
            completion.handleError(new WalletConnectorError.UnknownEntity());
            return;
        }

        if (!transaction.isSigned()) {
            completion.handleError(new WalletConnectorError.UnsignedTransaction());
            return;
        }

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
    class Digest implements com.blockset.walletkit.WalletConnector.Digest {

        private final byte[]        data32;
        final WKWalletConnector     core;

        Digest(WKWalletConnector   core,
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
    class Signature implements com.blockset.walletkit.WalletConnector.Signature {

        private final byte[]        data;
        final WKWalletConnector     core;

        Signature(WKWalletConnector   core,
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
    class Serialization implements com.blockset.walletkit.WalletConnector.Serialization {

        private final byte[]        data;
        final WKWalletConnector     core;

        Serialization(WKWalletConnector   core,
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
    class Transaction implements com.blockset.walletkit.WalletConnector.Transaction {

        private final Serialization     serialization;
        final WKWalletConnector         core;
        final boolean                   isSigned;

        Transaction(WKWalletConnector   core,
                    Serialization       serialization,
                    boolean             isSigned        ) {

            this.core = core;
            this.serialization = serialization;
            this.isSigned = isSigned;
        }

        @Override
        public Serialization getSerialization() { return serialization; }

        @Override
        public boolean isSigned() { return isSigned; }
    }

    /** Translate native error enumeration to Walletkit Java class equivalents
     *  Importantly -- returns null when the status code indicates no error
     * @param error The native enumeration of the error
     * @return A com.blockset.walletkit.errors error class or null if there was no error
     */
    private static WalletConnectorError wkErrorToError(WKWalletConnectorError error) {

        // Currently defined native errors...
        switch (error) {
            case UNSUPPORTED_CONNECTOR:
                return new WalletConnectorError.UnsupportedConnector();
            case ILLEGAL_OPERATION:
                return new WalletConnectorError.IllegalOperation();
            case INVALID_TRANSACTION_ARGUMENTS:
                return new WalletConnectorError.InvalidTransactionArguments();
            case INVALID_DIGEST:
                return new WalletConnectorError.InvalidDigest();
            case INVALID_SIGNATURE:
                return new WalletConnectorError.InvalidSignature();
            case INVALID_SERIALIZATION:
                return new WalletConnectorError.InvalidTransactionSerialization();
        }
        return null;
    }
}
