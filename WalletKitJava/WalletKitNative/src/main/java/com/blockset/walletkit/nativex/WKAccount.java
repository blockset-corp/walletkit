/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
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
import com.google.common.primitives.UnsignedLong;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.StringArray;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.concurrent.TimeUnit;

import static com.google.common.base.Preconditions.checkArgument;

public class WKAccount extends PointerType {

    public static Optional<WKAccount> createFromPhrase(byte[] phraseUtf8, UnsignedLong timestamp, String uids, boolean isMainnet) {
        // ensure string is null terminated
        phraseUtf8 = Arrays.copyOf(phraseUtf8, phraseUtf8.length + 1);
        try {
            Memory phraseMemory = new Memory(phraseUtf8.length);
            try {
                phraseMemory.write(0, phraseUtf8, 0, phraseUtf8.length);
                ByteBuffer phraseBuffer = phraseMemory.getByteBuffer(0, phraseUtf8.length);

                return Optional.fromNullable(
                        WKNativeLibraryDirect.wkAccountCreate(
                                phraseBuffer,
                                timestamp.longValue(),
                                uids,
                                isMainnet ? WKBoolean.WK_TRUE : WKBoolean.WK_FALSE
                        )
                ).transform(WKAccount::new);
            } finally {
                phraseMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(phraseUtf8, (byte) 0);
        }
    }

    public static Optional<WKAccount> createFromSerialization(byte[] serialization, String uids) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkAccountCreateFromSerialization(
                        serialization,
                        new SizeT(serialization.length),
                        uids
                )
        ).transform(WKAccount::new);
    }

    public static byte[] generatePhrase(List<String> words) {
        checkArgument(WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAccountValidateWordsList(new SizeT(words.size())));

        StringArray wordsArray = new StringArray(words.toArray(new String[0]), "UTF-8");

        Pointer phrasePtr = WKNativeLibraryDirect.wkAccountGeneratePaperKey(wordsArray);
        try {
            return phrasePtr.getByteArray(0, (int) phrasePtr.indexOf(0, (byte) 0));
        } finally {
            Native.free(Pointer.nativeValue(phrasePtr));
        }
    }

    public static boolean validatePhrase(byte[] phraseUtf8, List<String> words) {
        checkArgument(WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAccountValidateWordsList(new SizeT(words.size())));

        StringArray wordsArray = new StringArray(words.toArray(new String[0]), "UTF-8");

        // ensure string is null terminated
        phraseUtf8 = Arrays.copyOf(phraseUtf8, phraseUtf8.length + 1);
        try {
            Memory phraseMemory = new Memory(phraseUtf8.length);
            try {
                phraseMemory.write(0, phraseUtf8, 0, phraseUtf8.length);
                ByteBuffer phraseBuffer = phraseMemory.getByteBuffer(0, phraseUtf8.length);

                return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAccountValidatePaperKey(phraseBuffer, wordsArray);
            } finally {
                phraseMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(phraseUtf8, (byte) 0);
        }
    }

    public WKAccount() {
        super();
    }

    public WKAccount(Pointer address) {
        super(address);
    }

    public Date getTimestamp() {
        Pointer thisPtr = this.getPointer();

        return new Date(TimeUnit.SECONDS.toMillis(WKNativeLibraryDirect.wkAccountGetTimestamp(thisPtr)));
    }

    public String getUids() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkAccountGetUids(thisPtr).getString(0, "UTF-8");
    }

    public String getFilesystemIdentifier() {
        Pointer thisPtr = this.getPointer();

        Pointer ptr = WKNativeLibraryDirect.wkAccountGetFileSystemIdentifier(thisPtr);
        try {
            return ptr.getString(0, "UTF-8");
        } finally {
            Native.free(Pointer.nativeValue(ptr));
        }
    }

    public byte[] serialize() {
        Pointer thisPtr = this.getPointer();

        SizeTByReference bytesCount = new SizeTByReference();
        Pointer serializationPtr = WKNativeLibraryDirect.wkAccountSerialize(thisPtr, bytesCount);
        try {
            return serializationPtr.getByteArray(0, UnsignedInts.checkedCast(bytesCount.getValue().longValue()));
        } finally {
            Native.free(Pointer.nativeValue(serializationPtr));
        }
    }

    public boolean validate(byte[] serialization) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkAccountValidateSerialization(thisPtr,
                serialization, new SizeT(serialization.length));
    }

    public boolean isInitialized(WKNetwork network) {
        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkNetworkIsAccountInitialized(
                network.getPointer(),
                this.getPointer());
    }

    public byte[] getInitializationData(WKNetwork network) {
        Pointer thisPtr = this.getPointer();

        SizeTByReference bytesCount = new SizeTByReference();
        Pointer serializationPtr = WKNativeLibraryDirect.wkNetworkGetAccountInitializationData(
                network.getPointer(),
                thisPtr,
                bytesCount);
        if (null == serializationPtr) return null;

        try {
            return serializationPtr.getByteArray(0, UnsignedInts.checkedCast(bytesCount.getValue().longValue()));
        } finally {
            Native.free(Pointer.nativeValue(serializationPtr));
        }
    }

    public void initialize(WKNetwork network, byte[] data) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkNetworkInitializeAccount(
                network.getPointer(),
                thisPtr,
                data,
                new SizeT(data.length));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkAccountGive(thisPtr);
    }
}
