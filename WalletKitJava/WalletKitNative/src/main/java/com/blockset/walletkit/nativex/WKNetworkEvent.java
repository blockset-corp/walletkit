/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 5/31/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */

package com.blockset.walletkit.nativex;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

public class WKNetworkEvent extends Structure {

    public int typeEnum;

    public WKNetworkEvent() {
        super();
    }

    public WKNetworkEventType type() {
        return WKNetworkEventType.fromCore(typeEnum);
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("typeEnum");
    }

    public WKNetworkEvent(int type) {
        super();
        this.typeEnum = type;
    }

    public WKNetworkEvent(Pointer peer) {
        super(peer);
    }

    @Override
    public void read() {
        super.read();
    }

    public static class ByReference extends WKNetworkEvent implements Structure.ByReference {

    }

    public static class ByValue extends WKNetworkEvent implements Structure.ByValue {

    }

}
