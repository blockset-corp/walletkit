/*
 * Created by Bryan Goring <bryan.goring@breadwallet.com> on 08/24/21.
 * Copyright (c) 2021 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
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

public class WKWalletConnector extends PointerType {

    public WKWalletConnector() {
        super ();
    }

    public WKWalletConnector(Pointer address) {
        super (address);
    }

    /* May wrap return types with an WKError which can be promoted to class
     * by the connector?
     * 
     * public class DigestResult {
     *  Optional<byte[]> digest;
     *  int wkError;
     *  }
     */
    public /*DigestResult*/Optional<byte[]> getDigest(byte[] message, boolean prefix) {

        SizeTByReference digestLenRef = new SizeTByReference();
        IntByReference err = new IntByReference();
        
        Pointer digestPtr = WKNativeLibraryDirect.wkWalletConnectorGetDigest(this.getPointer(),
                                                                             message,
                                                                             new SizeT(message.length),
                                                                             prefix,
                                                                             digestLenRef,
                                                                             err    );
        Optional<byte[]> digestOptional = Optional.absent();
        try {
            
            // TODO: properly treat WKWalletConnectorError
            int digestLen = UnsignedInts.checkedCast(digestLenRef.getValue().intValue());
            if (!digestPtr.equals(Pointer.NULL) && digestLen > 0) {
                byte[] digest = digestPtr.getByteArray(0, digestLen);
                digestOptional = Optional.of(digest);
            } else {
                // Log error
            }
        } finally {
            Native.free(Pointer.nativeValue(digestPtr));
        }

        return digestOptional;
    }
    
    
    // Similar question wrt errors as in getDigest
    public Optional<byte[]> sign(byte[] data, WKKey key) {
        
        Pointer keyPtr = key.getPointer();
        SizeTByReference signatureLenRef = new SizeTByReference();
        IntByReference err = new IntByReference();
        Optional<byte[]> signatureOptional = Optional.absent();

        // First find the required buffer length for the signature.
        Pointer signaturePtr = WKNativeLibraryDirect.wkWalletConnectorSignData(this.getPointer(),
                                                                               data,
                                                                               new SizeT(data.length),
                                                                               keyPtr,
                                                                               signatureLenRef,
                                                                               err  );
        try {
            
            // TODO: properly treat WKWalletConnectorError
            int signatureLen = UnsignedInts.checkedCast(signatureLenRef.getValue().intValue());
            if (!signaturePtr.equals(Pointer.NULL) && signatureLen > 0) {
                byte[] signature = signaturePtr.getByteArray(0, signatureLen);
                signatureOptional = Optional.of(signature);
            } else {
                // Log error
            }
        } finally {
            Native.free(Pointer.nativeValue(signaturePtr));
        }

        return signatureOptional;
    }
    
    public Optional<byte[]> createTransactionFromArguments(
            List<String> keys, 
            List<String> values,
            int          countOfKeyValuePairs) {
        
        StringArray keysArray = new StringArray(keys.toArray(new String[0]), "UTF-8");
        StringArray valuesArray = new StringArray(values.toArray(new String[0]), "UTF-8");
        SizeTByReference serLengthRef = new SizeTByReference();
        IntByReference err = new IntByReference();
        Optional<byte[]> serializationOptional = Optional.absent();
        
        // There are equal number of keys and values
        Pointer serializationPtr = 
            WKNativeLibraryDirect.wkWalletConnectorCreateTransactionFromArguments(this.getPointer(),
                                                                                  keysArray,
                                                                                  valuesArray,
                                                                                  new SizeT(countOfKeyValuePairs),
                                                                                  serLengthRef,
                                                                                  err   );
        
        // TODO: properly treat WKWalletConnectorError
        try {
            int serializationLen = UnsignedInts.checkedCast(serLengthRef.getValue().intValue());
            
            // Failure deduced by bad return. Should we have a representative error code out of native
            // and returned in some fasion (return value, or argument)
            if (!serializationPtr.equals(Pointer.NULL) && serializationLen > 0) {
                byte[] serialization = serializationPtr.getByteArray(0, serializationLen);
                serializationOptional = Optional.of(serialization);
            }
        } finally {
            Native.free(Pointer.nativeValue(serializationPtr));
        }
        
        return serializationOptional;
        
    }

    public class CreateTransactionFromSerializationResult {
        public final Optional<byte[]>   serializationOptional;
        public final boolean            isSigned;
        // Plus error when revisiting errors at this level
        
        public CreateTransactionFromSerializationResult(Optional<byte[]>    serOptional,
                                                        boolean             isSigned ) {
            this.serializationOptional = serOptional;
            this.isSigned = isSigned;
        }
    }
    
    public CreateTransactionFromSerializationResult createTransactionFromSerialization(byte[] data) {

        SizeTByReference serLengthRef = new SizeTByReference();
        Optional<byte[]> serializationOptional = Optional.absent();
        IntByReference isSignedRef = new IntByReference();
        IntByReference err = new IntByReference();
        
        Pointer serializationPtr = WKNativeLibraryDirect.wkWalletConnectorCreateTransactionFromSerialization(
                this.getPointer(),
                data,
                new SizeT(data.length),
                serLengthRef,
                isSignedRef,
                err );
        
        // TODO: properly treat WKWalletConnectorError
        try {
            int serializationLen = UnsignedInts.checkedCast(serLengthRef.getValue().intValue());
            
            // Failure deduced by bad return. Should we have a representative error code out of native
            // and returned in some fasion (return value, or argument)
            if (!serializationPtr.equals(Pointer.NULL) && serializationLen > 0) {
                byte[] serialization = serializationPtr.getByteArray(0, serializationLen);
                serializationOptional = Optional.of(serialization);
            }
        } finally {
            Native.free(Pointer.nativeValue(serializationPtr));
        }
        
        return new CreateTransactionFromSerializationResult(serializationOptional,
                                                            isSignedRef.getValue() > 0 ? true : false);
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

    /* TODO: Hook WKWalletConnector methods to the actual wkMethods exposed by WKWalletConnector.h
       as needed by WalletConnector.
     */
}
