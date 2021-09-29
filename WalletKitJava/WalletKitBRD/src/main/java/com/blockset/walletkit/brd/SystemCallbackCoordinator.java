/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.brd;

import com.blockset.walletkit.nativex.utility.Cookie;
import com.blockset.walletkit.Amount;
import com.blockset.walletkit.errors.FeeEstimationError;
import com.blockset.walletkit.errors.LimitEstimationError;
import com.blockset.walletkit.utility.CompletionHandler;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

/* package */
final class SystemCallbackCoordinator {

    private static final AtomicInteger HANDLER_IDS = new AtomicInteger(0);

    private final ScheduledExecutorService executor;

    private final Map<Cookie, CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError>> handlers;

    /* package */
    SystemCallbackCoordinator(ScheduledExecutorService executor) {
        this.executor = executor;
        this.handlers = new ConcurrentHashMap<>();
    }

    // Operation callbacks

    /* package */
    Cookie registerFeeBasisEstimateHandler(CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError> handler) {
        Cookie cookie = new Cookie(HANDLER_IDS.incrementAndGet());
        handlers.put(cookie, handler);
        return cookie;
    }

    /* package */
    void completeFeeBasisEstimateHandlerWithSuccess(Cookie cookie, TransferFeeBasis feeBasis) {
        CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError> handler = handlers.remove(cookie);
        if (null != handler) {
            executor.submit(() -> handler.handleData(feeBasis));
        }
    }

    /* package */
    void completeFeeBasisEstimateHandlerWithError(Cookie cookie, FeeEstimationError error) {
        CompletionHandler<com.blockset.walletkit.TransferFeeBasis, FeeEstimationError> handler = handlers.remove(cookie);
        if (null != handler) {
            executor.submit(() -> handler.handleError(error));
        }
    }

    /* package */
    void completeLimitEstimateWithSuccess(CompletionHandler<Amount, LimitEstimationError> handler, Amount amount) {
        executor.submit(() -> handler.handleData(amount));
    }

    /* package */
    void completeLimitEstimateWithError(CompletionHandler<Amount, LimitEstimationError> handler, LimitEstimationError error) {
        executor.submit(() -> handler.handleError(error));
    }
}
