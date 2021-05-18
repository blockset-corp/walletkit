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
import com.google.common.base.Optional;
import com.google.common.primitives.Ints;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import static com.google.common.base.Preconditions.checkState;

public class WKCoder extends PointerType {

    // these must mirror BRCryptoCoderType's enum values
    private static final int CRYPTO_CODER_HEX          = 0;
    private static final int CRYPTO_CODER_BASE58       = 1;
    private static final int CRYPTO_CODER_BASE58CHECK  = 2;
    private static final int CRYPTO_CODER_BASE58RIPPLE = 3;

    public static Optional<WKCoder> createHex() {
        return create(CRYPTO_CODER_HEX);
    }

    public static Optional<WKCoder> createBase58() {
        return create(CRYPTO_CODER_BASE58);
    }

    public static Optional<WKCoder> createBase58Check() {
        return create(CRYPTO_CODER_BASE58CHECK);
    }

    public static Optional<WKCoder> createBase58Ripple() {
        return create(CRYPTO_CODER_BASE58RIPPLE);
    }

    private static Optional<WKCoder> create(int alg) {
        return Optional.fromNullable(WKNativeLibraryDirect.wkCoderCreate(alg)).transform(WKCoder::new);
    }

    public WKCoder() {
        super();
    }

    public WKCoder(Pointer address) {
        super(address);
    }

    public Optional<String> encode(byte[] input) {
        Pointer thisPtr = this.getPointer();

        SizeT length = WKNativeLibraryDirect.wkCoderEncodeLength(thisPtr, input, new SizeT(input.length));
        int lengthAsInt = Ints.checkedCast(length.longValue());
        if (0 == lengthAsInt) return Optional.absent();

        byte[] output = new byte[lengthAsInt];
        int result = WKNativeLibraryDirect.wkCoderEncode(thisPtr, output, new SizeT(output.length), input, new SizeT(input.length));
        return result == WKBoolean.WK_TRUE ? Optional.of(utf8BytesToString(output)) : Optional.absent();
    }

    public Optional<byte[]> decode(String inputStr) {
        Pointer thisPtr = this.getPointer();

        // ensure string is null terminated
        byte[] inputWithoutTerminator = inputStr.getBytes(StandardCharsets.UTF_8);
        byte[] inputWithTerminator = Arrays.copyOf(inputWithoutTerminator, inputWithoutTerminator.length + 1);

        SizeT length = WKNativeLibraryDirect.wkCoderDecodeLength(thisPtr, inputWithTerminator);
        int lengthAsInt = Ints.checkedCast(length.longValue());
        if (0 == lengthAsInt) return Optional.absent();

        byte[] output = new byte[lengthAsInt];
        int result = WKNativeLibraryDirect.wkCoderDecode(thisPtr, output, new SizeT(output.length), inputWithTerminator);
        return result == WKBoolean.WK_TRUE ? Optional.of(output) : Optional.absent();
    }

    private static String utf8BytesToString(byte[] message) {
        int end = 0;
        int len = message.length;
        while ((end < len) && (message[end] != 0)) {
            end++;
        }
        return new String(message, 0, end, StandardCharsets.UTF_8);
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkCoderGive(thisPtr);
    }
}
