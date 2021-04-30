/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex.support;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

public class WKSecret extends Structure {

    public byte[] u8 = new byte[256 / 8]; // UInt256

    public WKSecret() {
        super();
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("u8");
    }

    public WKSecret(byte u8[]) {
        super();
        if ((u8.length != this.u8.length)) {
            throw new IllegalArgumentException("Wrong array size!");
        }
        this.u8 = u8;
    }

    public WKSecret(Pointer peer) {
        super(peer);
    }

    public ByValue toByValue() {
        ByValue other = new ByValue();
        System.arraycopy(this.u8, 0, other.u8, 0, this.u8.length);
        return other;
    }

    public static class ByReference extends WKSecret implements Structure.ByReference {
    }

    public static class ByValue extends WKSecret implements Structure.ByValue {
    }
}
