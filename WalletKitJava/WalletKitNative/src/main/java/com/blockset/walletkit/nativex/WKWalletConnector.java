/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.support.WKResult;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.blockset.walletkit.nativex.utility.SizeTByReference;
import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedInts;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.StringArray;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.PointerByReference;

import java.util.List;

/** WKWalletConnector processes interactions with WalletKit native code
 *  and transmits expected results or errors in the form of WKResults.
 */
public class WKWalletConnector extends PointerType {

    public WKWalletConnector() {
        super ();
    }

    public WKWalletConnector(Pointer address) {
        super (address);
    }

    /** Creates a standard message out of the message input
     * @param message The input message
     * @return A {@link WKResult} object containing the new standard message, or
     *         a WalletKit native error.
     */
    public WKResult<byte[], WKWalletConnectorError>
    createStandardMessage (byte[] message) {
        WKResult res;
        SizeTByReference standardMessageLenRef = new SizeTByReference();
        IntByReference err = new IntByReference();

        Pointer standardMessagePtr = WKNativeLibraryDirect.wkWalletConnectorCreateStandardMessage(
                this.getPointer(),
                message,
                new SizeT(message.length),
                standardMessageLenRef,
                err    );
        try {

            int standardMessageLen = UnsignedInts.checkedCast(standardMessageLenRef.getValue().intValue());

            // Firstly deal with the potential of an indicated error
            if (standardMessagePtr.equals(Pointer.NULL)) {
                res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
            } else {
                res = WKResult.success(standardMessagePtr.getByteArray(0, standardMessageLen));
            }
        } finally {
            Native.free(Pointer.nativeValue(standardMessagePtr));
        }

        return res;
    }

    /** Gets a digest of the message bytes using native WalletKit calls
     *
     * @param message The message to be processed
     * @return A {@link WKResult} object containing the digest byte buffer, or
     *         WalletKit native error representation.
     */
    public WKResult<byte[], WKWalletConnectorError>
    getDigest(  byte[]      message ) {

        WKResult res;
        SizeTByReference digestLenRef = new SizeTByReference();
        IntByReference err = new IntByReference();
        
        Pointer digestPtr = WKNativeLibraryDirect.wkWalletConnectorGetDigest(this.getPointer(),
                                                                             message,
                                                                             new SizeT(message.length),
                                                                             digestLenRef,
                                                                             err    );
        try {

            int digestLen = UnsignedInts.checkedCast(digestLenRef.getValue().intValue());

            // Firstly deal with the potential of an indicated error
            if (digestPtr.equals(Pointer.NULL)) {
                res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
            } else {
                res = WKResult.success(digestPtr.getByteArray(0, digestLen));
            }
        } finally {
            Native.free(Pointer.nativeValue(digestPtr));
        }

        return res;
    }

    /** Creates a signing key from BIP 39 mnemonic phrase
     *
     */
    public WKResult<WKKey, WKWalletConnectorError>
    createKey(  String phrase   ) {
        WKResult res;
        IntByReference err = new IntByReference();

        Pointer key = WKNativeLibraryDirect.wkWalletConnectorCreateKey(this.getPointer(),
                                                                       phrase,
                                                                       err  );
        if (key.equals(Pointer.NULL)) {
            res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
        }
        return WKResult.success(new WKKey(key));
    }

    /** Signs the byte buffer using native WalletKit calls
     *
     * @param data The data to be signed
     * @param key The key, presumed to have private key information, to sign with
     * @return A {@link WKResult} object containing the signature buffer, or a
     *         WalletKit native error representation.
     */
    public WKResult<byte[], WKWalletConnectorError>
    sign(   byte[]  data,
            WKKey   key ) {

        WKResult res;
        Pointer keyPtr = key.getPointer();
        SizeTByReference signatureLenRef = new SizeTByReference();
        IntByReference err = new IntByReference();

        // First find the required buffer length for the signature.
        Pointer signaturePtr = WKNativeLibraryDirect.wkWalletConnectorSignData(this.getPointer(),
                                                                               data,
                                                                               new SizeT(data.length),
                                                                               keyPtr,
                                                                               signatureLenRef,
                                                                               err  );
        try {

            int signatureLen = UnsignedInts.checkedCast(signatureLenRef.getValue().intValue());

            // Firstly deal with the potential directly indicated error of signing
            if (signaturePtr.equals(Pointer.NULL)) {
                res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
            } else {
                res = WKResult.success(signaturePtr.getByteArray(0, signatureLen));
            }
        } finally {
            Native.free(Pointer.nativeValue(signaturePtr));
        }

        return res;
    }

    /** Recovers the public key in the form of WKKey using the provided
     *  digest and signature
     * @param digest The digest
     * @param signature The signature
     * @return A {@link WKResult} containing the recovered key or WalletKit WalletConnect
     *         related error
     */
    public WKResult<WKKey, WKWalletConnectorError>
    recover(    byte[] digest,
                byte[] signature    ) {

        WKResult res;
        IntByReference err = new IntByReference();

        Pointer keyPtr = WKNativeLibraryDirect.wkWalletConnectorRecoverKey(this.getPointer(),
                                                                           digest,
                                                                           new SizeT(digest.length),
                                                                           signature,
                                                                           new SizeT(signature.length),
                                                                           err);

        // A null return key is an indicator of error. Otherwise the core key is claimed
        // in the result.
        if (keyPtr.equals(Pointer.NULL)) {
            res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
        } else {
            res = WKResult.success(new WKKey(keyPtr));
        }

        return res;
    }
    
    /** Creates a transaction serialization in the form of byte array,
     *  from a list of key & associated value pairs, using native WalletKit calls.
     *  The count of keys is provided explicitly so that it may be assumed
     *  the number of values corresponds directly to keys.
     *
     * @param keys The list of key values
     * @param values The associated values
     * @return A {@link WKResult} object containing the transaction buffer, or a
     *         WalletKit native error representation.
     */
    public WKResult<byte[], WKWalletConnectorError>
    createTransactionFromArguments(
            List<String> keys, 
            List<String> values,
            int          countOfKeyValuePairs) {

        WKResult res;
        StringArray keysArray = new StringArray(keys.toArray(new String[0]), "UTF-8");
        StringArray valuesArray = new StringArray(values.toArray(new String[0]), "UTF-8");
        SizeTByReference serLengthRef = new SizeTByReference();
        IntByReference err = new IntByReference();
        
        // There are equal number of keys and values
        Pointer serializationPtr = 
            WKNativeLibraryDirect.wkWalletConnectorCreateTransactionFromArguments(this.getPointer(),
                                                                                  keysArray,
                                                                                  valuesArray,
                                                                                  new SizeT(countOfKeyValuePairs),
                                                                                  serLengthRef,
                                                                                  err   );
        

        try {
            int serializationLen = UnsignedInts.checkedCast(serLengthRef.getValue().intValue());

            // Firstly deal with explicity communicated issues via error value
            if (serializationPtr.equals(Pointer.NULL)) {
                res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
            } else {
                res = WKResult.success(serializationPtr.getByteArray(0, serializationLen));
            }
        } finally {
            Native.free(Pointer.nativeValue(serializationPtr));
        }
        
        return res;

    }

    /** An object to hold both the serialization of the transaction and
     *  an indication of its signing status.
     */
    public class CreateTransactionFromSerializationResult {
        public final byte[]     serialization;
        public final boolean    isSigned;
        
        public CreateTransactionFromSerializationResult(byte[]    serialization,
                                                        boolean   isSigned ) {
            this.serialization = serialization;
            this.isSigned = isSigned;
        }
    }

    /** Creates a transaction serialization in the form of byte array,
     *  from a provided serialization, through WalletKit native code.
     *
     *  This implies that the input serialization may be processed, validated
     *  and re-serialized correctly.
     *
     * @param data The previously serialized transaction data]
     * @return A {@link WKResult} object containing the serialized transaction buffer, or a
     *         WalletKit native error representation.
     */
    public WKResult<CreateTransactionFromSerializationResult, WKWalletConnectorError>
    createTransactionFromSerialization(byte[] data) {

        WKResult res;
        SizeTByReference serLengthRef = new SizeTByReference();
        IntByReference isSignedRef = new IntByReference();
        IntByReference err = new IntByReference();
        
        Pointer serializationPtr = WKNativeLibraryDirect.wkWalletConnectorCreateTransactionFromSerialization(
                this.getPointer(),
                data,
                new SizeT(data.length),
                serLengthRef,
                isSignedRef,
                err );
        
        try {
            int serializationLen = UnsignedInts.checkedCast(serLengthRef.getValue().intValue());

            if (serializationPtr.equals(Pointer.NULL)) {
                res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
            } else {
                res = WKResult.success(
                        new CreateTransactionFromSerializationResult(
                                serializationPtr.getByteArray(0, serializationLen),
                                isSignedRef.getValue() > 0 ? true : false));
            }
        } finally {
            Native.free(Pointer.nativeValue(serializationPtr));
        }
        
        return res;
    }

    /** A wrapper of signed transaction serialization and transaction identifier
     */
    public class WKTransactionSigningResult {
        private final byte[] signedTransactionData;
        private final byte[] identifier;
        public WKTransactionSigningResult(byte[] signedTransactionData,
                                          byte[] transactionIdentifier) {
            this.signedTransactionData = signedTransactionData;
            this.identifier = transactionIdentifier;
        }

        public byte[] getIdentifier() { return this.identifier; }
        public byte[] getTransactionData() { return this.signedTransactionData; }
    }

    /** Signs the data assuming it is a validated, serialized transaction.
     *
     * @param transactionData The transaction to be signed
     * @param key The key, presumed to have private key information, to sign with
     * @return A {@link WKResult} object containing the {@link WKTransactionSigningResult}
     *         which has both the signed transaction serialization and the associated
     *         transaction identifier, or, a WalletKit native error representation.
     */
    public WKResult<WKTransactionSigningResult, WKWalletConnectorError>
    signTransaction(    byte[]  transactionData,
                        WKKey   key ) {

        WKResult            res;
        Pointer             keyPtr = key.getPointer();
        SizeTByReference    signedDataLenRef = new SizeTByReference();
        IntByReference      err = new IntByReference();
        SizeTByReference    transactionIdentifierLenRef = new SizeTByReference();
        PointerByReference  transactionIdentifier = new PointerByReference();

        // First find the required buffer length for the signature.
        Pointer signedTransactionDataPtr = WKNativeLibraryDirect.wkWalletConnectorSignTransactionData(
                this.getPointer(),
                transactionData,
                new SizeT(transactionData.length),
                keyPtr,
                transactionIdentifier,
                transactionIdentifierLenRef,
                signedDataLenRef,
                err  );
        try {
            int signedDataLength = UnsignedInts.checkedCast(signedDataLenRef.getValue().intValue());
            int transactionIdentifierLength = UnsignedInts.checkedCast(transactionIdentifierLenRef.getValue().intValue());

            // Firstly deal with the potential directly indicated error of signing
            if (signedTransactionDataPtr.equals(Pointer.NULL)) {
                res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
            } else {
                byte[] identifier = transactionIdentifier.getValue().getByteArray(0, transactionIdentifierLength);
                byte[] signedTransactionData = signedTransactionDataPtr.getByteArray(0, signedDataLength);
                res = WKResult.success(new WKTransactionSigningResult(signedTransactionData, identifier));
            }
        } finally {
            Native.free(Pointer.nativeValue(signedTransactionDataPtr));
            Native.free(Pointer.nativeValue(transactionIdentifier.getValue()));
        }

        return res;
    }

    /** A wrapper of digest and signature, which in the case of
     *  signed typed data, can be returned in one call.
     */
    public class WKTypedDataSigningResult {
        private final byte[] digest;
        private final byte[] signature;
        public WKTypedDataSigningResult(byte[] digest, byte[] signature) {
            this.digest = digest;
            this.signature = signature;
        }

        public byte[] getDigest() { return this.digest; }
        public byte[] getSignature() { return this.signature; }
    }

    /** Signs the typed data provided, which string should contain a valid JSON
     *  representing a typed data structure in the idiom of the networks typed
     *  data standards.
     *
     * @param typedData The typed data to be signed
     * @param key The signing key
     * @return A {@link WKResult} object containing the signature and digest wrapper,
     *         or a suitable {@link WKWalletConnectorError} in the case of a failure
     */
    public WKResult<WKTypedDataSigningResult, WKWalletConnectorError>
    sign( String    typedData,
          WKKey     key     ) {

        WKResult            res;
        IntByReference      err = new IntByReference();
        Pointer             keyPtr = key.getPointer();
        SizeTByReference    signedTypedDataLenRef = new SizeTByReference();
        SizeTByReference    digestDataLenRef = new SizeTByReference();
        PointerByReference  digestData = new PointerByReference();

        Pointer signedTypedDataPtr = WKNativeLibraryDirect.wkWalletConnectorSignTypedData(
                this.getPointer(),
                typedData,
                keyPtr,
                digestData,
                digestDataLenRef,
                signedTypedDataLenRef,
                err );

        try {
            int signedTypedDataLen = UnsignedInts.checkedCast(signedTypedDataLenRef.getValue().intValue());
            int digestLength = UnsignedInts.checkedCast(digestDataLenRef.getValue().intValue());

            if (signedTypedDataPtr.equals(Pointer.NULL)) {
                res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
            } else {
                byte[] signature = signedTypedDataPtr.getByteArray(0, signedTypedDataLen);
                byte[] digest = digestData.getValue().getByteArray(0, digestLength);
                WKTypedDataSigningResult signingResult = new WKTypedDataSigningResult(digest,
                                                                                      signature);
                res = WKResult.success(signingResult);
            }
        } finally {
            Native.free(Pointer.nativeValue(signedTypedDataPtr));
            Native.free(Pointer.nativeValue(digestData.getValue()));
        }
        return res;
    }

    public static Optional<WKWalletConnector> create(WKWalletManager coreManager) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkWalletConnectorCreate(coreManager.getPointer())
        ).transform(WKWalletConnector::new);
    }

    public void give() {
        Pointer thisPtr = this.getPointer();
        WKNativeLibraryDirect.wkWalletConnectorRelease(thisPtr);
    }
}
