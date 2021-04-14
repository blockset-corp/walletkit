/*
 * Created by Ed Gamble.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.google.common.primitives.UnsignedInteger;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class BRCryptoClientCurrencyDenominationBundle extends PointerType {
    public static BRCryptoClientCurrencyDenominationBundle create(
            String name,
            String code,
            String symbol,
            UnsignedInteger decimals) {
        return new BRCryptoClientCurrencyDenominationBundle(
                CryptoLibraryDirect.cryptoClientCurrencyDenominationBundleCreate(
                name,
                code,
                symbol,
                decimals.byteValue()));
    }

    public BRCryptoClientCurrencyDenominationBundle() {
        super();
    }

    public BRCryptoClientCurrencyDenominationBundle(Pointer address) {
        super(address);
    }

}
