/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 9/18/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;


import java.util.List;
import java.util.Locale;

import com.blockset.walletkit.SystemClient.CurrencyDenomination;
import com.google.common.collect.ImmutableList;
import com.google.common.primitives.UnsignedInteger;

/* package */
final class Blockchains {

    /* package */
    static final String ADDRESS_BRD_MAINNET = "0x558ec3152e2eb2174905cd19aea4e34a23de9ad6";

    /* package */
    static final String ADDRESS_BRD_TESTNET = "0x7108ca7c4718efa810457f228305c9c71390931a";

    /* package */
    static List<CurrencyDenomination> makeCurrencyDemominationsErc20 (String code, UnsignedInteger decimals) {
        String name = code.toUpperCase(Locale.ROOT);
        code = code.toLowerCase(Locale.ROOT);

        // TODO: confirm that its OK to poke through the inscrutability of
        //       the SystemClient interface and reach creation method for BRD/blockset
        //       in order to get something concrete. This should be OK? since this java
        //       is a BRD package and so might be assumed to be accessible to the
        //       JSON/blockset derivations. Otherwise the SystemClient interface
        //       will also have to impose 'create' methods that each implemenation
        //       provides
        return ImmutableList.of(
                com.blockset.walletkit.systemclient.brd.CurrencyDenomination.create(
                        String.format(Locale.ROOT, "%s Token INT", name),
                        String.format(Locale.ROOT, "%si", code),
                        UnsignedInteger.ZERO,
                        String.format(Locale.ROOT, "%si", code)
                ),
                com.blockset.walletkit.systemclient.brd.CurrencyDenomination.create(
                        String.format(Locale.ROOT, "%s Token", name),
                        code,
                        decimals,
                        code
                )
        );
    }


}
