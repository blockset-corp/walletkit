package com.breadwallet.corenative.crypto;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.Union;

import java.util.Arrays;
import java.util.List;

public class BRCryptoNetworkEvent extends Structure {

    public int typeEnum;

    public BRCryptoNetworkEvent() {
        super();
    }

    public BRCryptoNetworkEventType type() {
        return BRCryptoNetworkEventType.fromCore(typeEnum);
    }

    protected List<String> getFieldOrder() {
        return Arrays.asList("typeEnum");
    }

    public BRCryptoNetworkEvent(int type) {
        super();
        this.typeEnum = type;
    }

    public BRCryptoNetworkEvent(Pointer peer) {
        super(peer);
    }

    @Override
    public void read() {
        super.read();
    }

    public static class ByReference extends BRCryptoNetworkEvent implements Structure.ByReference {

    }

    public static class ByValue extends BRCryptoNetworkEvent implements Structure.ByValue {

    }

}
