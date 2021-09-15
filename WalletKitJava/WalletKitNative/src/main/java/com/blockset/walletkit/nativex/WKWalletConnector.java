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

    /** Gets a digest of the message bytes using native WalletKit calls
     *
     * @param message The message to be processed
     * @param prefix An optional prefix to add to the message before digestion
     * @return A {@link WKResult} object containing the digest byte buffer, or
     *         WalletKit native error representation.
     */
    public WKResult<byte[], WKWalletConnectorError>
    getDigest(  byte[]      message,
                boolean     prefix  ) {

        WKResult res;
        SizeTByReference digestLenRef = new SizeTByReference();
        IntByReference err = new IntByReference();
        
        Pointer digestPtr = WKNativeLibraryDirect.wkWalletConnectorGetDigest(this.getPointer(),
                                                                             message,
                                                                             new SizeT(message.length),
                                                                             prefix,
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

    /** Signs the data assuming it is a validated, serialized transaction.
     *
     * @param transactionData The transaction to be signed
     * @param key The key, presumed to have private key information, to sign with
     * @return A {@link WKResult} object containing the signature buffer, or a
     *         WalletKit native error representation.
     */
    public WKResult<byte[], WKWalletConnectorError>
    signTransaction(    byte[]  transactionData,
                        WKKey   key ) {

        WKResult res;
        Pointer keyPtr = key.getPointer();
        SizeTByReference signedDataLenRef = new SizeTByReference();
        IntByReference err = new IntByReference();

        // First find the required buffer length for the signature.
        Pointer signedTransactionDataPtr = WKNativeLibraryDirect.wkWalletConnectorSignTransactionData(
                this.getPointer(),
                transactionData,
                new SizeT(transactionData.length),
                keyPtr,
                signedDataLenRef,
                err  );
        try {

            int signatureLen = UnsignedInts.checkedCast(signedDataLenRef.getValue().intValue());

            // Firstly deal with the potential directly indicated error of signing
            if (signedTransactionDataPtr.equals(Pointer.NULL)) {
                res = WKResult.failure(WKWalletConnectorError.fromCore(err.getValue()));
            } else {
                res = WKResult.success(signedTransactionDataPtr.getByteArray(0, signatureLen));
            }
        } finally {
            Native.free(Pointer.nativeValue(signedTransactionDataPtr));
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
