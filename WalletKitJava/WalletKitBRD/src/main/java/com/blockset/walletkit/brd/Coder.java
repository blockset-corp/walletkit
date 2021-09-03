/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import androidx.annotation.Nullable;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKCoder;
import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

/* package */
final class Coder implements com.blockset.walletkit.Coder {

    @Nullable
    private static final Coder CODER_HEX = WKCoder.createHex().transform(Coder::create).orNull();

    @Nullable
    private static final Coder CODER_BASE58 = WKCoder.createBase58().transform(Coder::create).orNull();

    @Nullable
    private static final Coder CODER_BASE58CHECK = WKCoder.createBase58Check().transform(Coder::create).orNull();

    @Nullable
    private static final Coder CODER_BASE58RIPPLE = WKCoder.createBase58Ripple().transform(Coder::create).orNull();

    /* package */
    static Coder createForAlgorithm(Algorithm algorithm) {
        Coder coder = null;

        switch (algorithm) {
            case HEX:
                coder = CODER_HEX;
                break;
            case BASE58:
                coder = CODER_BASE58;
                break;
            case BASE58CHECK:
                coder = CODER_BASE58CHECK;
                break;
            case BASE58RIPPLE:
                coder = CODER_BASE58RIPPLE;
                break;
        }

        checkNotNull(coder);
        return coder;
    }

    private static Coder create(WKCoder core) {
        Coder coder = new Coder(core);
        ReferenceCleaner.register(coder, core::give);
        return coder;
    }

    private final WKCoder core;

    private Coder(WKCoder core) {
        this.core = core;
    }

    @Override
    public Optional<String> encode(byte[] source) {
        return core.encode(source);
    }

    @Override
    public Optional<byte[]> decode(String source) {
        return core.decode(source);
    }
}
