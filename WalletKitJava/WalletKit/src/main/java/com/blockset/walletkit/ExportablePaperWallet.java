/*
 * Created by Ehsan Rezaie <ehsan@brd.com> on 11/23/20.
 * Copyright (c) 2020 Breadwinner AG.  All right reserved.
 *
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit;

import com.google.common.base.Optional;

public interface ExportablePaperWallet {

    Optional<? extends Key> getKey();
    Optional<? extends Address> getAddress();
}
