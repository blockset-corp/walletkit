/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.events.system;

import com.blockset.walletkit.System;
import com.blockset.walletkit.events.network.DefaultNetworkListener;
import com.blockset.walletkit.events.transfer.DefaultTransferListener;
import com.blockset.walletkit.events.wallet.DefaultWalletListener;
import com.blockset.walletkit.events.walletmanager.DefaultWalletManagerListener;

public interface DefaultSystemListener extends SystemListener,
        DefaultWalletManagerListener,
        DefaultWalletListener,
        DefaultTransferListener,
        DefaultNetworkListener {

    default void handleSystemEvent(System system, SystemEvent event) {
    }
}
