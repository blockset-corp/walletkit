/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.support.WKSecret;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.google.common.base.Optional;
import com.sun.jna.Memory;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;
import com.sun.jna.StringArray;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.List;

public class WKKey extends PointerType {

    public WKKey(Pointer address) {
        super(address);
    }

    public WKKey() {
        super();
    }

    public static boolean isProtectedPrivateKeyString(byte[] keyString) {
        // ensure string is null terminated
        keyString = Arrays.copyOf(keyString, keyString.length + 1);
        try {
            Memory keyMemory = new Memory(keyString.length);
            try {
                keyMemory.write(0, keyString, 0, keyString.length);
                ByteBuffer keyBuffer = keyMemory.getByteBuffer(0, keyString.length);

                return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkKeyIsProtectedPrivate(keyBuffer);
            } finally {
                keyMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(keyString, (byte) 0);
        }
    }

    public static Optional<WKKey> createFromPhrase(byte[] phraseUtf8, List<String> words) {
        StringArray wordsArray = new StringArray(words.toArray(new String[0]), "UTF-8");

        // ensure string is null terminated
        phraseUtf8 = Arrays.copyOf(phraseUtf8, phraseUtf8.length + 1);
        try {
            Memory phraseMemory = new Memory(phraseUtf8.length);
            try {
                phraseMemory.write(0, phraseUtf8, 0, phraseUtf8.length);
                ByteBuffer phraseBuffer = phraseMemory.getByteBuffer(0, phraseUtf8.length);

                return Optional.fromNullable(
                        WKNativeLibraryDirect.wkKeyCreateFromPhraseWithWords(
                                phraseBuffer,
                                wordsArray
                        )
                ).transform(WKKey::new);
            } finally {
                phraseMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(phraseUtf8, (byte) 0);
        }
    }

    public static Optional<WKKey> createFromPrivateKeyString(byte[] keyString) {
        // ensure string is null terminated
        keyString = Arrays.copyOf(keyString, keyString.length + 1);
        try {
            Memory keyMemory = new Memory(keyString.length);
            try {
                keyMemory.write(0, keyString, 0, keyString.length);
                ByteBuffer keyBuffer = keyMemory.getByteBuffer(0, keyString.length);

                return Optional.fromNullable(
                        WKNativeLibraryDirect.wkKeyCreateFromStringPrivate(
                                keyBuffer
                        )
                ).transform(WKKey::new);
            } finally {
                keyMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(keyString, (byte) 0);
        }
    }

    public static Optional<WKKey> createFromPrivateKeyString(byte[] keyString, byte[] phraseString) {
        // ensure strings are null terminated
        keyString = Arrays.copyOf(keyString, keyString.length + 1);
        phraseString = Arrays.copyOf(phraseString, phraseString.length + 1);
        try {
            Memory memory = new Memory(keyString.length + phraseString.length);
            try {
                memory.write(0, keyString, 0, keyString.length);
                memory.write(keyString.length, phraseString, 0, phraseString.length);

                ByteBuffer keyBuffer = memory.getByteBuffer(0, keyString.length);
                ByteBuffer phraseBuffer = memory.getByteBuffer(keyString.length, phraseString.length);

                return Optional.fromNullable(
                        WKNativeLibraryDirect.wkKeyCreateFromStringProtectedPrivate(
                                keyBuffer,
                                phraseBuffer
                        )
                ).transform(WKKey::new);
            } finally {
                memory.clear();
            }
        } finally {
            // clear out our copies; caller responsible for original arrays
            Arrays.fill(keyString, (byte) 0);
            Arrays.fill(phraseString, (byte) 0);
        }
    }

    public static Optional<WKKey> createFromPublicKeyString(byte[] keyString) {
        // ensure string is null terminated
        keyString = Arrays.copyOf(keyString, keyString.length + 1);
        try {
            Memory keyMemory = new Memory(keyString.length);
            try {
                keyMemory.write(0, keyString, 0, keyString.length);
                ByteBuffer keyBuffer = keyMemory.getByteBuffer(0, keyString.length);

                return Optional.fromNullable(
                        WKNativeLibraryDirect.wkKeyCreateFromStringPublic(
                                keyBuffer
                        )
                ).transform(WKKey::new);
            } finally {
                keyMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(keyString, (byte) 0);
        }
    }

    public static Optional<WKKey> createForPigeon(WKKey key, byte[] nonce) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkKeyCreateForPigeon(
                        key.getPointer(),
                        nonce,
                        new SizeT(nonce.length)
                )
        ).transform(WKKey::new);
    }

    public static Optional<WKKey> createForBIP32ApiAuth(byte[] phraseUtf8, List<String> words) {
        StringArray wordsArray = new StringArray(words.toArray(new String[0]), "UTF-8");

        // ensure string is null terminated
        phraseUtf8 = Arrays.copyOf(phraseUtf8, phraseUtf8.length + 1);
        try {
            Memory phraseMemory = new Memory(phraseUtf8.length);
            try {
                phraseMemory.write(0, phraseUtf8, 0, phraseUtf8.length);
                ByteBuffer phraseBuffer = phraseMemory.getByteBuffer(0, phraseUtf8.length);

                return Optional.fromNullable(
                        WKNativeLibraryDirect.wkKeyCreateForBIP32ApiAuth(
                                phraseBuffer,
                                wordsArray
                        )
                ).transform(WKKey::new);
            } finally {
                phraseMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(phraseUtf8, (byte) 0);
        }
    }

    public static Optional<WKKey> createForBIP32BitID(byte[] phraseUtf8, int index, String uri, List<String> words) {
        StringArray wordsArray = new StringArray(words.toArray(new String[0]), "UTF-8");

        // ensure string is null terminated
        phraseUtf8 = Arrays.copyOf(phraseUtf8, phraseUtf8.length + 1);
        try {
            Memory phraseMemory = new Memory(phraseUtf8.length);
            try {
                phraseMemory.write(0, phraseUtf8, 0, phraseUtf8.length);
                ByteBuffer phraseBuffer = phraseMemory.getByteBuffer(0, phraseUtf8.length);

                return Optional.fromNullable(
                        WKNativeLibraryDirect.wkKeyCreateForBIP32BitID(
                                phraseBuffer,
                                index,
                                uri,
                                wordsArray
                        )
                ).transform(WKKey::new);

            } finally {
                phraseMemory.clear();
            }
        } finally {
            // clear out our copy; caller responsible for original array
            Arrays.fill(phraseUtf8, (byte) 0);
        }
    }

    public static Optional<WKKey> cryptoKeyCreateFromSecret(byte[] secret) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkKeyCreateFromSecret(
                        new WKSecret(secret).toByValue()
                )
        ).transform(WKKey::new);
    }

    public byte[] encodeAsPrivate() {
        Pointer thisPtr = this.getPointer();

        Pointer ptr = WKNativeLibraryDirect.wkKeyEncodePrivate(thisPtr);
        try {
            return ptr.getByteArray(0, (int) ptr.indexOf(0, (byte) 0));
        } finally {
            Native.free(Pointer.nativeValue(ptr));
        }
    }

    public byte[] encodeAsPublic() {
        Pointer thisPtr = this.getPointer();

        Pointer ptr = WKNativeLibraryDirect.wkKeyEncodePublic(thisPtr);
        try {
            return ptr.getByteArray(0, (int) ptr.indexOf(0, (byte) 0));
        } finally {
            Native.free(Pointer.nativeValue(ptr));
        }
    }

    public boolean hasSecret() {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkKeyHasSecret(thisPtr);
    }

    public byte[] getSecret() {
        Pointer thisPtr = this.getPointer();

        return WKNativeLibraryDirect.wkKeyGetSecret(thisPtr).u8;
    }

    public boolean privateKeyMatch(WKKey other) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkKeySecretMatch(thisPtr, other.getPointer());
    }

    public boolean publicKeyMatch(WKKey other) {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkKeyPublicMatch(thisPtr, other.getPointer());
    }

    public void providePublicKey(int useCompressed, int compressed) {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkKeyProvidePublicKey(thisPtr, useCompressed, compressed);
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkKeyGive(thisPtr);
    }
}
