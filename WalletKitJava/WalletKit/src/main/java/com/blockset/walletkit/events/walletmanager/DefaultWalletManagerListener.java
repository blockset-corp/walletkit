/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.events.walletmanager;

import com.blockset.walletkit.System;
import com.blockset.walletkit.WalletManager;

public interface DefaultWalletManagerListener extends WalletManagerListener {

    default void handleManagerEvent(System system, WalletManager manager, WalletManagerEvent event) {

    }
}
