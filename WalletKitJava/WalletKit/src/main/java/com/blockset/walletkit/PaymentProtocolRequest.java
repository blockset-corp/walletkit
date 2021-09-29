/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 10/28/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit;

import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.errors.PaymentProtocolError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;

public interface PaymentProtocolRequest {

    static boolean checkPaymentMethodSupported(Wallet                     wallet,
                                               PaymentProtocolRequestType protocolType) {
        return Api.getProvider().paymentProvider().checkPaymentMethodSupported(wallet,
                                                                               protocolType);
    }

    static Optional<PaymentProtocolRequest> createForBip70(Wallet wallet, byte[] serialization) {
        return Api.getProvider().paymentProvider().createRequestForBip70(wallet, serialization);
    }

    static Optional<PaymentProtocolRequest> createForBitPay(Wallet wallet, String json) {
        return Api.getProvider().paymentProvider().createRequestForBitPay(wallet, json);
    }

    PaymentProtocolRequestType getType();

    boolean isSecure();

    Optional<String> getMemo();

    Optional<String> getPaymentUrl();

    Optional<? extends Amount> getTotalAmount();

    Optional<? extends Address> getPrimaryTarget();

    Optional<String> getCommonName();

    Optional<? extends NetworkFee> getRequiredNetworkFee();

    Optional<PaymentProtocolError> validate();

    void estimate(NetworkFee fee, CompletionHandler<TransferFeeBasis, FeeEstimationError> completion);

    Optional<? extends Transfer> createTransfer(TransferFeeBasis feeBasis);

    boolean signTransfer(Transfer transfer, byte[] phraseUtf8);

    void submitTransfer(Transfer transfer);

    Optional<? extends PaymentProtocolPayment> createPayment(Transfer transfer);
}
