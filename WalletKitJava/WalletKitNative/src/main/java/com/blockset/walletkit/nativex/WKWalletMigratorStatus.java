/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

public class WKWalletMigratorStatus extends Structure {

    // these values must be in sync with the enum values for BRCryptoWalletMigratorStatusType
    public static final int CRYPTO_WALLET_MIGRATOR_SUCCESS = 0;

    public int type;

    public WKWalletMigratorStatus() {
        super();
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("type");
    }

    public WKWalletMigratorStatus(int type) {
        super();
        this.type = type;
    }

    public WKWalletMigratorStatus(Pointer peer) {
        super(peer);
    }

    public static class ByReference extends WKWalletMigratorStatus implements Structure.ByReference {

    }

    public static class ByValue extends WKWalletMigratorStatus implements Structure.ByValue {

    }
}
