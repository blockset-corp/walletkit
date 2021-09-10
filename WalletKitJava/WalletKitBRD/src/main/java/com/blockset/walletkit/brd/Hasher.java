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
import com.blockset.walletkit.nativex.WKHasher;
import com.google.common.base.Optional;

import static com.google.common.base.Preconditions.checkNotNull;

/* package */
final class Hasher implements com.blockset.walletkit.Hasher {

    @Nullable
    private static final Hasher HASHER_SHA1 = WKHasher.createSha1().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_SHA224 = WKHasher.createSha224().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_SHA256 = WKHasher.createSha256().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_SHA256_2 = WKHasher.createSha256_2().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_SHA384 = WKHasher.createSha384().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_SHA512 = WKHasher.createSha512().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_SHA3 = WKHasher.createSha3().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_RMD160 = WKHasher.createRmd160().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_HASH160 = WKHasher.createHash160().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_KECCAK256 = WKHasher.createKeccak256().transform(Hasher::create).orNull();

    @Nullable
    private static final Hasher HASHER_MD5 = WKHasher.createMd5().transform(Hasher::create).orNull();

    /* package */
    static Hasher createForAlgorithm(Algorithm algorithm) {
        Hasher hasher = null;

        switch (algorithm) {
            case SHA1:
                hasher = HASHER_SHA1;
                break;
            case SHA224:
                hasher = HASHER_SHA224;
                break;
            case SHA256:
                hasher = HASHER_SHA256;
                break;
            case SHA256_2:
                hasher = HASHER_SHA256_2;
                break;
            case SHA384:
                hasher = HASHER_SHA384;
                break;
            case SHA512:
                hasher = HASHER_SHA512;
                break;
            case SHA3:
                hasher = HASHER_SHA3;
                break;
            case RMD160:
                hasher = HASHER_RMD160;
                break;
            case HASH160:
                hasher = HASHER_HASH160;
                break;
            case KECCAK256:
                hasher = HASHER_KECCAK256;
                break;
            case MD5:
                hasher = HASHER_MD5;
                break;
        }

        checkNotNull(hasher);
        return hasher;
    }

    private static Hasher create(WKHasher core) {
        Hasher hasher = new Hasher(core);
        ReferenceCleaner.register(hasher, core::give);
        return hasher;
    }

    private final WKHasher core;

    private Hasher(WKHasher core) {
        this.core = core;
    }

    @Override
    public Optional<byte[]> hash(byte[] data) {
        return core.hash(data);
    }
}
