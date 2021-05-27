package com.breadwallet.corenative.crypto;

import com.breadwallet.corenative.CryptoLibraryDirect;
import com.breadwallet.corenative.CryptoLibraryIndirect;
import com.breadwallet.corenative.utility.SizeT;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

import java.util.List;

import javax.annotation.Nullable;

public class BRCryptoClientCurrencyBundle extends PointerType {

    public static BRCryptoClientCurrencyBundle create(
            String uids,
            String name,
            String code,
            String type,
            String blockchainId,
            @Nullable String address,
            boolean verified,
            List<BRCryptoClientCurrencyDenominationBundle> denomniations) {

        return new BRCryptoClientCurrencyBundle(
                CryptoLibraryIndirect.cryptoClientCurrencyBundleCreate(
                        uids,
                        name,
                        code,
                        type,
                        blockchainId,
                        address,
                        verified,
                        new SizeT(denomniations.size()),
                        denomniations.toArray(new BRCryptoClientCurrencyDenominationBundle[denomniations.size()])));
    }

    public void release () {
        CryptoLibraryDirect.cryptoClientCurrencyBundleRelease(
                this.getPointer());
    }

    public BRCryptoClientCurrencyBundle() {
        super();
    }

    public BRCryptoClientCurrencyBundle(Pointer address) {
        super(address);
    }
}
