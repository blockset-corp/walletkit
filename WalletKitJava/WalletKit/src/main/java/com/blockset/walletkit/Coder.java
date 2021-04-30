/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit;

import com.google.common.base.Optional;

public interface Coder {

    enum Algorithm {
        HEX,
        BASE58,
        BASE58CHECK,
        BASE58RIPPLE
    }

    static Coder createForAlgorithm(Algorithm algorithm) {
        return Api.getProvider().coderPrivider().createCoderForAlgorithm(algorithm);
    }

    Optional<String> encode(byte[] source);

    Optional<byte[]> decode(String source);
}
