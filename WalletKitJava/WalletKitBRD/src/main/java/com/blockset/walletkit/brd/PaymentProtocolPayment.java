/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/29/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.nativex.cleaner.ReferenceCleaner;
import com.blockset.walletkit.nativex.WKPaymentProtocolPayment;
import com.google.common.base.Optional;

/* package */
final class PaymentProtocolPayment implements com.blockset.walletkit.PaymentProtocolPayment {

    /* package */
    static Optional<PaymentProtocolPayment> create(PaymentProtocolRequest request, Transfer transfer, Address target) {
        return WKPaymentProtocolPayment.create(
                request.getBRCryptoPaymentProtocolRequest(),
                transfer.getCoreBRCryptoTransfer(),
                target.getCoreBRCryptoAddress()
        ).transform(PaymentProtocolPayment::create);
    }

    /* package */
    static PaymentProtocolPayment create(WKPaymentProtocolPayment core) {;
        PaymentProtocolPayment payment = new PaymentProtocolPayment(core);
        ReferenceCleaner.register(payment, core::give);
        return payment;
    }

    private final WKPaymentProtocolPayment core;

    private PaymentProtocolPayment(WKPaymentProtocolPayment core) {
        this.core = core;
    }

    @Override
    public Optional<byte[]> encode() {
        return core.encode();
    }
}
