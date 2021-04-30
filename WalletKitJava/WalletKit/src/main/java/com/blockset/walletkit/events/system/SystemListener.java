/*
 * Created by Michael Carrara <michael.carrara@breadwallet.com> on 7/1/19.
 * Copyright (c) 2019 Breadwinner AG.  All right reserved.
*
 * See the LICENSE file at the project root for license information.
 * See the CONTRIBUTORS file at the project root for a list of contributors.
 */
package com.blockset.walletkit.events.system;

import com.blockset.walletkit.events.network.NetworkListener;
import com.blockset.walletkit.events.transfer.TransferListener;
import com.blockset.walletkit.events.wallet.WalletListener;
import com.blockset.walletkit.events.walletmanager.WalletManagerListener;
import com.blockset.walletkit.System;

public interface SystemListener extends WalletManagerListener, WalletListener, TransferListener, NetworkListener {

    void handleSystemEvent(System system, SystemEvent event);
}
