/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit;

import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.utility.CompletionHandler;
import com.google.common.base.Optional;

public interface WalletSweeper {

    Optional<? extends Amount> getBalance();

    void estimate(NetworkFee fee, CompletionHandler<TransferFeeBasis, FeeEstimationError> completion);

    Optional<? extends Transfer> submit(TransferFeeBasis feeBasis);
}
