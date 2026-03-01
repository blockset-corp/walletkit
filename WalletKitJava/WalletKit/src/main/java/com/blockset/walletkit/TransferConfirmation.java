/*
 * TransferConfirmation
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 1/22/18.
 * Copyright (c) 2018-2019 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit;

import com.google.common.base.Optional;
import com.google.common.primitives.UnsignedLong;

import java.util.Date;
import java.util.concurrent.TimeUnit;

public final class TransferConfirmation {

    private final UnsignedLong blockNumber;
    private final UnsignedLong transactionIndex;
    private final Date timestamp;
    private final Optional<Amount> fee;
    private final TransferIncludeStatus status;

    public TransferConfirmation(UnsignedLong blockNumber,
                                UnsignedLong transactionIndex,
                                UnsignedLong timestamp,
                                Optional<Amount> fee,
                                TransferIncludeStatus status) {
        this.blockNumber = blockNumber;
        this.transactionIndex = transactionIndex;
        this.timestamp = new Date(TimeUnit.SECONDS.toMillis(timestamp.longValue()));
        this.fee = fee;
        this.status = status;
    }

    public UnsignedLong getBlockNumber() {
        return blockNumber;
    }

    public UnsignedLong getTransactionIndex() {
        return transactionIndex;
    }

    public Date getConfirmationTime() {
        return timestamp;
    }

    public Optional<Amount> getFee() {
        return fee;
    }

    public TransferIncludeStatus getStatus() {
        return status;
    }
}
