/*
 * Created by Ehsan Rezaie <ehsan@brd.com> on 11/23/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.errors;

public abstract class ExportablePaperWalletError extends Exception {
    /* package */
    ExportablePaperWalletError() {
        super();
    }

    /* package */
    ExportablePaperWalletError(String message) {
        super(message);
    }

    /* package */
    ExportablePaperWalletError(Throwable throwable) {
        super(throwable);
    }
}