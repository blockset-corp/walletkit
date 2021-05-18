/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/29/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.nativex;

import com.blockset.walletkit.nativex.library.WKNativeLibraryDirect;
import com.blockset.walletkit.nativex.utility.SizeT;
import com.google.common.base.Optional;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.PointerType;

public class WKPaymentProtocolRequest extends PointerType {

    // must remain in sync with BRCryptoPaymentProtocolType
    private static final int BITPAY    = 0;
    private static final int BIP70     = 1;

    public static boolean validateForBitPay(WKNetwork cryptoNetwork,
                                            WKCurrency cryptoCurrency,
                                            WKWallet cryptoWallet) {
        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkPaymentProtocolRequestValidateSupported(
                BITPAY,
                cryptoNetwork.getPointer(),
                cryptoCurrency.getPointer(),
                cryptoWallet.getPointer()
        );
    }

    public static boolean validateForBip70(WKNetwork cryptoNetwork,
                                           WKCurrency cryptoCurrency,
                                           WKWallet cryptoWallet) {
        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkPaymentProtocolRequestValidateSupported(
                BIP70,
                cryptoNetwork.getPointer(),
                cryptoCurrency.getPointer(),
                cryptoWallet.getPointer()
        );
    }

    public static Optional<WKPaymentProtocolRequest> createForBip70(WKNetwork cryptoNetwork,
                                                                    WKCurrency cryptoCurrency,
                                                                    WKPayProtReqBitPayAndBip70Callbacks callbacks,
                                                                    byte[] serialization) {
        return Optional.fromNullable(
                WKNativeLibraryDirect.wkPaymentProtocolRequestCreateForBip70(
                        cryptoNetwork.getPointer(),
                        cryptoCurrency.getPointer(),
                        callbacks.toByValue(),
                        serialization,
                        new SizeT(serialization.length))
        ).transform(
                WKPaymentProtocolRequest::new
        );
    }

    public WKPaymentProtocolRequest() {
        super();
    }

    public WKPaymentProtocolRequest(Pointer address) {
        super(address);
    }

    public WKPaymentProtocolType getType() {
        Pointer thisPtr = this.getPointer();

        return WKPaymentProtocolType.fromCore(WKNativeLibraryDirect.wkPaymentProtocolRequestGetType(thisPtr));
    }

    public boolean isSecure() {
        Pointer thisPtr = this.getPointer();

        return WKBoolean.WK_TRUE == WKNativeLibraryDirect.wkPaymentProtocolRequestIsSecure(thisPtr);
    }

    public Optional<String> getMemo() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(WKNativeLibraryDirect.wkPaymentProtocolRequestGetMemo(thisPtr))
                .transform(v -> v.getString(0, "UTF-8"));
    }

    public Optional<String> getPaymentUrl() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(WKNativeLibraryDirect.wkPaymentProtocolRequestGetPaymentURL(thisPtr))
                .transform(v -> v.getString(0, "UTF-8"));
    }

    public Optional<WKAmount> getTotalAmount() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(WKNativeLibraryDirect.wkPaymentProtocolRequestGetTotalAmount(thisPtr))
                .transform(WKAmount::new);
    }

    public Optional<WKNetworkFee> getRequiredNetworkFee() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(WKNativeLibraryDirect.wkPaymentProtocolRequestGetRequiredNetworkFee(thisPtr))
                .transform(WKNetworkFee::new);
    }

    public Optional<WKAddress> getPrimaryTargetAddress() {
        Pointer thisPtr = this.getPointer();

        return Optional.fromNullable(WKNativeLibraryDirect.wkPaymentProtocolRequestGetPrimaryTargetAddress(thisPtr))
                .transform(WKAddress::new);
    }

    public Optional<String> getCommonName() {
        Pointer thisPtr = this.getPointer();

        Pointer returnValue = WKNativeLibraryDirect.wkPaymentProtocolRequestGetCommonName(thisPtr);
        try {
            return Optional.fromNullable(returnValue)
                    .transform(v -> v.getString(0, "UTF-8"));
        } finally {
            if (returnValue != null) Native.free(Pointer.nativeValue(returnValue));
        }
    }

    public WKPaymentProtocolError isValid() {
        Pointer thisPtr = this.getPointer();

        return WKPaymentProtocolError.fromCore(WKNativeLibraryDirect.wkPaymentProtocolRequestIsValid(thisPtr));
    }

    public void give() {
        Pointer thisPtr = this.getPointer();

        WKNativeLibraryDirect.wkPaymentProtocolRequestGive(thisPtr);
    }
}
